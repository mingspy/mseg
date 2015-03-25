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
const int MIN_TOKEN_BUFSIZE = 3000;

struct Token {
public:
    int start; // start index.
    int end; // len
    int pos; // type of this Token or attribute index.
    double val;
    Token(int start_off = 0, int end_off = 0, int attr = -1, double score = 0)
        :start(start_off),end(end_off),pos(attr),val(score) { }

    friend ostream & operator<<(ostream & out, const Token & t) {
        out<<"("<<t.start<<","<<t.end<<","<<t.pos<<","<<t.val<<")";
        return out;
    }

    inline bool operator==(const Token& other) const { return start == other.start && end == other.end; }
    inline bool operator!=(const Token& other) const { return start != other.start || end != other.end; }
    inline bool operator<(const Token& other) const { return val < other.val; }
};

//bool diff(const vector<Token> & v1, const vector<Token> & v2)
//{
//    if (v1.size() != v2.size()) return true;
//    int vsize = v1.size();
//    for (int i = 0; i < vsize; i ++) { if (v1[i] != v2[i]) return true; }
//    return false;
//}

//bool token_compare_dsc(const Token & o1, const Token & o2) { 
//    double d = o1.val - o2.val;
//    if (d <= -0.00000001) return true;
//    return false;
//}

void print(const Token* tokenArr, int len) {
    for(int i = 0; i< len; i++) { cout<<tokenArr[i]; }
    cout<<endl;
}

void print(const string & str, const Token * tokenArr, int len, const Dictionary * dict = NULL) {
    const char * pos = NULL;
    for(int i = 0; i< len; i++) {
        cout<<str.substr(tokenArr[i].start,tokenArr[i].end - tokenArr[i].start)<<"/";
        if(dict &&(pos = dict->getWord(tokenArr[i].pos)) != NULL){
            cout<<pos<<" ";
        }else{
            cout<<tokenArr[i].pos<<" ";
        }
    }
    cout<<endl;
}

void substrs(const string & str, const Token * tokenArr, int len, vector<string> & result) {
    for(int i = 0; i< len; i++) {
        result.push_back(str.substr(tokenArr[i].start,tokenArr[i].end - tokenArr[i].start));
    }
}


const int VITERBI_MAX_STATE_SIZE = 20; 
#define setMatrixVal(matrix,row,col,val) (matrix[(row) * VITERBI_MAX_STATE_SIZE + col] = (val))
#define getMatrixVal(matrix,row,col) matrix[(row) * VITERBI_MAX_STATE_SIZE + col]

double viterbi( const Dictionary & dict, const SparseVector<int> * * Observs,const int T, 
        Token * tokenArr, int arrLen) {
    const int matrix_cap = T*VITERBI_MAX_STATE_SIZE;
    double delta[matrix_cap];
    int psi[matrix_cap];
    memset(delta, 0 , sizeof(double)*matrix_cap);
    memset(psi, 0 , sizeof(int)*matrix_cap);

    // 1. Initialize.
    // 设置第一个节点路径值为emit value
    double nature_total = dict.getWordFreq(POS_FREQ_TOTAL);
    assert(Observs[0]->size() <= VITERBI_MAX_STATE_SIZE);
    for(int i = 0; i< Observs[0]->size(); i++) {
        double emitp = -log(((*Observs[0])[i].val + 1.0)/ (nature_total + 10.0));
        delta[i]  = emitp;
    }

    // 2. Induction.
    // Get the best path from ti-1 -> ti;
    // In HMM: delta[t][j] = Max(delta[t-1][i]*a[i][j])*b[j][Obs[t]]
    //    or  delta[t][j] = Min{-[log[delta[t-1][i]+log(a[i][j])+log(b[j][Obs[t])]}
    int best_state = 0;
    double minProb = PROB_INFINT;
    double proba = 0;
    for ( int t = 1; t < T; t++ ) {
        // Calculate each roles best delta from previous to current.
        for ( int j = 0; j < Observs[t]->size(); j++ ) {
            assert(Observs[t]->size() <= VITERBI_MAX_STATE_SIZE);
            minProb = PROB_INFINT;
            int state_cur = (*Observs[t])[j].id;
            for ( int i = 0; i < Observs[t - 1]->size(); i++ ) {
                // Get a[i][j], same as -logP(ti-1|ti).
                int state_pre = (*Observs[t - 1])[i].id;
                proba = -log((dict.getAttrFreq(state_pre, state_cur) + 1.0)/(dict.getWordFreq(state_pre)+10.0));
                // Add delta[t-1][i].
                proba += getMatrixVal(delta,t - 1,i);
                if ( proba < minProb ) {
                    best_state = i;
                    minProb = proba;
                }
            }
            double emitProb = -log(((*Observs[t])[j].val + 1.0)/ (dict.getWordFreq(state_cur) + 10.0));
            setMatrixVal(delta,t,j,minProb + emitProb);
            setMatrixVal(psi,t,j,best_state);
        }
    }

    // 3.Terminal.
    // Record the best role tag's index.
    minProb = PROB_INFINT;
    best_state = 0;
    for (int i = 0; i < Observs[T - 1]->size(); i ++){
        if ( getMatrixVal(delta,T - 1, i) < minProb ) {
            best_state = i;
            minProb = getMatrixVal(delta,T-1,i);
        }
    }

    // Get best path.
    tokenArr[T-1].pos =  (*Observs[T-1])[best_state].id;
    for ( int t = T-1 ; t > 0; t-- ) {
        best_state = getMatrixVal(psi,t,best_state);
        tokenArr[t-1].pos =  (*Observs[t-1])[best_state].id;
    }

    assert(minProb >=0);
    return minProb;
}

