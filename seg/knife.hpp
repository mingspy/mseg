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
#include "../util/utils.hpp"
#include "../util/sparse.hpp"
#include "../util/minheap.hpp"

using namespace std;

namespace mingspy
{
const int TYPE_ESTR = -1000;
const int TYPE_ATOM = -1;
const int TYPE_IN_DICT = -2000;
const double PROB_INFINT = DBL_MAX;

inline int mkId(int start_row, int end_row)  { return end_row + ((start_row << 16)&0xffff0000);}
inline int getStart(int id)  { return (id >> 16) & 0xffff;}
inline int getEnd(int id)  { return id  & 0xffff;}

struct Chip {
public:
    int _start; // start index.
    int _end; // len
    int _attr; // type of this Chip or attribute index.
    double _val;
    //wstring _word; // maybe empty
    Chip(int start = 0, int end = 0, int attr = -1, double score = 0):_start(start),_end(end),_attr(attr),_val(score) {
    }

	friend ostream & operator<<(ostream & out, const Chip & t) {
        out<<"("<<t._start<<","<<t._end<<","<<t._attr<<","<<t._val<<")";
		return out;
    }

    inline bool operator==(const Chip& other) const{
        return _start == other._start && _end == other._end;
    }

    inline bool operator!=(const Chip& other) const{
        return _start != other._start || _end != other._end;
    }

    inline bool operator<(const Chip& other) const{
        return _val < other._val;
    }
};

bool diff(const vector<Chip> & v1, const vector<Chip> & v2){
    if (v1.size() != v2.size()) return true;
    int vsize = v1.size();
    for (int i = 0; i < vsize; i ++){
        if (v1[i] != v2[i]) return true;
    }
    return false;
}
bool chip_compare_asc(const Chip & o1, const Chip & o2){
    double d = o1._val - o2._val;
    if (d <= -0.00000001) return true;
    return false;
}

double viterbi( const Dictionary & dict, const vector<SparseVector<int> *> & Observs, vector<int> & bestPos)
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
    bestPos.push_back(index);
	for ( int t = T - 1; t > 0; t-- ) {
        bestPos.push_back(psi[t].getVal(bestPos[T- 1 - t]));
	}

    reverse(bestPos.begin(),bestPos.end());
	// Get best pose.
	for ( int i = 0; i < T; i++ ) {
		bestPos[i] = Observs[i]->getId(bestPos[i]);
	}
	assert(minProb >=0);
	return minProb;
}
	
class Graph{
    vector<vector<Chip> > rows;
    vector<int> offs;
    void ensureRow(int row){
        while(rows.size() <= row){ rows.push_back(vector<Chip>());}
    }
    bool _ended;
public:
    inline void addOff(int off) {offs.push_back(off);}
    inline int getOff(int id) { return offs[id];}
    inline int offSize() { return offs.size();}
    inline void addChip(const Chip & chip) {
        assert(!_ended);
        ensureRow(chip._start);
        vector<Chip>::iterator it = find(rows[chip._start].begin(),rows[chip._start].end(),chip);
        if (it == rows[chip._start].end())
            rows[chip._start].push_back(chip);
        else *it = chip;

    }

    inline int rowSize() const { 
        return rows.size();
    } 
    vector<Chip> & getChips(int row){ 
        assert(row < rows.size());
        return rows[row];
    }
    Chip & getChip(int row,int col){ return rows[row][col]; }
    void calcWeights(double totalFreq){
        for (int i = 0; i < rows.size(); i ++){
            for(int j = 0; j < rows[i].size(); j++){
                rows[i][j]._val = -log((rows[i][j]._val + 1.0)/totalFreq);
            }
        }
    }

    inline void start() {rows.clear();offs.clear();_ended = false;}
    // add last one as an tombstone 
    void end(){
        addChip(Chip(rows.size(), 0, 0, 0));
        _ended = true;
    }

