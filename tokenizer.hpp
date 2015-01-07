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
    Token(int start = 0, int end = 0, int attr = -1)
        :_start(start),_end(end),_attr(attr)
    {
    }
	friend ostream & operator<<(ostream & out, const Token & t)
    {
        out<<"("<<t._start<<","<<t._end<<","<<t._attr<<")";
		return out;
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

class ITokenizer
{
public:
    /*
    * according dictionary, forward split the str. From the head char
    * of input, this method will lookup the dictionary, then take the
    * @param str : the input str to split.
    * @param result : result words
    */
    virtual void split(const string &str, vector<Token> & result) = 0;
	void setDict(Dictionary * pdict){dict = pdict;}
    void setName(const string & name){myname = name;}
    string getName() const {return myname;}
protected:
	Dictionary * dict;
    string myname;
};

class MaxMatcher: public ITokenizer
{
public:
    MaxMatcher(Dictionary * refdict = NULL)
    {
        setDict(refdict);
        setName("max match tokenizer");
    }

    /*
    * according dictionary, forward split the str. From the head char
    * of input, this method will lookup the dictionary, then take the
    * longest word from the str step by step.
    * For example: "AB" "ABCD" "BCD" "BCDE" "EF" are
    * words in dictionary; when given str="ABCDEF", the split result
    * will be: ABCD/ EF/
    * @param str : the input str to split.
    * @param result : result words
    */
    virtual void split(const string &str, vector<Token> & result)
    {
		int len = str.length();
        for(int i = 0; i < len; ) {
			int estr = utf8_next_estr(str,i);
			if (estr > i){
				result.push_back(Token(i,estr,TYPE_ESTR));
				i = estr;
				continue;
			}
			int j = i+utf8_char_len(str[i]);
			int next = j;
			while( j < len ){
				j += utf8_char_len(str[j]);
				if(dict->exist(str,i,j)){
					next = j;
				}else if(!dict->hasPrefix(str,i,j)){
					break;
				}
			}
			result.push_back(Token(i,next));
            i = next;
        }
    }
};
class InverseTokenizer: public ITokenizer
{
public:
    InverseTokenizer(Dictionary * refdict = NULL)
    {
        setDict(refdict);
        setName("inverse tokenizer");
    }

    /*
    * according dictionary, forward split the str. From the head char
    * of input, this method will lookup the dictionary, then take the
    * longest word from the str step by step.
    * For example: "AB" "ABCD" "BCD" "BCDE" "EF" are
    * words in dictionary; when given str="ABCDEF", the split result
    * will be: ABCD/ EF/
    * @param str : the input str to split.
    * @param result : result words
    */
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
			int j = i+utf8_char_len(strUtf8[i]);
            pos.push_back(len - j);
            i = j;
        }
        string str(strUtf8.rbegin(),strUtf8.rend());
        for(int i = pos.size() - 1; i > 0; ) {
			int next = i-1;
            for(int j = i-2; j >= 0; j--){
				if(dict->exist(str,pos[i],pos[j])){
					next = j;
				}else if(!dict->hasPrefix(str,pos[i],pos[j])){
					break;
                }
            }
			result.push_back(Token(len - pos[next], len - pos[i]));
            i = next;
        }
        reverse(result.begin(), result.end());
    }
};
}
