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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>

#if _MSC_VER > 1000
#include <io.h>
#else
#include<sys/types.h>
#include <dirent.h>
#endif


using namespace std;

inline bool isDir(const char * path){
    struct stat info;
    if (stat(path,&info) == 0)
        return S_ISDIR(info.st_mode);
    perror(path);
    return false;
}

inline bool isFile(const char * path){
    struct stat info;
    if (stat(path,&info) == 0)
        return S_ISREG(info.st_mode);
    perror(path);
    return false;
}

inline size_t fileSize(const char * path)
{
    struct stat info;
    if (stat(path,&info) == 0)
        return info.st_size;
    perror(path);
    return 0;
}

inline bool fileExist(const char * path){
    return access(path,0) == 0;
}

string changeToInnerPath(const string & path)
{
    string p = path;
    replace(p.begin(),p.end(), '\\', '/');
    return p;
}

string ensureDir(const string & path)
{
    string p = changeToInnerPath(path);
    if(p.find_last_of('/') != p.length() - 1) {
        p.append("/");
    }
    return p;
}

string combinPath(const string & path, const string & path2)
{
    string r1 = ensureDir(path);
    string r2 = path2;
    replace(r2.begin(),r2.end(), '\\', '/');
    if(r2.at(0) == '/') {
        r2.erase(0, 1);
    }
    r1.append(r2);
    return r1;
}

void listFiles( const string & path, vector<string>& files ,bool recursive = false)
{
    if(path.empty()) {
        return;
    }

    string pathInner = changeToInnerPath(path);
    if(pathInner.at(pathInner.length() - 1) == '/') {
        pathInner.erase(pathInner.length() - 1);
    }
#if _MSC_VER > 1000
    long   hFile   =   0;
    struct _finddata_t fileinfo;
    string p;
    if((hFile = _findfirst(p.assign(pathInner).append("/*").c_str(),&fileinfo)) !=  -1) {
        do {
            if((fileinfo.attrib &  _A_SUBDIR)) {
                if(recursive && strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
                    listFiles( p.assign(pathInner).append("/").append(fileinfo.name), files );
            } else {
                files.push_back(p.assign(pathInner).append("/").append(fileinfo.name));
            }
        } while(_findnext(hFile, &fileinfo)  == 0);
        _findclose(hFile);
    }
#else
    DIR * pDir = opendir(path.c_str());
    if(pDir == NULL) {
        return;
    }

    dirent * ent = NULL;
    while((ent = readdir(pDir)) != NULL) {
        if(ent->d_type & DT_DIR) {
            if(recursive && strcmp(ent->d_name,".")!=0 && strcmp(ent->d_name,"..")!=0)
                listFiles(pathInner+"/"+ent->d_name, files);
        } else {
            files.push_back(pathInner+"/"+ent->d_name);
        }
    }
    closedir(pDir);

#endif
}


/*
* Split the given @src by @separator, the @separator will not appear in result.
* @param src - the string to be split.
* @param separator - the separator.
* @param result - the split pieces of string.
*/
void split(const string& src, const string& separator, vector<string>& result)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;

    do {
        index = str.find_first_of(separator,start);
        if (index != string::npos) {
            if ((index - start ) > 0)
                result.push_back(str.substr(start,index - start));
            start = str.find_first_not_of(separator,index);
            if (start == string::npos) return;
        }
    } while(index != string::npos);

    //the last token
    result.push_back(str.substr(start));
}

/*
* Split the given @src by @separator, the @separator will not appear in result.
* @param src - the string to be split.
* @param separator - the separator.
* @param result - the split pieces of string.
*/
void split(const wstring& src, const wstring& separator, vector<wstring>& result)
{
    if(src.empty()) return;

    wstring str = src;
    wstring substring;
    wstring::size_type start = 0, index;

    do {
        index = str.find_first_of(separator,start);
        if (index != wstring::npos) {
            substring = str.substr(start,index-start);
            result.push_back(substring);
            start = str.find_first_not_of(separator,index);
            if (start == wstring::npos) return;
        }
    } while(index != wstring::npos);

    //the last token
    substring = str.substr(start);
    result.push_back(substring);
}

