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
#include <cstring>
#include <vector>
#include <exception>
#include <algorithm>
#include "dict.hpp"
#include "../util/utils.hpp"

using namespace std;

namespace mingspy
{

struct PeopleEntity {
    string pos;
    string entpos;
    string word;
    vector<PeopleEntity> sub;
    bool isCompose;
    bool isBegin;
    bool isEnd;
    PeopleEntity():isCompose(false),isBegin(false),isEnd(false) {}
};

class parse_error
{
    string _msg;
public:
    parse_error(const string & msg):_msg(msg)
    {
    }

    friend ostream&operator<<(ostream & out,const  parse_error & e)
    {
        out<<e._msg;
        return out;
    }
};
inline bool isPos(const string & s)
{
    const char * p = s.c_str();
    while(*p) {
        if (!isalpha(*p)) {
            return false;
        }
        p ++;
    }
    return true;
}

PeopleEntity parseEntity(string & t)
{
    int chinese_len =  string("中").length();
    PeopleEntity ent;
    int idx = t.find('/');
    if (t.find("http://") != string::npos)
        idx = t.rfind('/');
    if (idx == string::npos)
        throw parse_error("format error dont found / ");
    ent.word = t.substr(0,idx);
    ent.pos = t.substr(idx+1);
    if(ent.word.empty() && ent.pos.empty())
        throw parse_error("null word or role:");
    if (ent.word == "" && ent.pos == "/w") {
        ent.word = "/";
        ent.pos = "w";
    } else if(ent.word == ""&& ent.pos == "w")
        ent.word = " ";
    if (ent.word.length() > 1 && ent.word[0] == '[' ) {
        ent.word = ent.word.substr(1);
        ent.isBegin = true;
    }
    int ridx = ent.pos.find("]");
    if (ridx >= 0) {
        ent.isEnd = true;
        ent.entpos = ent.pos.substr(ridx+1);
        if (ent.entpos[0] == '/')
            ent.entpos = ent.entpos.substr(1);
        ent.pos = ent.pos.substr(0,ridx);
    }
    int didx = ent.pos.find('/');
    if (didx >= 0) {
        string role1 = ent.pos.substr(0,didx);
        string role2 = ent.pos.substr(didx+1);
        bool is_r1_pos = isPos(role1);
        bool is_r2_pos = isPos(role2);
        if (isPos(role1))
            ent.pos = role1;
        else if (isPos(role2)) {
            ent.pos = role2;
            ent.word += '/'+role1;
        }
    }

    if (ent.word.length() > chinese_len) {
        int wlen = ent.word.length();
        if (isPunc(ent.word.substr(0,3)) || isPunc(ent.word.substr(0,1)) 
            || isPunc(ent.word.substr(wlen - 1)) || isPunc(ent.word.substr(wlen -1))){
                throw parse_error("has punction");
        }
    }
    return  ent;

}

bool parsePeopleEntities(vector<string> & tokenStrList,vector<PeopleEntity> & vec)
{
    PeopleEntity * compose_entity = NULL;
    for (int i = 0; i < tokenStrList.size(); i++) {
        if(tokenStrList[i].empty() || tokenStrList[i] == " ") {
            continue;
        }
        PeopleEntity ent = parseEntity(tokenStrList[i]);
        if (ent.word.empty() and ent.pos.empty())
            throw parse_error("parse token failed");
        if ((ent.isBegin && compose_entity) || (ent.isEnd && !compose_entity))
            throw parse_error("composed entity checking error");
        if (ent.isBegin) {
            vec.push_back(PeopleEntity());
            compose_entity = &vec[vec.size() -1];
            compose_entity->isCompose = true;
        }
        if(compose_entity) {
            compose_entity->sub.push_back(ent);
        } else {
            vec.push_back(ent);
        }
        if (ent.isEnd) {
            compose_entity->entpos = ent.entpos;
            compose_entity = NULL;
        }
    }
    return true;
}

class Builder
{
    int start_word_id;
    int start_pos_id;
    int words_total;
    int pos_total;
    Dictionary dict;
    int errors;
    bool inverse;
    void add(string & word, string & pos)
    {
        dict.setMaxWordId(start_pos_id);
        dict.addWord(pos);
        start_pos_id = dict.getMaxWordId();
        dict.setMaxWordId(start_word_id);
        if(inverse){
            reverse(word.begin(),word.end());
        }
        dict.addWord(word);
        start_word_id = dict.getMaxWordId();
        dict.addAttrFreq(word,pos,1);
        words_total += 1;
        pos_total += 1;
    }
public:
    Builder(int pos_id=0,int word_id=1000):start_pos_id(pos_id),
        start_word_id(word_id),pos_total(0),words_total(0),errors(0),inverse(false) {}
    bool buildFromPeopleDaily(const vector<string> &files, const string & output)
    {
        Timer timer;
        int line_count = 0;
        for(int i = 0; i< files.size(); i++) {
            if(!endswith(files[i],".txt")) continue;
            LineFileReader reader(files[i]);
            string * line;
            // 处理词信息
            while((line = reader.getLine()) != NULL) {
                procALine(*line);
                if(++line_count%100 == 0) {
                    cout<<"\rparsed lines -> "<<line_count;
                }
            }
        }
        dict.addAttrFreq(POS_FREQ_TOTAL,-1,pos_total);
        dict.addAttrFreq(WORDS_FREQ_TOTAL,-1,words_total);

        double load_word_end_time = timer.elapsed();
        cout<<"\n---------------------------------------------------------------"<<endl
            <<"parsed total lines:"<<line_count<<" used:"<<(load_word_end_time)
            <<"s\nerrors="<<errors<<endl
            <<"total words:"<<start_word_id - 1000<<" total pos:"<<start_pos_id<<endl;
        bool result = dict.save(output);
        double end_time = timer.elapsed();
        Dictionary d2;
        d2.open(output);
        double open_end_time = timer.elapsed();
        cout<<"build finished! total used:"<<(end_time)
            <<"s\ndict save used:"<<(end_time - load_word_end_time)
            <<"s\ndict open used:"<<(open_end_time - end_time)<<endl;
        return result;
    }

    void procALine(string & line)
    {
        vector<string> tokens;
        split(line," ",tokens);
        vector<PeopleEntity> vec;
        try {
            parsePeopleEntities(tokens,vec);
            // word - frequency
            for(int i = 0; i < vec.size(); i++) {
                PeopleEntity & ent = vec[i];
                if (ent.isCompose) {
                    for(int j = 0; j < ent.sub.size(); j++) {
                        add(ent.sub[j].word, ent.sub[j].pos);
                    }
                } else {
                    add(ent.word,ent.pos);
                }
            }
            // pos-pos - frequency
            for(int i = 0; i < vec.size() - 1; i++) {
                PeopleEntity & ent = vec[i];
                string next =  vec[i+1].pos;
                if(vec[i+1].isCompose){
                    next = vec[i+1].sub[0].pos;
                }

                string pre = vec[i].pos;
                if (ent.isCompose) {
                    pre = ent.sub[0].pos;
                    for(int j = 1; j < ent.sub.size(); j++){
                        dict.addAttrFreq(pre,ent.sub[j].pos, 1);
                        pre = ent.sub[j].pos;
                    }
                }
                dict.addAttrFreq(pre,next, 1);
            }
        } catch(parse_error e) {
            errors ++;
            cerr<<e<<" \""<<line<<"\""<<endl;
        }
    }
    void setInverse(bool isInverse){inverse = isInverse;}
};
}