    int  getEndChipId(){
        return mkId(rows.size() - 1, 0);
    }
    void print(){
        cout<<"-------------------------graph----------------"<<endl;
        cout<<"offs->";
        for(int i = 0; i < offs.size(); i++){
            cout<<i<<":"<<offs[i]<<" ";
        }
        cout<<endl;
        cout<<"rows->"<<endl;
        for(int i = 0; i < rows.size(); i++){
            cout<<"\t"<<i<<":";
            for(int j = 0; j < rows[i].size();j++){
                cout<<rows[i][j];
            } 
            cout<<endl;
        }
        cout<<endl;
        cout<<"----------------------------------------------"<<endl;
    }
    Graph():_ended(false){}
};

void print(const vector<Chip> & chips){
    for(int i = 0; i< chips.size(); i++) {
        cout<<chips[i];
    }
    cout<<endl;
}

void print(const string & str, const vector<Chip> & chips)
{
    for(int i = 0; i< chips.size(); i++) {
        cout<<"'"<<str.substr(chips[i]._start,chips[i]._end - chips[i]._start)<<"'  ";
    }
    cout<<endl;
}

void substrs(const string & str, const vector<Chip> & chips, vector<string> & result)
{
    for(int i = 0; i< chips.size(); i++) {
        result.push_back(str.substr(chips[i]._start,chips[i]._end - chips[i]._start));
    }
}

void genWordGraph(const Dictionary & dict,const string &strUtf8, Graph & graph)
{
    // 1. push all sigle atom word into graph
    graph.start();
    vector<int> atom_offs;
    int len = strUtf8.length();
    int row = 0;
    graph.addOff(0);
    for(int i = 0; i < len; ) {
        int next = utf8_next_estr(strUtf8,i);
        int tp = TYPE_ESTR;
        if (next <= i) {
            tp = TYPE_ATOM;
            next  = i + utf8_char_len(strUtf8[i]);
        }
        graph.addChip(Chip(row,row+1,tp));
        graph.addOff(next);
        row ++;
        i = next;
    }
    // 2. find all possible word
    Dictionary::FreqInfo *info = NULL;
    int asize = graph.offSize();
    for ( int i = 0; i < asize - 1; i ++){
        for(int j = i+1; j < asize; j ++){
            if (dict.exist(strUtf8,graph.getOff(i),graph.getOff(j))){
                if ((info = dict.getFreqInfo(strUtf8,graph.getOff(i),graph.getOff(j))) != NULL){
                     graph.addChip(Chip(i,j,TYPE_IN_DICT,info->sum()));
                }else{
                     graph.addChip(Chip(i,j,TYPE_IN_DICT));
                }
            }else if(!dict.hasPrefix(strUtf8,graph.getOff(i),graph.getOff(j))){
                break; 
            }
        }
    }
#ifdef DEBUG
    graph.print();
#endif
}

