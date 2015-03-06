/*
 * Copyright (C) 2014  mingspy@163.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <map>
#include <algorithm>
#include <float.h>
#include <math.h>
#include "dict.hpp"
#include "utils.hpp"
#include "sparse.hpp"
#include "minheap.hpp"

using namespace std;

namespace mingspy
{
const int TYPE_ESTR = -1000;
const int TYPE_ATOM = -1;
const int TYPE_IN_DICT = -2000;
const double PROB_INFINT = DBL_MAX;

struct Token {
public:
    int start; // start index.
    int end; // len
    int pos; // type of this Token or attribute index.
    double val;
    //wstring _word; // maybe empty
    Token(int start_off = 0, int end_off = 0, int attr = -1, double score = 0):start(start_off),end(end_off),pos(attr),val(score)
    {
    }

    friend ostream & operator<<(ostream & out, const Token & t)
    {
        out<<"("<<t.start<<","<<t.end<<","<<t.pos<<","<<t.val<<")";
        return out;
    }

    inline bool operator==(const Token& other) const
    {
        return start == other.start && end == other.end;
    }

    inline bool operator!=(const Token& other) const
    {
        return start != other.start || end != other.end;
    }

    inline bool operator<(const Token& other) const
    {
        return val < other.val;
    }
};

bool diff(const vector<Token> & v1, const vector<Token> & v2)
{
    if (v1.size() != v2.size()) return true;
    int vsize = v1.size();
    for (int i = 0; i < vsize; i ++) {
        if (v1[i] != v2[i]) return true;
    }
    return false;
}

bool token_compare_dsc(const Token & o1, const Token & o2)
{
    double d = o1.val - o2.val;
    if (d <= -0.00000001) return true;
    return false;
}

void print(const vector<Token> & tokenArr)
{
    for(int i = 0; i< tokenArr.size(); i++) {
        cout<<tokenArr[i];
    }
    cout<<endl;
}

void print(const string & str, const Token * tokenArr, int len)
{
    for(int i = 0; i< len; i++) {
        cout<<str.substr(tokenArr[i].start,tokenArr[i].end - tokenArr[i].start)<<"/"<<tokenArr[i].pos<<" ";
    }
    cout<<endl;
}

void substrs(const string & str, const Token * tokenArr, int len, vector<string> & result)
{
    for(int i = 0; i< len; i++) {
        result.push_back(str.substr(tokenArr[i].start,tokenArr[i].end - tokenArr[i].start));
    }
}


double viterbi( const Dictionary & dict, const vector<const SparseVector<int> *> & Observs, 
        Token * tokenArr, int arrLen)
{
    int T = Observs.size();
    Matrix<double> delta;
    Matrix<int> psi;

    // 1. Initialize.
    double nature_total = dict.getWordFreq(POS_FREQ_TOTAL);
    for(int i = 0; i< Observs[0]->size(); i++) {
        double emitp = -log((Observs[0]->getVal(i) + 1.0)/ (nature_total + 10.0));
        delta[0].setAttrVal(Observs[0]->getId(i), emitp);
    }

    // 2. Induction.
    // Get the best path from ti-1 -> ti;
    // In HMM: delta[t][j] = Max(delta[t-1][i]*a[i][j])*b[j][Obs[t]]
    //    or  delta[t][j] = Min{-[log[delta[t-1][i]+log(a[i][j])+log(b[j][Obs[t])]}
    int index = 0;
    double minProb = PROB_INFINT;
    double proba = 0;

    for ( int t = 1; t < T; t++ ) {
        // Calculate each roles best delta from previous to current.
        for ( int j = 0; j < Observs[t]->size(); j++ ) {
            minProb = PROB_INFINT;
            int state_cur = Observs[t]->getId(j);
            for ( int i = 0; i < Observs[t - 1]->size(); i++ ) {
                // Get a[i][j], same as -logP(ti-1|ti).
                int state_pre = Observs[t - 1]->getId(i);
                proba = -log((dict.getAttrFreq(state_pre, state_cur) + 1.0)/(dict.getWordFreq(state_pre)+10.0));
                // Add delta[t-1][i].
                proba += delta[t - 1].getVal(i);
                if ( proba < minProb ) {
                    index = i;
                    minProb = proba;
                }
            }
            psi[ t ].setAttrVal(j,index);

            double emitProb = -log((Observs[t]->getVal(j) + 1.0)/ (dict.getWordFreq(state_cur) + 10.0));
            assert(emitProb >= 0);
            delta[ t ].setAttrVal(j, minProb + emitProb);
        }
    }

    // 3.Terminal.
    // Record the best role tag's index.
    //bestPos.setAttrVal(T, -1);
    minProb = PROB_INFINT;
    index = 0;
    SparseVector<double> & lastDelta = delta[T - 1];
    for ( int i = 0; i < lastDelta.size(); i++ ) {
        if ( lastDelta.getVal(i) < minProb ) {
            index = i;
            minProb = lastDelta.getVal(i);
        }
    }


    // Get best path.
    //bestPos.push_back(index);
    tokenArr[T-1].pos = index;
    for ( int t = T - 1; t > 0; t-- ) {
        //bestPos.push_back(psi[t].getVal(bestPos[T- 1 - t]));
        index = psi[t].getVal(index);
        tokenArr[t-1].pos = index;
    }

    //reverse(bestPos.begin(),bestPos.end());
    // Get best pose.
    for ( int t = 0; t < T; t++ ) {
        //bestPos[i] = Observs[i]->getId(bestPos[i]);
        tokenArr[t].pos = Observs[t]->getId(tokenArr[t].pos);
    }
    assert(minProb >=0);
    return minProb;
}

struct Vetex {
    int id;
    double val;
    Vetex(int vid = 0, double v = 0):id(vid),val(v) {}
};

class Graph
{
private:
    vector<int> offs;
    vector<vector<Vetex> > rows;
    void ensureRow(int row)
    {
        while(rows.size() <= row) {
            rows.push_back(vector<Vetex>());
        }
    }
public:
    inline void push(int off)
    {
        offs.push_back(off);
    }
    inline int get(int id)
    {
        return offs[id];
    }
    inline int size()
    {
        return offs.size();
    }
    inline void setVetex(int row, int col, double val)
    {
        ensureRow(row);
        vector<Vetex> & r = rows[row];
        int N = r.size();
        for (int i = 0; i < N; i++) {
            if(r[i].id == col) {
                r[i].val = val;
                return;
            }
        }
        r.push_back(Vetex(col,val));
    }

    inline vector<Vetex> & getVetexs(int row)
    {
        assert(row < rows.size());
        return rows[row];
    }

    inline vector<Vetex> & operator[](int row)
    {
        assert(row < rows.size());
        return rows[row];
    }

    void calcLogProb(double totalFreq)
    {
        const int R = rows.size();
        for (int i = 0; i < R; i ++) {
            const int C = rows[i].size();
            for(int j = 0; j < C; j++) {
                rows[i][j].val = -log((rows[i][j].val + 1.0)/totalFreq);
            }
        }
    }

    inline void clear()
    {
        offs.clear();
        for(int i = 0; i < rows.size(); i++) {
            rows[i].clear();
        }
    }
    void print()
    {
        cout<<"-------------------------graph----------------"<<endl;
        cout<<"offs->";
        for(int i = 0; i < offs.size(); i++) {
            cout<<i<<":"<<offs[i]<<" ";
        }
        cout<<endl;
        cout<<"rows->"<<endl;
        for(int i = 0; i < rows.size(); i++) {
            cout<<"\t"<<i<<":";
            for(int j = 0; j < rows[i].size(); j++) {
                cout<<"("<<rows[i][j].id<<","<<rows[i][j].val<<")";
            }
            cout<<endl;
        }
        cout<<endl;
        cout<<"----------------------------------------------"<<endl;
    }
};

void genWordGraph(const Dictionary & dict,const string &strUtf8, Graph & graph)
{
    // 1. push all sigle atom word into graph
    graph.clear();
    vector<int> atom_offs;
    int len = strUtf8.length();
    int row = 0;
    graph.push(0);
    for(int i = 0; i < len; ) {
        int next = utf8_next_estr(strUtf8,i);
        int tp = TYPE_ESTR;
        if (next <= i) {
            tp = TYPE_ATOM;
            next  = i + utf8_char_len(strUtf8[i]);
        }
        graph.setVetex(row,row+1,1);
        graph.push(next);
        row ++;
        i = next;
    }
    // 2. find all possible word
    Dictionary::FreqInfo *info = NULL;
    for ( int i = 0; i < row; i ++) {
        for(int j = i+1; j <= row; j ++) {
            if (dict.exist(strUtf8,graph.get(i),graph.get(j))) {
                if (info = dict.getFreqInfo(strUtf8,graph.get(i),graph.get(j))) {
                    graph.setVetex(i,j,info->sum());
                } else {
                    graph.setVetex(i,j,1);
                }
            } else if(!dict.hasPrefix(strUtf8,graph.get(i),graph.get(j))) {
                break;
            }
        }
    }
#ifdef DEBUG
    graph.print();
#endif
}

class NShortPath
{
    Graph & _graph;
    MinHeap<Token> *  _edges;
    bool sorted;
public:
    NShortPath(Graph & graph, int n):_graph(graph),sorted(false)
    {
        _edges = new MinHeap<Token>[_graph.size()];
        for(int i = 0; i < graph.size(); i++) {
            _edges[i].resize(n);
        }
    }
    ~NShortPath()
    {
        delete [] _edges;
    };
    void calc()
    {
        // 1. initial weights
        // 2.calc weights:
        _edges[0].add_if_small(Token(-1,-1,0));
        int rsize = _graph.size() - 1;// 最后一个为结束节点，不参与计算
        for (int i = 0; i < rsize; i++) {
            // 当前节点的路径
            MinHeap<Token> & paths = _edges[i];
            // 当前节点可到达的节点集合
            vector<Vetex> & nexts =  _graph.getVetexs(i);
            for (int j = 0; j < paths.size(); j++) {
                for(int k = 0; k < nexts.size(); k++) {
                    double weight = paths[j].val + nexts[k].val;
                    _edges[nexts[k].id].add_if_small(Token(i,j,0,weight));
                }
            }
        }
#ifdef DEBUG
        print();
#endif
    }
    int getBestPath(int idx, Token * resultVec)
    {
        int last = _graph.size() - 1;
        MinHeap<Token> & endpath = _edges[last];
        if (idx >= endpath.size()) return false;
        if (!sorted) {
            endpath.sort();
            sorted = true;
        }
        vector<int> backoff;
        while(last != -1) {
            backoff.push_back(last);
            Token & cp = _edges[last][idx];
            idx = cp.end;
            last = cp.start;
        }
		int retLen;
        for(int i = backoff.size() - 1; i > 0; i--) {
			resultVec[retLen].start = _graph.get(backoff[i]);
			resultVec[retLen++].end = _graph.get(backoff[i - 1]);
        }
        return retLen;
    }
    void print()
    {
        cout<<"-----------------edges--------------------"<<endl;
        for (int i = 0; i < _graph.size(); i ++) {
            cout<<i<<":";
            MinHeap<Token> & heap = _edges[i];
            for(int j = 0; j < heap.size(); j++) {
                cout<<heap[j]<<" ";
            }
            cout<<endl;
        }
        cout<<"------------------------------------------"<<endl;
    }
};

class ShortPath
{
    struct Point {
        Point(int f = 0, double v = DBL_MAX):from(f),val(v) {}
        int from;
        double val;
    };
    Graph & _graph;
public:
    ShortPath(Graph & graph):_graph(graph)
    {
    }

    int getBestPath(Token * resultVec)
    {
        // 1. initial weights
        // 2.calc weights:
        int rsize = _graph.size();
        Point *points = new Point[rsize];
        points[0].val = 0;
        for (int i = 0; i < rsize - 1; i++) {
            vector<Vetex > & vts =  _graph.getVetexs(i);
            for(int j = 0; j < vts.size(); j++) {
                Vetex & chip = vts[j]; // 当前节点可到达的下一节点
                double weight = points[i].val + chip.val;
                if( points[chip.id].val > weight) {
                    points[chip.id].val = weight;
                    points[chip.id].from = i;
                }
            }
        }
#ifdef DEBUG
        cout<<"short path"<<endl;
        for( int i = 0; i< rsize; i++) {
            cout<<i<<"->"<<points[i].from<<" "<<points[i].val<<endl;
        }
#endif
        vector<int> backoff;
        for(int i = rsize - 1; i > 0;) {
            backoff.push_back(i);
            i = points[i].from;
        }
        backoff.push_back(0);
		int retLen = 0;
        for(int i = backoff.size() - 1; i > 0; i--) {
			resultVec[retLen].start = _graph.get(backoff[i]);
			resultVec[retLen++].end = _graph.get(backoff[i - 1]);
        }
        delete [] points;
		return retLen;
    }
};

class Tagger
{
protected:
    const Dictionary * _dict;
    mutable Dictionary::FreqInfo _possible_info;
    mutable bool _is_possible_gened;
    Dictionary::FreqInfo * getPossibleInfo() const
    {
        if (!_is_possible_gened){
            genPossibleInfo();
            _is_possible_gened = true;
        }
        return &_possible_info;
    }

    virtual void genPossibleInfo() const
    {
        vector<string> _possible_natures;
        _possible_natures.push_back("n");
        _possible_natures.push_back("a");
        _possible_natures.push_back("v");
        _possible_natures.push_back("c");
        _possible_natures.push_back("u");
        _possible_natures.push_back("p");
        _possible_info.clear();
        vector<int> freqs;
        double sum = 0;
        for(int i = 0; i < _possible_natures.size(); i++) {
            int freq = _dict->getWordFreq(_possible_natures[i]) + 1;
            sum += freq;
            freqs.push_back(freq);
        }

        for(int i = 0; i < freqs.size(); i++) {
            _possible_info.setAttrVal(_dict->getWordId(_possible_natures[i]), freqs[i] * 1000/sum);
        }
    }
public:
    explicit Tagger(const Dictionary * dict = NULL):_dict(dict),_is_possible_gened(false)
    {
    }
    void setDict(const Dictionary * dict = NULL){
        _dict = dict;
    }
    bool tagging(const string &utf8Str, Token * tokenArr, int arrLen) const
    {
        vector<const Dictionary::FreqInfo *> infos;
        for( int i = 0; i < arrLen; i ++) {
            const Dictionary::FreqInfo * info = _dict->getFreqInfo(utf8Str,tokenArr[i].start, tokenArr[i].end);
            if(info == NULL) {
                info = getPossibleInfo();
            }
            infos.push_back(info);
        }
        //vector<int> bests;
        viterbi(*_dict, infos, tokenArr, arrLen);
        return true;
    }

    bool tagging(const vector<string> & words, vector<string>& tags) const
    {
        // prepair tag infos
        vector<const SparseVector<int> *> infos;
        for( int i = 0; i < words.size(); i ++) {
            const Dictionary::FreqInfo * info = _dict->getFreqInfo(words[i]);
            if(info == NULL) {
                info = getPossibleInfo();
            }
            infos.push_back(info);
        }
        return tagging(infos,tags);
    }

    bool tagging(const vector<const SparseVector<int> *> & infos, vector<string>& tags) const
    {
        //vector<int> bests;
        //viterbi(*_dict, infos, bests);
        //for(int i = 0; i < bests.size(); i ++) {
        //    tags.push_back(_dict->getWord(bests[i]));
        //}
        Token tokenArr[infos.size()];
        viterbi(*_dict, infos, tokenArr, infos.size());
        for(int i = 0; i < infos.size(); i ++) {
            tags.push_back(_dict->getWord(tokenArr[i].pos));
        }
        return true;
    }
};

class RoleTagger: public Tagger{
public:
    explicit RoleTagger(const Dictionary * dict):Tagger(dict)
    {
    }
protected:
    virtual void genPossibleInfo() const {
        _possible_info.setAttrVal(_dict->getWordId("O"), _dict->getWordFreq("O") + 1);
        _possible_info.setAttrVal(_dict->getWordId("D"), _dict->getWordFreq("D") + 1);
        _possible_info.setAttrVal(_dict->getWordId("C"), _dict->getWordFreq("C") + 1);
        _possible_info.setAttrVal(_dict->getWordId("S"), _dict->getWordFreq("S") + 1);
    }
};

class NamedEntityRecognizer{
    RoleTagger *tagger;
    int PosType;
    int B;
    int C;
    int D;
    int S;
    int MaxLen; 
public:
    // type: the pos of ne to be set when recognized.
    // the max len of an ne word in byte
    NamedEntityRecognizer(const Dictionary * dict,int type, int maxLen){
        tagger = new RoleTagger(dict);
        PosType = type;
        B = dict->getWordId("B");
        C = dict->getWordId("C");
        D = dict->getWordId("D");
        S = dict->getWordId("S");
        MaxLen = maxLen;
    }
    ~NamedEntityRecognizer(){
        delete tagger;
    }

    int recognize(const string & utf8str, Token * tokenArr, int tokenArrLen){
        tagger->tagging(utf8str, tokenArr, tokenArrLen);
        int ent_len = 0;
        for(int i = 0; i < tokenArrLen;i++){
            if(tokenArr[i].pos == B){
                int j = i+1;
                while(j<tokenArrLen && tokenArr[j].pos == C){
                    j++;
                }
                if(j<tokenArrLen && tokenArr[j].pos == D && (tokenArr[j].end - tokenArr[i].start) < MaxLen){
                    tokenArr[ent_len].start = tokenArr[i].start;
                    tokenArr[ent_len].end = tokenArr[j].end;
                    tokenArr[ent_len].pos = PosType;
                    i = j;
                }
            }
            else{
                tokenArr[ent_len].start = tokenArr[i].start;
                tokenArr[ent_len].end = tokenArr[i].end;
                tokenArr[ent_len].pos = tokenArr[i].pos;
                if(tokenArr[i].pos == S){
                    tokenArr[ent_len].pos = PosType;
                }
            }
            ent_len ++;
        } 
        return ent_len;
    }
};

class IKnife
{
public:
    //IKnife(const Dictionary * the_dict):dict(the_dict){}
    virtual ~IKnife() {}
    /*
     * split out the str, then save the (start,end) into vector.
     * @param strUtf8 : the input str to split.
     * @param resultVec : resultVec words
     */
    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const = 0;
    void setDict(Dictionary * pdict){dict = pdict;}
    void setName(const string & name)
    {
        myname = name;
    }
    string getName() const
    {
        return myname;
    }
