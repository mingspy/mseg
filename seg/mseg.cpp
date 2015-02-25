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

#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include "utils.hpp"
#include "dict.hpp"
#include "knife.hpp"
#include "config.hpp"
#include "mseg.h"
#include "utils.hpp"

using namespace std;
using namespace mingspy;

static Dictionary core_dict;
static Dictionary inverse_dict;
static Flycutter flycutter;
static Renda renda;
static Paoding paoding;
static Unigram unigram;
/**
 *
 * over_ride 如果为false, 并且不存在词性，并且词在词典中存在，则不添加
 *           如果为true,不存在词性，则把词性设置为UDF(未定义)
 */
static void mseg_load_user_dict(Dictionary & dict, const string & file, bool is_reverse = false, bool over_ride = false)
{
    // 词\t词频
    LineFileReader reader(file);
    string *line = NULL;
    while(line = reader.getLine()) {
        vector<string> vec;
        split(*line, "\t",vec);
        if(vec.size() == 0) continue;
        if (is_reverse){
            reverse(vec[0].begin(),vec[0].end());
        }
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


void mseg_config_init(const char * config_path){
    Config::instance().loadSettingsFromConf(config_path);
}

void mseg_config_set(const char * key, const char * val){
    Config::instance().set(key,val);
}

void mseg_init(){
    Config & config = Config::instance();
    core_dict.open(config.getStr(KEY_CORE_PATH));
    if (config.getBool(KEY_ISLOAD_INVS)){
        inverse_dict.open(config.getStr(KEY_INVS_PATH));
	}

    string udfpath = config.getStr(KEY_UDF_DICT_PATH);
    vector<string> files;
    if (fileExist(udfpath.c_str())){
        if(isFile(udfpath.c_str())){
            files.push_back(udfpath);
        }else if(isDir(udfpath.c_str())){
            listFiles(udfpath,files);
        }
    }

    for (int i = 0; i < files.size(); i ++){
        mseg_load_user_dict(core_dict, files[i], false, false);
        if (config.getBool(KEY_ISLOAD_INVS)){
            mseg_load_user_dict(inverse_dict, files[i], true, false);
	    }
    }

	flycutter.setDict(&core_dict);
	paoding.setDict(&core_dict);
	unigram.setDict(&core_dict);
    renda.setDict(&inverse_dict);
}

int mseg_forward_split(const char * str, struct Token * result, int result_len){
	return flycutter.split(str,result,result_len);
}

int mseg_backward_split(const char * str, struct Token * result, int result_len){
	return renda.split(str,result,result_len);
}

int mseg_smart_split(const char * str, struct Token * result, int result_len){
	return unigram.split(str,result,result_len);
}

int mseg_full_split(const char * str, struct Token * result, int result_len){
	return paoding.split(str,result,result_len);
} 

int mseg_tagging(const char * str, struct Token * result, int result_len){
}

const char * mseg_get_pos(int id){
	return core_dict.getWord(id);
}