struct Paths{
#ifdef USE_MINHEAP 
    MinHeap<Chip> _paths;
    void addPath(int N, int fromId, int idx, double val){
        _paths.resize(N);
        _paths.add_if_small(Chip(fromId,idx,0,val)); 
    }
#else
    int maxid;
    vector<Chip> _paths;
    Paths():maxid(-1){}
    void addPath(int N, int fromId, int idx, double val){
        if(_paths.size() < N){
           _paths.push_back(Chip(fromId,idx,0,val)); 
        }else{
            double max_val = -1000000;
            if(maxid == -1){
                for(int i = 0; i < _paths.size(); i ++){
                    if(_paths[i]._val > max_val){
                        maxid = i;
                        max_val = _paths[i]._val;
                    }
                }
            }
            if(val < _paths[maxid]._val){
                _paths[maxid]._start = fromId;
                _paths[maxid]._end = idx;
                _paths[maxid]._val = val;
                maxid = -1;
            }
        }
    }
#endif 
};
class NShortPath{
    Graph & _graph;
    map<int, Paths> _edges;
    const int N;
    bool sorted;
public:
    NShortPath(Graph & graph, int n):_graph(graph),N(n),sorted(false){}
    void calc(){
        // 1. initial weights
        // 2.calc weights:
         vector<Chip > &chips =  _graph.getChips(0);
         for(int i = 0; i < chips.size(); i++){
             addPathWeight(mkId(chips[i]._start,chips[i]._end),0,0,0);
         }
        int rsize = _graph.rowSize() - 1;
        for (int i = 0; i < rsize; i++){
            vector<Chip > & nodes =  _graph.getChips(i);
            for(int j = 0; j < nodes.size(); j++){
                //assert(nodes[j]._start == i);
                relax(nodes[j]);
            }
        }
#ifdef DEBUG
        print();
#endif
    }
    bool getBestPath(int idx, vector<Chip> & result){
        int wordid = _graph.getEndChipId();
        Paths & endpath = _edges[wordid];
        if (idx >= endpath._paths.size()) return false;
        if (!sorted){
#ifdef USE_MINHEAP
            endpath._paths.sort();
#else
            sort(endpath._paths.begin(),endpath._paths.end(), chip_compare_asc);
#endif
            sorted = true;
        }
        vector<int> back_ids;
        while(wordid != 0){
            Chip & cp = _edges[wordid]._paths[idx];
            wordid = cp._start;
            idx = cp._end;
            if (wordid != 0) back_ids.push_back(wordid);
        }
        for(int i = back_ids.size() - 1; i >= 0;i--){
            result.push_back(Chip(_graph.getOff(getStart(back_ids[i])),_graph.getOff(getEnd(back_ids[i]))));
        }
        return true;
    }
    void print(){
        cout<<"-----------------edges--------------------"<<endl;
        for (map<int,Paths>::iterator it = _edges.begin(); it != _edges.end(); it++){
            cout<<getStart(it->first)<<","<<getEnd(it->first)<<"->";
            for(int i = 0; i < it->second._paths.size();i++){
                cout<<it->second._paths[i]<<endl;
            }
        }
        cout<<"------------------------------------------"<<endl;
    }
private:
    void relax(const Chip & chip){
        int chipid = mkId(chip._start, chip._end);
        vector<Chip > & nexts =  _graph.getChips(chip._end);
        for(int j = 0; j < nexts.size(); j++){
            for(int i = 0; i < _edges[chipid]._paths.size(); i++){
                addPathWeight(mkId(nexts[j]._start,nexts[j]._end), chipid, i, _edges[chipid]._paths[i]._val+chip._val);
            }
        }
    }
    inline void addPathWeight(int id, int fromId, int idx, double val){
        _edges[id].addPath(N,fromId,idx,val);
    }
};

class ShortPath{
    struct Point{
        Point(int f = 0, double v = DBL_MAX):from(f),val(v){}
        int from;
        double val;
    };
    Graph & _graph;
public:
    ShortPath(Graph & graph):_graph(graph)
    {
    }

