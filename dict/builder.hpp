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
#include "utils.hpp"

using namespace std;

namespace mingspy
{

struct PeopleEntity {
    string pos; // 词性
    string entpos; // 组合词词性
    string word; // 词
    vector<PeopleEntity> sub; // 如果是组合词，那么所以组合都放入sub中
    bool isCompose; // 是否组合词
    bool isBegin; // 是否组合词开始
    bool isEnd; // 是否组合词结束
    PeopleEntity():isCompose(false),isBegin(false),isEnd(false) {}
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
    static int chinese_len =  string("中").length();
    PeopleEntity ent;
    int idx = t.find('/');
    if (t.find("http://") != string::npos)
        idx = t.rfind('/');
    if (idx == string::npos)
        throw parse_error("format error dont found / ",(void *)"builder.hpp Line 66");
    ent.word = t.substr(0,idx);
    ent.pos = t.substr(idx+1);
    if(ent.word.empty() && ent.pos.empty())
        throw parse_error("null word or role:",(void *)ent.word.c_str());
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
                || isPunc(ent.word.substr(wlen - 1)) || isPunc(ent.word.substr(wlen -1))) {
            throw parse_error("has punction",(void *)"end");
        }
    }
    return  ent;

}

// 分析一段训练文本，解析成词/词性列表。组合词会外包一层PeopleEntity,属性设置
// 成组合词，词内容为所有子词的集合
bool parsePeopleEntities(vector<string> & tokenStrList,vector<PeopleEntity> & result_vec)
{
    vector<PeopleEntity> vec;
    PeopleEntity * compose_entity = NULL;
    for (int i = 0; i < tokenStrList.size(); i++) {
        if(tokenStrList[i].empty() || tokenStrList[i] == " ") {
            continue;
        }
        PeopleEntity ent = parseEntity(tokenStrList[i]);
        if (ent.word.empty() and ent.pos.empty())
            throw parse_error("parse token failed",(void *)"parsePeopleEntities");
        if ((ent.isBegin && compose_entity) || (ent.isEnd && !compose_entity))
            throw parse_error("composed entity checking error",(void *)"parsePeopleEntities");
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
            compose_entity->pos = ent.entpos;
            for(int i = 0; i < compose_entity->sub.size(); i++){
                compose_entity->word += compose_entity->sub[i].word;
            }
            compose_entity = NULL;
        }
    }

    // 人名合并：
    for(int i = 0; i < vec.size(); ){
        if(i < (vec.size() - 1 )&&vec[i].pos == "nr" && vec[i+1].pos == "nr"){
            // 一个utf8汉字占3个字节
            if(vec[i].word.length() == 3 && vec[i+1].word.length() <= 6){
                result_vec.push_back(PeopleEntity());
                PeopleEntity &ent = result_vec[result_vec.size() -1];
                ent.isCompose = true;
                ent.pos = "nr";
                ent.sub.push_back(vec[i]);
                ent.sub.push_back(vec[i+1]);
                ent.word += vec[i].word;
                ent.word += vec[i+1].word;
                i += 2;
                continue;
            }
        }
        result_vec.push_back(vec[i]);
        i += 1;
    }
    return true;
}

// 命名实体识别角色统一定义如下
// name Entity roles: 
// A - prefix 上文
// B - begin 开始
// C - conject 中间
// D - end 结束
// E - posfix 后缀
// M - middle 实体之间连词
// O - others 其他
// S - 单独成词 
class Builder
{
    int words_total;
    int pos_total;
    Dictionary dict;
    Dictionary person_dict;
    int errors;
    bool inverse;
    void add(string word, const string & pos, int freq = 1)
    {
        if(inverse) {
           word = reverse_utf8(word);
        }
        dict.addWord(word);
        if(!inverse) {
            dict.addAttrFreq(word,pos,freq);
        }else{
            dict.addAttrFreq(word,-1,freq);
        }
        words_total += 1;
        pos_total += 1;
    }
public:
    Builder():pos_total(0),words_total(0),errors(0),inverse(false) {
            load_pos();
            add_delimiters();
            person_dict.addWord("A");
            person_dict.addWord("B");
            person_dict.addWord("C");
            person_dict.addWord("D");
            person_dict.addWord("E");
            person_dict.addWord("M");
            person_dict.addWord("O");
            person_dict.addWord("S");
        }
    bool buildFromPeopleDaily(const vector<string> &files)
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
        cout<<POS_FREQ_TOTAL<<" get pos total"<<dict.getWordFreq(POS_FREQ_TOTAL)<<" get attr total"<<dict.getAttrFreq(POS_FREQ_TOTAL,-1)<<endl;
        //dict.addAttrFreq("一一列举","UDF",1);
        cout<<POS_FREQ_TOTAL<<" get pos total"<<dict.getWordFreq(POS_FREQ_TOTAL)<<" get attr total"<<dict.getAttrFreq(POS_FREQ_TOTAL,-1)<<endl;
        cout<<"一一列举 get pos total"<<dict.getWordFreq("一一列举")<<" get attr total"<<dict.getAttrFreq("一一列举",-1)<<endl;

