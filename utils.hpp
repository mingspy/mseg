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

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#if _MSC_VER > 1000
#include <io.h>
#else
#include<sys/types.h>
#include <dirent.h>
#endif


using namespace std;


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

void getFiles( const string & path, vector<string>& files )
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
                if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
                    getFiles( p.assign(pathInner).append("/").append(fileinfo.name), files );
            } else {
                files.push_back(p.assign(pathInner).append("/").append(fileinfo.name) );
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
            if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
                continue;
            getFiles(pathInner+"/"+ent->d_name, files);
        } else {
            files.push_back(pathInner+"/"+ent->d_name);
        }
    }

#endif
}

size_t fileSize(const string& filename)
{
    ifstream file (filename.c_str(), ios::in|ios::binary|ios::ate);
    if(!file.good()) {
        return 0;
    }

    file.seekg(0, ios::end);
    long length = file.tellg();
    file.close();
    return length;
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
            substring = str.substr(start,index-start);
            result.push_back(substring);
            start = str.find_first_not_of(separator,index);
            if (start == string::npos) return;
        }
    } while(index != string::npos);

    //the last token
    substring = str.substr(start);
    result.push_back(substring);
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

inline bool startswith(const string & str, const string & sub){
    return str.find(sub) == 0;
}

inline bool endswith(const string & str, const string & sub){
    return str.rfind(sub) == (str.length() - sub.length());
}

class LineFileReader
{
private:
    ifstream inf;
    string lastLine;
public:
    LineFileReader(const string & file)
    {
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
        return (double)(end_time - start_time) / CLOCKS_PER_SEC;
    }

    friend ostream & operator<< (ostream & out, Timer & timer)
    {
        out<<" elapsed: "<<timer.elapsed()<<"s";
        return out;
    }
};