    void getBestPath(vector<Chip> & result){
        // 1. initial weights
        // 2.calc weights:
        int rsize = _graph.offSize();
        Point *points = new Point[rsize];
        points[0].val = 0;
        for (int i = 0; i < rsize - 1; i++){
            vector<Chip > & nodes =  _graph.getChips(i);
            for(int j = 0; j < nodes.size(); j++){
                Chip & chip = nodes[j]; // 当前节点可到达的下一节点
                double weight = points[i].val + chip._val;
                if( points[chip._end].val > weight){
                    points[chip._end].val = weight;
                    points[chip._end].from = i;
                }
            }
        }
#ifdef DEBUG
        cout<<"short path"<<endl;
        for( int i = 0; i< rsize; i++){
            cout<<i<<"->"<<points[i].from<<" "<<points[i].val<<endl;
        }
#endif
        vector<int> backoff;
        for(int i = rsize - 1; i > 0;){
            backoff.push_back(i);
            i = points[i].from;
        }
        backoff.push_back(0);
        for(int i = backoff.size() - 1; i > 0;i--){
            result.push_back(Chip(_graph.getOff(backoff[i]),_graph.getOff(backoff[i - 1])));
        }
        delete [] points;
    }
};
class Tagger{
    const Dictionary &_dict;
    mutable SparseVector<int> _possible_info;
    mutable bool _info_gened;
    vector<string> _possible_natures;
public:
    SparseVector<int> *  genPossibleInfo() const {
        if(!_info_gened){
            _possible_info.clear();
            vector<int> freqs;
            double sum = 0;
            for(int i = 0; i < _possible_natures.size(); i++){
                int freq = _dict.getWordFreq(_possible_natures[i]) + 1;
                sum += freq;
                freqs.push_back(freq);
            }

            for(int i = 0; i < freqs.size(); i++){
                _possible_info.setAttrVal(_dict.getWordId(_possible_natures[i]), freqs[i] * 1000/sum); 
            }
            _info_gened = true;
        }
        return &_possible_info;
    }
public:
    explicit Tagger(Dictionary & dict):_dict(dict),_info_gened(false){
        _possible_natures.push_back("n");
        _possible_natures.push_back("a");
        _possible_natures.push_back("v");
        _possible_natures.push_back("c");
        _possible_natures.push_back("u");
        _possible_natures.push_back("p");
    }
    inline bool tagging(const string &utf8Str, vector<Chip>& chips) const{
        vector<SparseVector<int> *> infos; 
        for( int i = 0; i < chips.size(); i ++){
            Dictionary::FreqInfo * info = _dict.getFreqInfo(utf8Str,chips[i]._start, chips[i]._end);
            if(info == NULL){
                info = genPossibleInfo();
            } 
            infos.push_back(info);
        }
        vector<int> bests;
        viterbi(_dict, infos, bests);
        for(int i = 0; i < bests.size(); i ++){
            chips[i]._attr = bests[i];
        }
        return true;
    }

    inline bool tagging(const vector<string> & words, vector<string>& tags) const{
        // prepair tag infos
        vector<Dictionary::FreqInfo *> infos; 
        for( int i = 0; i < words.size(); i ++){
            Dictionary::FreqInfo * info = _dict.getFreqInfo(words[i]);
            if(info == NULL){
                info = genPossibleInfo();
            } 
            infos.push_back(info);
        }
        return tagging(infos,tags);
    }

    inline bool tagging(const vector<SparseVector<int> *> & infos, vector<string>& tags) const{
        vector<int> bests;
        viterbi(_dict, infos, bests);
        for(int i = 0; i < bests.size(); i ++){
            tags.push_back(_dict.getWord(bests[i]));
        }
        return true;
    }
};

class IKnife
{
public:
    virtual ~IKnife(){}
    /*
     * split out the str, then save the (start,end) into vector.
     * @param strUtf8 : the input str to split.
     * @param result : result words
     */
    virtual void split(const string &strUtf8, vector<Chip> & result) const = 0;
	void setDict(Dictionary * pdict){dict = pdict;}
    void setName(const string & name){myname = name;}
    string getName() const {return myname;}
protected:
	Dictionary * dict;
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
    Flycutter(Dictionary * refdict = NULL)
    {
        setDict(refdict);
        setName("--* Lee's fly cutter: a forward max match tokenizer *--");
    }

