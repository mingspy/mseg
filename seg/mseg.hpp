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
#include <cstdlib>
#include <string>
#include "../util/utils.hpp"
#include "dict.hpp"
#include "knife.hpp"

using namespace std;

namespace mingspy
{
/**
 *
 * over_ride 如果为false, 并且不存在词性，并且词在词典中存在，则不添加
 *           如果为true,不存在词性，则把词性设置为UDF(未定义)
 */
void mseg_load_user_dict(Dictionary & dict, const string & file, bool over_ride = false)
{
    // 词\t词频
    LineFileReader reader(file);
    string *line = NULL;
    while(line = reader.getLine()) {
        vector<string> vec;
        split(*line, "\t",vec);
        if(vec.size() == 0) continue;
        string * word = &vec[0];
        int freq = 1;
        const string * pos = NULL;
        if(vec.size() >= 2)
            freq = atoi(vec[1].c_str());
        if(vec.size() == 3)
            pos = &vec[2];
        if (!over_ride && dict.exist(*word)) continue;
        if (!pos)
            pos = &UDF;
        dict.addAttrFreq(*word,*pos,freq);

    }
}

void mseg_split(const IKnife & knif, const string & utf8_str, vector<Chip> & chips)
{
    int split_len = 120;
    int slen = utf8_str.length();
    if (slen <= split_len) {
        knif.split(utf8_str, chips);
        return;
    }
    static string puncs[] = {"。",",","，","!"," ","\n","！"};
    int puncs_size = 7;
    // sentance split to litte sentance
    int start = 0;
    int end = split_len;
    while(end < slen) {
        int nxt = end + utf8_char_len(utf8_str[end]);
        for(int k = 0; k< puncs_size; k ++) {
            if (equal(puncs[k], utf8_str, end, nxt)) {
                vector<Chip> result;
                knif.split(utf8_str.substr(start, nxt - start),result);
                for(int m = 0; m < result.size(); m++) {
                    result[m]._start += start;
                    result[m]._end += start;
                    chips.push_back(result[m]);
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
        vector<Chip> result;
        knif.split(utf8_str.substr(start, slen - start),result);
        for(int m = 0; m < result.size(); m++) {
            result[m]._start += start;
            result[m]._end += start;
            chips.push_back(result[m]);
        }
    }
}

}