protected:
    const Dictionary * dict;
    string myname;
};

/*
 * 前向最大切分算法，也是最快的方法
 * Forward max match algorithm, the fast one.
 * ---------------------------------------------------
 *  小李飞刀（Fly cutter of lee）无疑是最快的刀了，见百晓生兵器谱
 *  第一，天机棒，又名如意棒　“天机老人”孙老头 ，死于上官金虹之手
 *  第二，子母龙凤环 　　“龙凤环”上官金虹 ，死于李寻欢之手 　
 *  第三，小李飞刀 　　“小李探花”李寻欢
 *
 */
class Flycutter: public IKnife
{
public:
    Flycutter(const Dictionary * refdict = NULL)
    {
        dict = refdict;
        setName("--* Lee's fly cutter: a forward max match tokenizer *--");
    }

    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const
    {
        int len = strUtf8.length();
		int retLen = 0;
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i) {
                //resultVec.push_back(Token(i,estr,TYPE_ESTR));
				resultVec[retLen].start = i;
				resultVec[retLen++].end = estr;
                i = estr;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < len ) {
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)) {
                    next = j;
                } else if(!dict->hasPrefix(strUtf8,i,j)) {
                    break;
                }
            }
            //resultVec.push_back(Token(i,next));
			resultVec[retLen].start = i;
			resultVec[retLen++].end = next;
            i = next;
        }
		assert(retLen <= resultVecLen);
		return retLen;
    }
};