    virtual void split(const string &strUtf8, vector<Chip> & result) const
    {
        int len = strUtf8.length();
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i){
                result.push_back(Chip(i,estr,TYPE_ESTR));
                i = estr;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < len ){
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)){ next = j; }
                else if(!dict->hasPrefix(strUtf8,i,j)){ break; }
            }
            result.push_back(Chip(i,next));
            i = next;
        }
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
    Renda(Dictionary * refdict = NULL) {
        setDict(refdict);
        setName("--* renda: a backward max match tokenizer *--");
    }

    virtual void split(const string &strUtf8, vector<Chip> & result) const
    {
		int len = strUtf8.length();
        vector<int> pos;
        pos.push_back(len);
        for(int i = 0; i < len; ){
			int estr = utf8_next_estr(strUtf8,i);
            if(estr > i){
                i = estr;
                pos.push_back(len - estr);
                continue;
            }
			int j = i + utf8_char_len(strUtf8[i]);
            pos.push_back(len - j);
            i = j;
        }
        string str(strUtf8.rbegin(),strUtf8.rend());
        for(int i = pos.size() - 1; i > 0; ) {
			int next = i-1;
            for(int j = i-2; j >= 0; j--){
				if(dict->exist(str,pos[i],pos[j])){ next = j; }
                else if(!dict->hasPrefix(str,pos[i],pos[j])){ break; }
            }
			result.push_back(Chip(len - pos[next], len - pos[i]));
            i = next;
        }
        reverse(result.begin(), result.end());
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
class Paoding:public IKnife{
public:
    Paoding(Dictionary * refdict = NULL) {
        setDict(refdict);
        setName("--* paoding: a full words tokenizer *--");
    }

    virtual void split(const string &strUtf8, vector<Chip> & result) const
    {
        int len = strUtf8.length();
        int best = -1;
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i){
                result.push_back(Chip(i,estr,TYPE_ESTR));
                i = estr;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < len ){
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)){ 
                    best = j;
                    result.push_back(Chip(i,j));
                } else if(!dict->hasPrefix(strUtf8,i,j)){ break; }
            }
            if(best <= i){ result.push_back(Chip(i,next)); }
            i = next;
        }
    }
};

class Unigram:public IKnife{
public:
    Unigram(Dictionary * refdict = NULL) {
        setDict(refdict);
        setName("--* unigram: a unitary gram tokenizer *--");
    }

    virtual void split(const string &strUtf8, vector<Chip> & result) const
    {
        Graph graph;
        genWordGraph(*dict, strUtf8, graph);
        graph.calcWeights(dict->getWordFreq(WORDS_FREQ_TOTAL));
        graph.end();
#ifdef NSHORTPATH
        NShortPath npath(graph, 6);
        npath.calc();
        npath.getBestPath(0,result);
#else
        ShortPath shortPath(graph);
        shortPath.getBestPath(result);
#endif
    }
};

class Mixture:public IKnife{
public:
    Mixture(Dictionary * refdict = NULL) {
        setDict(refdict);
        setName("--* Mixture: strategy tokenizer using cutter,renda,unigram *--");
    }

    virtual void split(const string &strUtf8, vector<Chip> & result) const
    {
        vector<Chip> r1;
        Flycutter cutter(dict);
        cutter.split(strUtf8, r1);
        vector<Chip> r2;
        Renda renda(dict);
        renda.split(strUtf8,r2);
        if (!diff(r1,r2)){
            result = r2;
            return;
        }

        // gen graph
        Graph graph;
        map<int,int> off2row;
        int i = 0, j = 0, row = 0, off = 0;
        graph.addOff(0);
        off2row[0]=0;
        for(; i < r1.size() && j < r2.size();){
            if(r1[i]._end == r2[j]._end){ off = r1[i]._end; i++; j++;
            }else if(r1[i]._end < r2[j]._end){ off = r1[i]._end; i++;
            }else{ off = r2[j]._end; j++; }
            graph.addOff(off);
            ++row;
            off2row[off] = row;
        }
        Dictionary::FreqInfo *info = NULL;
        double freq = 0;
        for(i = 0; i < r1.size(); i++){
            freq = 0;
            if ((info = dict->getFreqInfo(strUtf8,r1[i]._start,r1[i]._end)) != NULL){
                freq = info->sum();
            }
            graph.addChip(Chip(off2row[r1[i]._start],off2row[r1[i]._end],-1,freq));
        }

        for(i = 0; i < r2.size(); i++){
            freq = 0;
            if ((info = dict->getFreqInfo(strUtf8,r2[i]._start,r2[i]._end)) != NULL){
                freq = info->sum();
            }
            graph.addChip(Chip(off2row[r2[i]._start],off2row[r2[i]._end],-1,freq));
        }
        graph.calcWeights(dict->getWordFreq(WORDS_FREQ_TOTAL));
        graph.end();
        NShortPath npath(graph, 8);
        npath.calc();
        npath.getBestPath(0,result);
    }

};
}
