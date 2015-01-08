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
#include <algorithm>
#include "dict.hpp"
#include "utils.hpp"

using namespace std;

namespace mingspy
{
const int TYPE_ESTR = -1000;
const int TYPE_IN_DICT = -2000;

struct Token {
public:
    int _attr; // type of this token or attribute index.
    int _start; // start index.
    int _end; // len
    //wstring _word; // maybe empty
    Token(int start = 0, int end = 0, int attr = -1) :_start(start),_end(end),_attr(attr) {
    }

	friend ostream & operator<<(ostream & out, const Token & t) {
        out<<"("<<t._start<<","<<t._end<<","<<t._attr<<")";
		return out;
    }

    inline bool operator==(const Token& other) const{
        return _start == other._start && _end == other._end && _attr == other._attr;
    }
};

void output(const vector<Token> & tokens){
    for(int i = 0; i< tokens.size(); i++) {
        cout<<tokens[i];
    }
    cout<<endl;
}
void output(const string & str, const vector<Token> & tokens)
{
    for(int i = 0; i< tokens.size(); i++) {
        cout<<"'"<<str.substr(tokens[i]._start,tokens[i]._end - tokens[i]._start)<<"'  ";
    }
    cout<<endl;
}

void genWordGraph(const Dictionary & dict,const string &strUtf8, Matrix & graph)
{
    int len = strUtf8.length();
    Dictionary::FreqInfo *info = NULL;
    int row = 0;
    for(int i = 0; i < len; ) {
        int estr = utf8_next_estr(strUtf8,i);
        if (estr > i) {
            graph[row++].setAttrVal(estr,100);
            i = estr;
            continue;
         }
         int j = i + utf8_char_len(strUtf8[i]);
         int next = j;
         while( j < len ) {
            if ((info = dict.getFreqInfo(strUtf8,i,j)) != NULL){
                next = j;
                graph[row].setAttrVal(j,info->sum());
            } else {
                if(!dict.hasPrefix(strUtf8,i,j)&&j!=next){ break; }
                graph[row].setAttrVal(j,5);
            }
            j += utf8_char_len(strUtf8[j]);
        }
        i = next;
        row++;
    }
}

class IKnife
{
public:
    virtual ~IKnife(){}
    /*
     * split out the str, then save the (start,end) into vector.
     * @param strUtf8 : the input str to split.
     * @param result : result words
     */
    virtual void split(const string &strUtf8, vector<Token> & result) = 0;
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

    virtual void split(const string &strUtf8, vector<Token> & result)
    {
        int len = strUtf8.length();
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i){
                result.push_back(Token(i,estr,TYPE_ESTR));
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
            result.push_back(Token(i,next));
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

    virtual void split(const string &strUtf8, vector<Token> & result)
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
			result.push_back(Token(len - pos[next], len - pos[i]));
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

    virtual void split(const string &strUtf8, vector<Token> & result)
    {
        int len = strUtf8.length();
        int best = -1;
        for(int i = 0; i < len; ) {
            int estr = utf8_next_estr(strUtf8,i);
            if (estr > i){
                result.push_back(Token(i,estr,TYPE_ESTR));
                i = estr;
                continue;
            }
            int j = i + utf8_char_len(strUtf8[i]);
            int next = j;
            while( j < len ){
                j += utf8_char_len(strUtf8[j]);
                if(dict->exist(strUtf8,i,j)){ 
                    best = j;
                    result.push_back(Token(i,j));
                } else if(!dict->hasPrefix(strUtf8,i,j)){ break; }
            }
            if(best <= i){ result.push_back(Token(i,next)); }
            i = next;
        }
    }
};
}