/*
 * 后向切分算法实现。
 * Backward max split algorithm.
 * ---------------------------------------------------------------
 * 逆刃刀其实是新井赤空打造出来供神的刀，
 * 供神的刀一般会同时打造两把，一把较差的称作隐打，
 * 往往用来赠送给友人，而较好的真打则用来供神.
 * 剑心先前得到的就是隐打，在与濑田宗次郎的战斗中折断，
 * 后来从新井赤空儿子的手中接过了真打，并打败了前来夺刀的刀狩阿张
 * ---------------------------------------------------------------
 */
class Renda: public IKnife
{
public:
    Renda(const Dictionary * refdict = NULL)
    {
        dict = refdict;
        setName("--* renda: a backward max match tokenizer *--");
    }

    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const
    {
        int len = strUtf8.length();
        vector<int> pos;
        pos.push_back(len);
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if(estr > i) {
                i = estr;
                pos.push_back(len - estr);
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            pos.push_back(len - j);
            i = j;
        }
        string str(strUtf8.rbegin(),strUtf8.rend());
		int retLen = 0;
        for(int i = pos.size() - 1; i > 0; ) {
            int next = i-1;
            for(int j = i-2; j >= 0; j--) {
                if(dict->exist(str,pos[i],pos[j])) {
                    next = j;
                } else if(!dict->hasPrefix(str,pos[i],pos[j])) {
                    break;
                }
            }
            //resultVec.push_back(Token(len - pos[next], len - pos[i]));;
			resultVec[retLen].start = len - pos[next];
			resultVec[retLen++].end = len - pos[i];
            i = next;
        }
		
        //reverse(resultVec.begin(), resultVec.end());
		assert(retLen <= resultVecLen);
		Token tmp;
		int half = retLen / 2;
		for (int i = 0; i < half; i++){
			tmp = resultVec[i];
			resultVec[i] = resultVec[retLen - 1 - i];
			resultVec[retLen - 1 - i] = tmp;
		}
		return retLen;
    }
};