/*
* Trim the given string, it removes all white space at the left and right side
* of the given string until meets a non-white space character.
*
* @param str - the string to be trim
* @return - the trimmed str.
*/
const string trim(const string& str)
{
    std::string::size_type first = str.find_first_not_of(" \n\t\r\0xb");
    if (first == std::string::npos) {
        return std::string();
    } else {
        std::string::size_type last = str.find_last_not_of(" \n\t\r\0xb");
        return str.substr( first, last - first + 1);
    }
}

/*
* Trim the given string, it removes all white space at the left and right side
* of the given string until meets a non-white space character.
*
* @param str - the string to be trim
* @return - the trimmed str.
*/
const wstring trim(const wstring& str)
{
    std::wstring::size_type first = str.find_first_not_of(L" \n\t\r\0xb");
    if (first == std::wstring::npos) {
        return std::wstring();
    } else {
        std::wstring::size_type last = str.find_last_not_of(L" \n\t\r\0xb");
        return str.substr( first, last - first + 1);
    }
}

inline bool startswith(const string & str, const string & sub) {
    return str.compare(0,sub.length(),sub) == 0;
}
inline bool startswith(const string & str, int start, const string & sub) {
    return str.compare(start,sub.length(),sub) == 0;
}

inline bool endswith(const string & str, const string & sub) {
    return str.compare(str.length() - sub.length(),sub.length(),sub) == 0;
}
inline bool endswith(const string & str, int end, const string & sub) {
    return str.compare(end - sub.length(),sub.length(),sub) == 0;
}

inline bool equal(const string & pattern, const string & str2, int start,int len)
{
    int i = 0;
    const int pLen = pattern.length();
    for (; i < pLen && i < len && pattern[i] == str2[i+start]; i++);
    return i == len;
}

inline bool  isPunc(const string &str)
{
    static map<string,bool> puncs;
    static bool inited = false;
    if (!inited) {
        //'(',')','（','）','《','》','【','】','，','、','？','!','。','；','：'
        puncs["("] = true;
        puncs[")"] = true;
        puncs["（"] = true;
        puncs["）"] = true;
        puncs["《"] = true;
        puncs["》"] = true;
        puncs["【"] = true;
        puncs["】"] = true;
        puncs["，"] = true;
        puncs["、"] = true;
        puncs["！"] = true;
        puncs[","] = true;
        puncs["。"] = true;
        puncs["!"] = true;
        puncs["；"] = true;
        puncs["："] = true;
        inited = true;
    }
    return puncs.find(str) != puncs.end();
}

class LineFileReader
{
private:
    ifstream inf;
    string lastLine;
public:
    LineFileReader(const string & file)
    {
        if(!fileExist(file.c_str())){
            cerr<<"error: "<<__FILE__<<":"<<__LINE__<<" "<<__func__<< "| "
                <<file.c_str()<<"not exist!"<<endl;
        }
        inf.open(file.c_str());
    }
    ~LineFileReader()
    {
        inf.close();
    }

    string * getLine()
    {
        if(inf.good()) {
            string line;
            do {
                if(getline(inf,line)) {
                    line = trim(line);
                    if(!line.empty()) {
                        lastLine = line;
                        return &lastLine;
                    }
                }
            } while(!inf.eof());
        }

        return NULL;
    }

private:
    LineFileReader();
    LineFileReader(const LineFileReader &);
    LineFileReader & operator = (const LineFileReader &);
};

class Timer
{
private:
    clock_t start_time;
    clock_t end_time;
public:
    Timer()
    {
        start_time = clock();
    }

    void restart()
    {
        start_time = clock();
    }

