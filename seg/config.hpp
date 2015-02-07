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
#include <map>
#include "../util/guard.hpp"
using namespace std;

namespace mingspy
{
#if _MSC_VER > 1000  // windows
static const string DEFAULT_CONF_PATH = "D:/mseg/data/mseg.conf"; // default config path
static const string DEFAULT_CORE_DICT_PATH = "D:/mseg/data/core.dic"; // default core dictionary path
static const string DEFAULT_INVS_DICT_PATH = "D:/mseg/data/invs.dic"; // default inverse dictionary path
static const string DEFAULT_UDF_DICT_PATH = "D:/mseg/data/userDicts/";

#else  // linux
static const string DEFAULT_CONF_PATH = "/opt/apps/mseg/data/mseg.conf";
static const string DEFAULT_CORE_DICT_PATH = "/opt/apps/mseg/data/core.dic";
static const string DEFAULT_INVS_DICT_PATH = "/opt/apps/mseg/data/invs.dic";
static const string DEFAULT_UDF_DICT_PATH = "/opt/apps/mseg/data/userDicts/";

#endif
static const string DEFAULT_ISLOAD_INVS = "false";

static const string ENV_MSEG_CONF_PATH = "MSEG_CONF_PATH"; // seg config key in environment
// keys in config file :
static const string KEY_CONF_PATH = "CONF_PATH";
static const string KEY_CORE_PATH = "CORE_DICT_PATH";
static const string KEY_INVS_PATH = "INVS_DICT_PATH";
static const string KEY_UDF_DICT_PATH = "UDF_DICT_PATH";
static ResGuard _confGuard;

/*
* Utility to read Configs.
* The default Config path was set to $DEFAULT_CONF_PATH.
* user can put config under the path. Or Set environment mseg_CONF_PATH to
* let the Config know where the config file located.
*/
class Config
{
public:

    static const Config & instance()
    {
        ResGuard::Lock lock(_confGuard);
        static Config _theConf;
        return _theConf;
    }

    string getStr(const string & key)
    {
        if(_confs.find(key) != _confs.end()) {
            return _confs[key];
        }
        return "";
    }

    int getInt(const string & key, int default_val = 0)
    {
        if(_confs.find(key) != _confs.end()) {
            return atoi(_confs[key].c_str());
        }

        return default_val;
    }

    bool getBool(const string &key, bool default_val = false)
    {
        if(_confs.find(key) != _confs.end()) {
            string & val = _confs[key];
            return ((val == "TRUE") || (val == "true"));
        }

        return default_val;
    }

    double getDouble(const string & key, double default_val = 0)
    {
        if(_confs.find(key) != _confs.end()) {
            return atof(_confs[key].c_str());
        }

        return default_val;
    }
    void set(const string & key, const string & val){
        _confs[key] = val;
    }

    Config()
    {
        loadSettings();
    }
    config(const string & config_path){
        loadSettingsFromConf(path);
    }
private:
    void loadSettings()
    {
        setDefaultConfigs();
        char * pconf = getenv(ENV_mseg_CONF_PATH.c_str());
        if(pconf != NULL) {
            _confs[KEY_CONF_PATH] = pconf;
        }
        loadSettingsFromConf(_confs[KEY_CONF_PATH]);
    }

    void loadSettingsFromConf(const string & path)
    {
        LineFileReader reader(path);
        string * line = NULL;
        while((line = reader.getLine()) != NULL) {
            if(line->at(0) == '#') {
                continue;
            }
            int idx = line->find_first_of('=');
            if(idx != string::npos) {
                string key = trim(line->substr(0, idx));
                string val = trim(line->substr(idx+1));
                _confs[key] = val;
            }
        }
    }

    void setDefaultConfigs()
    {
        _confs[KEY_CONF_PATH] = DEFAULT_CONF_PATH;
        _confs[KEY_CORE_PATH] = DEFAULT_CORE_DICT_PATH;
        _confs[KEY_INVS_PATH] = DEFAULT_INVS_DICT_PATH;
        _confs[KEY_ISLOAD_INVS] = DEFAULT_ISLOAD_INVS;
        _confs[KEY_UDF_DICT_PATH] = DEFAULT_UDF_DICT_PATH;
    }
private:
    map<string, string> _confs;

};
}