/*
 * 全切分方法，切出所有词典中出现的词
 * Full split algorithm, split all words in dictionary.
 * --------------------------------------------------------
 *  庖丁解牛，分毫不落。全切非庖丁不可了。
 *  庖丁为文惠君解牛，手之所触，肩之所倚，足之所履，
 *  膝之所踦， 砉然向然，奏刀騞然，莫不中音。
 */
class Paoding:public IKnife
{
public:
    Paoding(const Dictionary * refdict = NULL)
    {
        dict = refdict;
        setName("--* paoding: a full words tokenizer *--");
    }

    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const
    {
        int len = strUtf8.length();
        int best = -1;
		int retLen = 0;
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i) {
                //resultVec.push_back(Token(i,estr,TYPE_ESTR));
				resultVec[retLen].start = i;
				resultVec[retLen++].end = estr;
                i = estr;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < len ) {
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)) {
                    best = j;
                    //resultVec.push_back(Token(i,j));
					resultVec[retLen].start = i;
					resultVec[retLen++].end = j;
                } else if(!dict->hasPrefix(strUtf8,i,j)) {
                    break;
                }
            }
            if(best <= i) {
                //resultVec.push_back(Token(i,next));
				resultVec[retLen].start = i;
				resultVec[retLen++].end = next;
            }
            i = next;
        }
		assert(retLen <= resultVecLen);
		return retLen;
    }
};