    double elapsed()
    {
        end_time = clock();
        return ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    }

    friend ostream & operator<< (ostream & out, Timer & timer)
    {
        out<<" elapsed: "<<timer.elapsed()<<"s";
        return out;
    }

};

// POS_tag_id中的词性定义
const int  POS_UDF = 0;  // 未定义
const int  POS_MA = 30;  // 阿拉伯数字
const int  POS_MC = 31;  // 中文数字
const int  POS_MP = 32;  // 电话
const int  POS_URL = 40; // url
const int  POS_T = 101;  // 时间
const int  POS_NX = 112; // 外文
const int  POS_M = 102; // 外文
const int  POS_W = 25; // 外文

namespace mingspy
{
struct lookup_tables {
    static const unsigned char whitespace[ 256 ]; // Whitespace table
    static const unsigned char upcase[ 256 ]; // To uppercase conversion table for ASCII characters
    static const unsigned char estr[ 256 ];   // english character 
    static const unsigned char utf8len[ 256 ]; // english and some connect chars
    static const unsigned char num[ 256 ]; // arbic numbers:1,2,3,4,5,6,7,8,9,0
    static const unsigned char alnum[ 256 ]; // arbic numbers:1,2,3,4,5,6,7,8,9,0
};
const unsigned char lookup_tables::whitespace[ 256 ] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,     // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 1
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      // F
};

const unsigned char lookup_tables::upcase[ 256 ] = {
    // 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A   B   C   D   E   F
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,     // 0
    16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,  // 1
    32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,  // 2
    48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,  // 3
    64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,  // 4
    80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,  // 5
    96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,  // 6
    80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,  // 7
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143, // 8
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159, // 9
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175, // A
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191, // B
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207, // C
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223, // D
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239, // E
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255  // F
};

const unsigned char lookup_tables::estr[ 256 ] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 1
    0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1,     // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,     // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1,     // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,     // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      // F
};
const unsigned char lookup_tables::num[ 256 ] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0,     // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,     // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      // F
};
const unsigned char lookup_tables::alnum[ 256 ] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 1
    0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1,     // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1,     // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1,     // 5
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,     // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      // F
};
const unsigned char lookup_tables::utf8len[ 256 ] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 1
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 3
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 4
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 5
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 6
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 7
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 8
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 9
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // A
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // B
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,     // C
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,     // D
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,     // E
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1      // F
};


/*
* split out English string, http, email,numbers.
* @return then end of English string.
*/
inline int utf8_next_estr(const string & str, int start) {
    while(lookup_tables::estr[(unsigned char)(str[start])]) { start++; }
    return start;
}

inline int utf8_next_num(const string & str, int start) {
    while(lookup_tables::num[(unsigned char)(str[start])]) { start++; }
    return start;
}

inline int utf8_next_alnum(const string & str, int start) {
    while(lookup_tables::alnum[(unsigned char)(str[start])]) { start++; }
    return start;
}
/*
* split out English string, http, email,numbers.
* @return then end of English string.
*/
inline int utf8_char_len(unsigned char start) {
    return lookup_tables::utf8len[start];
}

inline int utf8_to_unicode_len(const char * str,int start, int end){
    int len = 0;
    for(int i = start; i < end;){
        int chlen = utf8_char_len(str[i]);
        len++;
        i+=chlen;
        //assert(len < 100000);
    }
    return len;
}


string reverse_utf8(const string & src, int start, int endoff){
    const int size =  endoff - start +1;
    char dest[size];
    int idx = size - 1;
    dest[idx] = 0;
    for(int i = start; i < endoff; ){
        int chlen = utf8_char_len(src[i]);
        idx -= chlen;
        for(int j = 0; j < chlen; j++){
            dest[idx + j] = src[j+i];
        }
        i += chlen;
    }
    return string(dest);
}

string reverse_utf8(const string & src){
    return reverse_utf8(src,0,src.length());
}

};