inline int next_estr_num(const string & strUtf8, int start, int * type) {
    int estr = utf8_next_estr(strUtf8,start);
    if (estr > start) {
        *type = POS_NX;
        return estr;
    }
    int num = utf8_next_num(strUtf8,start);
    if (num > start) {
        *type = POS_MA;
        return num;
    }
    return start;
}

class Graph
{
private:
    // 保存每个字的下标位置
    FixedMatrix<double> _rows;
    int _offs[300];
    int _size;
    inline void clear()
    {

        //offs.clear();
        _size = 0;
        _rows.clear();
    }
public:
    inline int getOffset(int id) const { return _offs[id]; }
    // how many rows of this graph
    inline int size() const { return _size; }
    void gen(const Dictionary & dict,const string &strUtf8,int start, int endoff)
    {
        clear();
        // 1. push all sigle atom word into graph
        _offs[_size++] = start;
        for(int i = start; i < endoff; ) {
            int next = utf8_next_alnum(strUtf8,i);
            if (next <= i) {
                next  = i + utf8_char_len(strUtf8[i]);
            }
            _offs[_size++] = next;
            i = next;
        }
        // 2. find all possible word
        Dictionary::FreqInfo *info = NULL;
        for ( int i = 0; i < _size - 1; i ++) {
            for(int j = i+1; j <_size; j ++) {
                if ((info = dict.getFreqInfo(strUtf8,_offs[i],_offs[j]))!=NULL) {
#ifdef DEBUG
                    cerr<<i<<":("<<_offs[i]<<","<<_offs[j]<<")->\'"<<strUtf8.substr(_offs[i],_offs[j] - _offs[i]).c_str()<<"\' in dict"<<endl;
#endif
                    _rows.push_back(i,j,info->sum());
                } else {
                    if (j == i + 1){
                        _rows.push_back(i,j,1);
                    }
                    if(!dict.hasPrefix(strUtf8,_offs[i],_offs[j])) {
                        break;
                    }
                }
            }
        }
#ifdef DEBUG
        print();
#endif
    }


    inline FixedMatrix<double>::Row & operator[](int row) { return _rows[row]; }
    void calcLogProb(double totalFreq)
    {
        for(int i = 0; i < _rows.size(); i ++){
            FixedMatrix<double>::Row & row = _rows[i];
            for ( int j = 0; j < row.size(); j ++){ 
                row[j].val = -log((row[j].val + 1.0)/(totalFreq+1.0));
            }
        }
    }

    void print()
    {
        cout<<"-------------------------graph----------------"<<endl;
        cout<<"offs->";
        for(int i = 0; i < size(); i++) {
            cout<<i<<":"<<_offs[i]<<" ";
        }
        cout<<endl;
        cout<<"rows->"<<_rows<<endl;
        cout<<"----------------------------------------------"<<endl;
    }

};

