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
#include "dict.hpp"
#include "utils.hpp"
#include <math.h>

using namespace std;

namespace mingspy
{
const int TYPE_ESTR = -1000;
const int TYPE_ATOM = -1;
const int TYPE_IN_DICT = -2000;

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
        if (_ended) return rows.size() - 1;
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
            if ((info = dict.getFreqInfo(strUtf8,graph.getOff(i),graph.getOff(j))) != NULL){
                graph.addChip(Chip(i,j,TYPE_IN_DICT,info->sum()));
            } else if(!dict.hasPrefix(strUtf8,graph.getOff(i),graph.getOff(j))){ break; }
        }
    }
#ifdef DEBUG
    graph.print();
#endif
}

struct Paths{
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
        int rsize = _graph.rowSize();
        for (int i = 0; i < rsize; i++){
            vector<Chip > & nodes =  _graph.getChips(i);
            for(int j = 0; j < nodes.size(); j++){
                assert(nodes[j]._start == i);
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
            sort(endpath._paths.begin(),endpath._paths.end(), chip_compare_asc);
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
            mingspy::print(it->second._paths);
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
        graph.calcWeights(dict->getTotalFreq());
        graph.end();
        NShortPath npath(graph, 8);
        npath.calc();
        npath.getBestPath(0,result);
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
        graph.calcWeights(dict->getTotalFreq());
        graph.end();
        NShortPath npath(graph, 8);
        npath.calc();
        npath.getBestPath(0,result);
    }

};
}