class Unigram:public IKnife
{
public:
    Unigram(const Dictionary * refdict = NULL)
    {
        dict = refdict;
        setName("--* unigram: a unitary gram tokenizer *--");
    }
#ifdef BREAKDOWN
    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const
    {
        int split_len = 60;
        int slen = strUtf8.length();
		int retLen = 0;
        Graph graph;
        if (slen <= split_len) {
            retLen = dosplit(strUtf8, resultVec, graph);
			assert(retLen <= resultVecLen);
            return retLen;
        }
        static string puncs[] = {"。",",","，","!"," ","\n","！"};
        int puncs_size = 7;
        // sentance split to litte sentance
        int start = 0;
        int end = split_len;
		Token part[1000];
        while(end < slen) {
            int nxt = end + utf8_char_len(strUtf8[end]);
            for(int k = 0; k< puncs_size; k ++) {
                if (equal(puncs[k], strUtf8, end, nxt)) {
                    //vector<Token> resultVec;
                    int partLen = dosplit(strUtf8.substr(start, nxt - start),part,graph);
                    for(int m = 0; m < partLen; m++) {
                        resultVec[retLen].start = part[m].start + start;
						resultVec[retLen++].end = part[m].end + start;
                    }
                    start = nxt;
                    end = nxt + split_len;
                    break;
                }
            }

            if (start == nxt) continue;
            else end = nxt;
        }

        if (start < slen) {
            int partLen = dosplit(strUtf8.substr(start, slen - start),part, graph);
            for(int m = 0; m < partLen; m++) {
                resultVec[retLen].start = part[m].start + start;
				resultVec[retLen++].end = part[m].end + start;
            }
        }
		assert(retLen <= resultVecLen);
		return retLen;
    }

#else
    virtual int split(const string &strUtf8, Token * resultVec, int resultVecLen) const
    {
        Graph graph;
		return dosplit(strUtf8, resultVec, graph);
    }
#endif
private:
    inline int dosplit(const string &strUtf8, Token * resultVec, Graph & graph) const
    {
        genWordGraph(*dict, strUtf8, graph);
        graph.calcLogProb(dict->getWordFreq(WORDS_FREQ_TOTAL));
#ifdef NSHORTPATH
        NShortPath npath(graph, 6);
        npath.calc();
        return npath.getBestPath(0,resultVec);
#else
        ShortPath shortPath(graph);
        return shortPath.getBestPath(resultVec);
#endif
    }
};

}