        double load_word_end_time = timer.elapsed();
        cout<<"\n---------------------------------------------------------------"<<endl
            <<"parsed total lines:"<<line_count<<" used:"<<(load_word_end_time)
            <<"s\nerrors="<<errors<<endl
            <<"total words:"<<words_total<<" total pos:"<<pos_total<<endl;
        double end_time = timer.elapsed();
        cout<<"build finished! total used:"<<(end_time) <<endl;
        return true;
    }

    void save(const string & output){
        cout<<"\n-------------------------saving dicts-----------------------------------"<<endl
            <<"total words:"<<words_total<<" total pos:"<<pos_total<<endl;
        cout<<POS_FREQ_TOTAL<<" get pos total"<<dict.getWordFreq(POS_FREQ_TOTAL)<<" get attr total"<<dict.getAttrFreq(POS_FREQ_TOTAL,-1)<<endl;
        cout<<"一一列举 get pos total"<<dict.getWordFreq("一一列举")<<" get attr total"<<dict.getAttrFreq("一一列举",-1)<<endl;
        dict.save(output);
        if (!inverse) person_dict.save("./person.dic");
    }

    void load_udf_dict(const string & path, const string & type){
        LineFileReader reader(path);
        string *line = NULL;
        const int PERSON = 1;
        int dict_type = 0;
        if (type == "nr"){
            dict_type = PERSON;
        }

        int line_count = 0;
        while((line = reader.getLine()) != NULL) {
            if (type !=  "UDF" || dict.getWordId(*line) > -1 )
                add(*line,type);
            if(++line_count%100 == 0) {
                cout<<"\rparsed lines -> "<<line_count;
            }
            if(!inverse&&dict_type == PERSON){
                person_dict.addAttrFreq(*line,type,1);
                for(int i = 0; i < line->size();){
                    int next = utf8_char_len((*line)[i]);
                    string word = line->substr(i,next);
                    if(i == 0){
                        person_dict.addAttrFreq(word,"B",1);
                    }else if(i + next == line->size()){
                        person_dict.addAttrFreq(word,"D",1);
                    }else{
                        person_dict.addAttrFreq(word,"C",1);
                    }
                    i += next;
                }
            }
        }
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
                    // Add compose word to dict
                    add(ent.word, ent.pos);
                } else {
                    add(ent.word,ent.pos);
                }
            }
            // pos-pos - frequency
            for(int i = 0; i < vec.size() - 1; i++) {
                PeopleEntity & ent = vec[i];
                string next =  vec[i+1].pos;
                if(vec[i+1].isCompose) {
                    next = vec[i+1].sub[0].pos;
                }

                string pre = vec[i].pos;
                if (ent.isCompose) {
                    pre = ent.sub[0].pos;
                    for(int j = 1; j < ent.sub.size(); j++) {
                        dict.addAttrFreq(pre,ent.sub[j].pos, 1);
                        pre = ent.sub[j].pos;
                    }
                }
                dict.addAttrFreq(pre,next, 1);
            }

            // person name
            string prev = "";
            for(int i = 0; i < vec.size(); i++){
                PeopleEntity & ent = vec[i];
                string role = "O";
                if(ent.pos == "nr"){
                    role = "S";
                    string tmp_prev = prev;
                    for(int i = 0; i < ent.word.length(); ){
                        string tmp_role = "C";
                        int next  = utf8_char_len(ent.word[i]);
                        if (i == 0){
                            tmp_role = "B";
                        }
                        else if (i + next == ent.word.length()){
                            tmp_role = "D";
                        }
                        person_dict.addAttrFreq(ent.word.substr(i,3),tmp_role,1);
                        if(!tmp_prev.empty()){
                            person_dict.addAttrFreq(tmp_prev,tmp_role,1);
                            if(tmp_prev == "S"){
                                person_dict.addAttrFreq("D",tmp_role,1);
                            }
                        }
                        tmp_prev = tmp_role;
                        i += next;
                    }

                    if (ent.word.length() / 3 == 2){
                        person_dict.addAttrFreq("BD",-1,1);
                    }else{
                        person_dict.addAttrFreq("BCD",-1,1);
                    }
                }else{
                    if (i > 0 && vec[i-1].pos == "nr"){
                        role = "E";
                    }

                    if (i < vec.size() - 1 && vec[i].pos == "nr"){
                        if(role == "E"){
                            role = "M";
                        } else{
                            role = "A";
                        }
                    }
                }

                person_dict.addAttrFreq(ent.word,role,1);
                if(prev != ""){
                    person_dict.addAttrFreq(prev,role,1);
                    if(prev == "S"){
                        person_dict.addAttrFreq("D",role,1);
                    }
                }
                prev = role;
            }
        } catch(parse_error e) {
            errors ++;
            cerr<<e.what()<<" \""<<line<<"\""<<endl;
        }
    }
    void setInverse(bool isInverse)
    {
        inverse = isInverse;
    }
private:
    void load_pos(){
        LineFileReader reader("./pos_tag_id.txt");
        string *line = NULL;
        while((line = reader.getLine()) != NULL) {
            vector<string> vec;
            split(*line," ",vec);
            dict.addWord(vec[0], atoi(vec[1].c_str()));
        }
    }

    void add_delimiters(){
        string delimiters = ",.?<>[]()（），。？；：’”“‘【】《》！、|·~;'\"@$^&=-+{}! \r\n\t";
        for(int i = 0; i < delimiters.length();){
            int len = utf8_char_len(delimiters[i]);
            add(delimiters.substr(i,len),"w",10000);
            i += len;
        }
    }
};
}