class NShortPath
{
public:
    static int getBestPath(Graph & _graph, Token * tokenArr,int N)
    {
        const int _esize = _graph.size();
        MinHeap<Token>  _edges[_esize];
        for(int i = 0; i < _esize; i++) {
            _edges[i].resize(N);
        }

        // 1. initial weights
        // 2.calc weights:
        _edges[0].add_if_small(Token(-1,-1,0));
        int rsize = _graph.size() - 1;// 最后一个为结束节点，不参与计算
        for (int i = 0; i < rsize; i++) {
            // 当前节点的路径
            MinHeap<Token> & paths = _edges[i];
            // 当前节点可到达的节点集合
            //SparseList<double> & nexts =  _graph.getVetexs(i);
            FixedMatrix<double>::Row &  nexts =  _graph[i];
            const int pSize = paths.size();
            for (int j = 0; j < pSize; j++) {
                for(int k = 0; k < nexts.size(); k++) {
                    double weight = paths[j].val + nexts[k].val;
                    _edges[nexts[k].id].add_if_small(Token(i,j,0,weight));
                }
            }
        }

        int idx = 0;
        int retLen = 0;
        int last = _graph.size() - 1;
        MinHeap<Token> & endpath = _edges[last];
        if (idx >= (int)endpath.size()) return false;
        endpath.sort();
        int backoff[_esize];
        int len = 0;
        while(last > - 1) {
            backoff[len++] = last;
            Token & cp = _edges[last][idx];
            idx = cp.end;
            last = cp.start;
        }
        for(int i = len - 1; i > 0; i--) {
            tokenArr[retLen].start = _graph.getOffset(backoff[i]);
            tokenArr[retLen++].end = _graph.getOffset(backoff[i - 1]);
        }
        return retLen;
    }
};

class ShortPath
{
    struct Point {
        Point(int f = 0, double v = DBL_MAX):from(f),val(v) {}
        int from;
        double val;
    };
public:
    static int getBestPath(Graph & _graph, Token * tokenArr)
    {
        // 1. initial weights
        // 2.calc weights:
        const int rsize = _graph.size();
        Point points[rsize];
        points[0].val = 0;
        for (int i = 0; i < rsize - 1; i++) {
            //SparseList<double> & vts =  _graph.getVetexs(i);
            FixedMatrix<double>::Row & row =  _graph[i];
            for(int j = 0; j < row.size(); j++) {
                double weight = points[i].val + row[j].val;
                if( points[row[j].id].val > weight) {
                    points[row[j].id].val = weight;
                    points[row[j].id].from = i;
                }
            }
        }
        int backoff[rsize];
        int len = 0;
        for(int i = rsize - 1; i > 0;) {
            backoff[len ++] = i;
            i = points[i].from;
        }
        backoff[len++] = 0;
        int retLen = 0;
        for(int i = len - 1; i > 0; i--) {
            tokenArr[retLen].start = _graph.getOffset(backoff[i]);
            tokenArr[retLen++].end = _graph.getOffset(backoff[i - 1]);
        }
        return retLen;
    }
};

// POS tagger
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
        const int pSize = _possible_natures.size();
        for(int i = 0; i < pSize; i++) {
            int freq = _dict->getWordFreq(_possible_natures[i]) + 1;
            sum += freq;
            freqs.push_back(freq);
        }

        const int fSize = freqs.size();
        for(int i = 0; i < fSize; i++) {
            _possible_info.setValById(_dict->getWordId(_possible_natures[i]), freqs[i] * 1000/sum);
        }
    }
