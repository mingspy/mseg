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
    while((line = reader.getLine())!= NULL) {
        vector<string> vec;
        split(*line, "\t",vec);
        if(vec.size() == 0) continue;
        if (is_reverse){
            vec[0] = reverse_utf8(vec[0]);
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
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " start --------------"<<endl;
    Config::instance().loadSettingsFromConf(config_path);
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " end --------------"<<endl <<endl;
}

void mseg_config_set(const char * key, const char * val){
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " start --------------"<<endl;
    Config::instance().set(key,val);
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " end --------------"<<endl <<endl;
}

void mseg_init(){
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " start --------------"<<endl;
    Config & config = Config::instance();
    string dictRoot = config.getStr(KEY_DICT_ROOT);
    cerr<<"get DICT_ROOT="<<dictRoot.c_str()<<endl;
    if (!fileExist(dictRoot.c_str())){
        cerr<<"error: DICT_ROOT not accessable:"<<dictRoot.c_str()<<endl;
        throw "dictionary dir of mseg not accessable.";
    }

    string core_dict_path = combinPath(dictRoot,config.getStr(KEY_CORE_NAME));
    cout<<"core dict path is:"<<core_dict_path<<endl;
    if (!fileExist(core_dict_path.c_str())){
        cerr<<"error: core dict not accessable:"<<core_dict_path.c_str()<<endl;
        throw "core dict not accessable";
    }

    core_dict.open(core_dict_path);
    if (config.getBool(KEY_ISLOAD_INVS)){
        string invs_dict_path = combinPath(dictRoot,config.getStr(KEY_INVS_NAME));
        cout<<"inverse dict path is:"<<invs_dict_path<<endl;
        if (!fileExist(invs_dict_path.c_str())){
            cerr<<"error: inverse dict not accessable:"<<invs_dict_path.c_str()<<endl;
            throw "inverse dict not accessable";
        }
        inverse_dict.open(invs_dict_path);
	}

    string udfpath = config.getStr(KEY_USER_DICT_DIR);
    vector<string> files;
    if (fileExist(udfpath.c_str())){
        if(isFile(udfpath.c_str())){
            files.push_back(udfpath);
        }else if(isDir(udfpath.c_str())){
            listFiles(udfpath,files);
        }
    }

    const int fSize = files.size();
    for (int i = 0; i < fSize; i ++){
        mseg_load_user_dict(core_dict, files[i], false, false);
        if (config.getBool(KEY_ISLOAD_INVS)){
            mseg_load_user_dict(inverse_dict, files[i], true, false);
	    }
    }

	flycutter.setDict(&core_dict);
	paoding.setDict(&core_dict);
	unigram.setDict(&core_dict);
    renda.setDict(&inverse_dict);
	flycutter.setPosDict(&core_dict);
	paoding.setPosDict(&core_dict);
	unigram.setPosDict(&core_dict);
    renda.setPosDict(&inverse_dict);
    cerr<<__DATE__<<" "<<__TIME__<<"|" <<__FILE__<<":"<<__LINE__<<" "<<__func__<< " end --------------"<<endl <<endl;
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
	return unigram.split(str,result,result_len, SPLIT_FLAG_POS);
}

const char * mseg_get_pos(int id){
	return core_dict.getWord(id);
}