public:
    explicit Tagger(const Dictionary * dict = NULL):_dict(dict),_is_possible_gened(false) {
    }
    void setDict(const Dictionary * dict = NULL){
        _dict = dict;
    }
    bool tagging(const string &utf8Str, Token * tokenArr, int arrLen) const
    {
        assert(_dict != NULL);
        const int strsize = utf8Str.length();
        const Dictionary::FreqInfo * infos[strsize];
        int infolen = 0;
        for( int i = 0; i < arrLen; i ++) {
            const Dictionary::FreqInfo * info = _dict->getFreqInfo(utf8Str,tokenArr[i].start, tokenArr[i].end);
            if(info == NULL) {
                info = getPossibleInfo();
            }
            infos[infolen++] = info;
            //infos.push_back(info);
        }
        viterbi(*_dict, infos, infolen, tokenArr, arrLen);
        return true;
    }

    bool tagging(const vector<string> & words, vector<string>& tags) const
    {
        // prepair tag infos
        const SparseVector<int> * infos[words.size()];
        int infos_len = 0;
        const int wSize = words.size();
        for( int i = 0; i < wSize; i ++) {
            const Dictionary::FreqInfo * info = _dict->getFreqInfo(words[i]);
            if(info == NULL) {
                info = getPossibleInfo();
            }
            infos[infos_len++] = info;
        }
        return tagging(infos,infos_len,tags);
    }

    bool tagging(const SparseVector<int> * * infos, const int infos_len, vector<string>& tags) const
    {
        Token tokenArr[1000];
        viterbi(*_dict, infos, infos_len, tokenArr, infos_len);
        for(int i = 0; i < infos_len; i ++) {
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
        _possible_info.setValById(_dict->getWordId("O"), _dict->getWordFreq("O") + 1);
        _possible_info.setValById(_dict->getWordId("D"), _dict->getWordFreq("D") + 1);
        _possible_info.setValById(_dict->getWordId("C"), _dict->getWordFreq("C") + 1);
        _possible_info.setValById(_dict->getWordId("S"), _dict->getWordFreq("S") + 1);
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

const int SPLIT_FLAG_NONE = 0; // do nothing extra split analysis
const int SPLIT_FLAG_POS = 1;  // do POS tagging
const int SPLIT_FLAG_NER = 2;  // do Named entitiy recognization

class IKnife
{
public:
    //IKnife(const Dictionary * the_dict):dict(the_dict){}
    IKnife():dict(NULL),pos_dict(NULL){}
    virtual ~IKnife() {}
    /*
     * split out the str, then save the (start,end) into vector.
     * @param strUtf8 : the input str to split.
     * @param tokenArr : tokenArr words
     */
    virtual int do_split(const string &strUtf8,int start,int endoff, Token * tokenArr) const = 0;
    int split(const string &strUtf8, Token * tokenArr, int tokenArrLen, int flag = 0) const
    {
        static const int split_len = 60;
        static const int max_sentence_len = 890; // 300 characters
        static const string puncs[] = {"。", "，", " ", "\n","\t" };
        static const int puncs_size = 5;
        //static const string puncs[] = {"。", "，", " ", "\n", "！", "!", "?","？","、",",","‘","“","\"","'"};
        //static const int puncs_size = 14;
        
        //bool do_ner = flag & SPLIT_FLAG_NER;
        bool do_pos = flag & SPLIT_FLAG_POS;

        int slen = strUtf8.length();
        int retLen = 0;
        // sentance split to litte sentance
        int start = 0;
        int cur = split_len;
        while(cur < slen) {
            int cur_char_len =  utf8_char_len(strUtf8[cur]);
            for(int k = 0; k< puncs_size; k ++) {
                if (((cur-start) > max_sentence_len)||equal(puncs[k], strUtf8, cur, cur_char_len)) {
                    int partLen = do_split(strUtf8, start, cur,tokenArr + retLen);
                    if(do_pos){
                        tagger.tagging(strUtf8,tokenArr+retLen,partLen);
                    }
                    retLen += partLen;
                    // set delimiter.
                    if ((cur-start) > max_sentence_len){
                        start = cur;
                    }else{
                        tokenArr[retLen].start = cur;
                        tokenArr[retLen].pos= POS_W;
                        tokenArr[retLen++].end = cur + cur_char_len;
                        start = cur_char_len + cur;
                    }
                    cur += split_len;
                    break;
                }
            }
            cur += cur_char_len;
        }

        if (start < slen) {
            int partLen = do_split(strUtf8, start, slen,tokenArr + retLen);
            if(do_pos){
                tagger.tagging(strUtf8,tokenArr+retLen,partLen);
            }
            retLen += partLen;
        }
        assert(retLen <= tokenArrLen);
        retLen = mark_alpha_num(strUtf8, tokenArr, retLen);
        return retLen;
    }

    static int mark_alpha_num(const string & str, Token * arr, int len){
        static bool number_dict_inited = false;
        static Dictionary number_dict;
        if(!number_dict_inited){
            string numbers = "〇一二三四五六七八九十零壹贰叁肆伍陆柒捌玖拾佰仟万百千亿兆";
            const int length = numbers.length();
            for(int i = 0; i < length;){
                int next = utf8_char_len(numbers[i]);
                number_dict.addAttrFreq(numbers.substr(i,next), POS_MC, 1);
                i += next;
            }
            number_dict_inited = true;
        }
        for(int i = 0; i < len; i ++){
            int inext = utf8_next_num(str, arr[i].start);
            if (inext == arr[i].end){
                arr[i].pos = POS_MA;
                // 简单规则判断是否是电话
                int ilen = arr[i].end - arr[i].start;
                if( ilen == 11 && str[arr[i].start] == '1' 
                        && (str[arr[i].start + 1] > '2' && str[arr[i].start + 1] < '9')){
                    arr[i].pos = POS_MP;
                }else if (ilen == 12 && (str[arr[i].start + 3] == '-' || str[arr[i].start + 4] == '-')){
                    arr[i].pos = POS_MP;
                }
                // 判断是否是时间
                // 2015-03-17
                else if(ilen == 10 && str[arr[i].start + 5] <= '1' && str[arr[i].start + 8] <= '3'
                        &&((str[arr[i].start + 4] == '-' && str[arr[i].start + 7] == '-')
                            || (str[arr[i].start + 4] == '/' && str[arr[i].start + 7] == '/')))
                {
                    arr[i].pos = POS_T;
                } // 2015-3-17 2015-12-1
                else if(ilen == 9&&(str[arr[i].start + 4] == '-' || str[arr[i].start + 4] == '/' ) 
                        &&((str[arr[i].start+7] <='3'&&(str[arr[i].start+6] == '-' || str[arr[i].start+6] == '/'))
                            ||(str[arr[i].start+5] <='3'&&(str[arr[i].start+7] == '-' || str[arr[i].start+7] == '/'))))
                {
                    arr[i].pos = POS_T;
                } // 18:21:19
                else if (ilen == 8 && str[arr[i].start + 2] == ':' && str[arr[i].start + 5] == ':'&&
                        str[arr[i].start] <= '2' && str[arr[i].start + 3] <= '6' 
                        && str[arr[i].start + 6] <='6'){
                    arr[i].pos = POS_T;

                } // 18:22
                else if (ilen == 5 && str[arr[i].start + 2] == ':' && str[arr[i].start] <= '2' && str[arr[i].start + 3] <= '6' 
                        ){
                    arr[i].pos = POS_T;
                }else if(i < len - 1 && ilen <= 4){
                    int nextlen = arr[i+1].end - arr[i+1].start;
                    if(equal("年",str,arr[i+1].start, nextlen) || 
                       equal("月",str,arr[i+1].start, nextlen) ||
                       equal("日",str,arr[i+1].start, nextlen) ||
                       equal("时",str,arr[i+1].start, nextlen) ||
                       equal("分",str,arr[i+1].start, nextlen) ||
                       equal("秒",str,arr[i+1].start, nextlen) ||
                       equal("点",str,arr[i+1].start, nextlen)
                      ){
                        arr[i].pos = POS_T;
                    }
                }
                continue;
            }

            inext = utf8_next_estr(str, arr[i].start);
            if (inext == arr[i].end){
                arr[i].pos = POS_NX;
                // 判断是否是url
                int ilen = arr[i].end - arr[i].start;
                if (ilen > 5){
                    if(startswith(str, arr[i].start,"http:/") 
                            ||startswith(str,arr[i].start,"www.")
                            ||startswith(str,arr[i].start,"mail.")
                            ||endswith(str,arr[i].end,".com")
                            ||endswith(str,arr[i].end,".net")
                            ||endswith(str,arr[i].end,".cn")
                            ||endswith(str,arr[i].end,".org")
                            ||startswith(str,arr[i].start, "ftp://")
                      ){
                        arr[i].pos = POS_URL;
                    }
                }
                continue;
            }

            bool is_number = true;
            for(int j = arr[i].start; j < arr[i].end;){
                int next = utf8_char_len(str[j]) + j;
                if(!number_dict.exist(str, j, next)){
                    is_number = false;
                    break;
                }
                j = next;
            }
            if(is_number){
                arr[i].pos = POS_MC;
            }
        }

        int retLen = 0;
        for( int i = 0; i < len; i ++){
            if(arr[i].pos == POS_MC || arr[i].pos == POS_MA){
                arr[retLen].start = arr[i].start;
                for(int j = i+1; j < len&&arr[j].pos == arr[i].pos; j++){
                    i = j;
                }
                arr[retLen].end = arr[i].end;
            }else if (retLen < i){
                arr[retLen] = arr[i];
            }
            retLen ++;
        }

        return retLen;
    }

    void setDict(const Dictionary * pdict){
        dict = pdict;
    }

    void setPosDict(const Dictionary * pdict){
        pos_dict = pdict;
        tagger.setDict(pdict);
    }

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
    const Dictionary * pos_dict;
    string myname;
    Tagger tagger;
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
        setDict(refdict);
        setName("--* Lee's fly cutter: a forward max match tokenizer *--");
    }

    virtual int do_split(const string &strUtf8,int start,int endoff, Token * tokenArr) const 
    {
        int retLen = 0;
        for(int i = start; i < endoff; ) {
            int inext = utf8_next_alnum(strUtf8,i);
            if (inext > i) {
                tokenArr[retLen].start = i;
                tokenArr[retLen++].end = inext;
                i = inext;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < endoff ) {
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)) {
                    next = j;
                } else if(!dict->hasPrefix(strUtf8,i,j)) {
                    break;
                }
            }
            tokenArr[retLen].start = i;
            tokenArr[retLen++].end = next;
            i = next;
        }
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
class Renda: public Flycutter{
public:
    Renda(const Dictionary * refdict = NULL)
    {
        setDict(refdict);
        setName("--* renda: a backward max match tokenizer *--");
    }

    virtual int do_split(const string &strUtf8,int start,int endoff, Token * tokenArr) const 
    {
        string inversed_str = reverse_utf8(strUtf8,start,endoff);
        int len = Flycutter::do_split(inversed_str,0,endoff - start,tokenArr);

        // reverse all words's start and end offset.
        int mid = len / 2;
        int last = len - 1;
        int tmp = 0;
        for(int i = 0; i < mid; i ++){
            tmp = endoff - tokenArr[i].start;
            tokenArr[i].start = endoff -  tokenArr[last - i].end;
            tokenArr[last - i].end = tmp;
            tmp = endoff - tokenArr[i].end;
            tokenArr[i].end = endoff - tokenArr[last - i].start;
            tokenArr[last - i].start = tmp;
        }
        if(len % 2 != 0){ // handle the middle one 
            tmp = endoff - tokenArr[mid].start;
            tokenArr[mid].start = endoff - tokenArr[mid].end;
            tokenArr[mid].end = tmp;
        }
        return len;
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
        setDict(refdict);
        setName("--* paoding: a full words tokenizer *--");
    }

    virtual int do_split(const string &strUtf8,int start,int endoff, Token * tokenArr) const 
    {
        int best = -1;
        int retLen = 0;
        for(int i = start; i < endoff; ) {
            int inext = utf8_next_alnum(strUtf8,i);
            if (inext > i) {
                tokenArr[retLen].start = i;
                tokenArr[retLen++].end = inext;
                i = inext;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < endoff ) {
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)) {
                    best = j;
                    tokenArr[retLen].start = i;
                    tokenArr[retLen++].end = j;
                } else if(!dict->hasPrefix(strUtf8,i,j)) {
                    break;
                }
            }
            if(best <= i) {
                tokenArr[retLen].start = i;
                tokenArr[retLen++].end = next;
            }
            i = next;
        }
        return retLen;
    }
};

class Unigram:public IKnife
{
public:
    Unigram(const Dictionary * refdict = NULL)
    {
        setDict(refdict);
        setName("--* unigram: a unitary gram tokenizer *--");
    }

    virtual int do_split(const string &strUtf8,int start,int endoff, Token * tokenArr) const 
    {
        Graph graph;
        graph.gen(*dict, strUtf8,start,endoff);
        graph.calcLogProb(dict->getWordFreq(WORDS_FREQ_TOTAL));
#ifdef NSHORTPATH
        return NShortPath::getBestPath(graph, tokenArr, 6);
#else
        return ShortPath::getBestPath(graph, tokenArr);
#endif
    }
};

}

