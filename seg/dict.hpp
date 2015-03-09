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
#include <map>
#include <cstdlib>
#include <limits.h>
#include "datrie.hpp"
#include "sparse.hpp"
#include "mempool.hpp"

using namespace std;

namespace mingspy
{
const int MAX_WORD_LEN = 200;
const string UDF = "UDF";
const string INNER_CODING = "utf-8"; // inner coding using utf-8.
const string POS_FREQ_TOTAL = "@POS^TOT@";
const string WORDS_FREQ_TOTAL = "@WD^TOT@";

class Dictionary
{
public:
    typedef SparseVector<int> FreqInfo;
    typedef map<string, int>::iterator NIT;
    typedef struct WordInfo {
        char * word;
        int id;
        FreqInfo * info;
        WordInfo():word(NULL),id(-1),info(NULL){}
    } WordInfo;
    typedef map<int, WordInfo*>::iterator WIT;
    typedef map<int, WordInfo*>::const_iterator CWIT;
protected:
    CharTrie datrie;
    // TODO change to self manganger type.
    mutable map<int, WordInfo *> words_info_table;
    string name;
    string filepath;
    //bool readonly;
    int max_word_id;
    MemoryPool<char> mempool;

public:
    explicit Dictionary():max_word_id(0)
    {
        mempool.setDynamicIncreament(1024*1024);
    }

    ~Dictionary()
    {
        clear();
    }
    inline void setMaxWordId(int id)
    {
        max_word_id = id;
    }
    inline int getMaxWordId() const
    {
        return max_word_id;
    }
    inline int addWord(const string & word)
    {
        int wordid = addWord(word, max_word_id + 1);
        return wordid;
    }

    inline int addWord(const string & word, int wordid)
    {
        int wid = 0;
        if(datrie.find(word.c_str(),&wid)) {
            return wid;
        }
        if (!datrie.add(word.c_str(), wordid)) {
            return -1;
        }

        if (wordid >= max_word_id){
            max_word_id = wordid;
        }

        WordInfo *info = reinterpret_cast<WordInfo *>(mempool.allocAligned(sizeof(WordInfo)));
        int sz = word.length()+1;
        info->word = mempool.allocStr(word.c_str(),sz);
        info->id = wordid;
        info->info = NULL;
        words_info_table[wordid] = info;
        return wordid;
    }

    int getWordId(const string & word) const
    {
        int id = 0;
        if(datrie.find(word.c_str(),&id)) {
            return id;
        }
        return -1;
    }

    int getWordId(const string & word,int start, int end) const
    {
        int id = 0;
        if(datrie.find(word.c_str(),start,end,&id)) {
            return id;
        }
        return -1;
    }

    bool addFreqInfo(const string & word, const FreqInfo & info)
    {
        int id = addWord(word);
        if (id <= 0) {
            return false;
        }

        if(words_info_table[id]->info == NULL) {
            words_info_table[id]->info = new FreqInfo(info);
        } else {
            words_info_table[id]->info->merge(info);
        }
        
        return true;
    }

    inline const WordInfo * getWordInfo(const string & word) const
    {
        int wordId = getWordId(word);
        return getWordInfo(wordId);
    }
    inline const WordInfo * getWordInfo(const string & word,int start,int end) const
    {
        int wordId = getWordId(word,start, end);
        return getWordInfo(wordId);
    }

    inline const WordInfo * getWordInfo(int wordId) const
    {
        /*
        CWIT it = words_info_table.find(wordId);
        if (it != words_info_table.end()) {
            return & (it->second);
        }
        return NULL;
        */
        if (wordId >= 0){
            return words_info_table[wordId];
        }
        return NULL;
    }

    const char * getWord(int wordId) const
    {
        const WordInfo * info = getWordInfo(wordId);
        if (info) {
            return info->word;
        }
        return NULL;
    }

    inline FreqInfo * getFreqInfo(const string & word,int start,int end) const
    {
        int id = getWordId(word,start,end);
        return getFreqInfo(id);
    }

    inline FreqInfo * getFreqInfo(const string & word) const
    {
        int id = getWordId(word);
        return getFreqInfo(id);
    }

    inline FreqInfo * getFreqInfo(int wordid) const
    {
        /*
        CWIT it = words_info_table.find(wordid);
        if(it == words_info_table.end()) {
            return NULL;
        }
        return it->second.info;
        */
        if (wordid > 0)
            return words_info_table[wordid]->info;

        return NULL;
    }

    inline FreqInfo * operator[](const string & word) const
    {
        return getFreqInfo(word);
    }

    int addAttrFreq(const string & word, const string & attr, int freq)
    {
        int id = addWord(word);
        if (id <= 0) {
            return INT_MIN;
        }
        int attrId = addWord(attr);
        if (attrId <= 0) {
            return INT_MIN;
        }
        return addAttrFreq(id, attrId, freq);
    }
    int addAttrFreq(const string & word, int attrId, int freq)
    {
        int id = addWord(word);
        return addAttrFreq(id, attrId, freq);
    }

    int addAttrFreq(int id, int attrId, int freq)
    {
        if (id <= 0) {
            return INT_MIN;
        }
        if (!words_info_table[id]->info) {
            words_info_table[id]->info = new FreqInfo();
        }
        return words_info_table[id]->info->addAttrFreq(attrId,freq);
    }

    int getAttrFreq(const string & word, int attrId) const
    {
        int wordId = getWordId(word);
        return getAttrFreq(wordId,attrId);
    }

    int getAttrFreq(const string & word, const string & attr) const
    {
        int wordId = getWordId(word);
        int attrId = getWordId(attr);
        return getAttrFreq(wordId,attrId);
    }

    int getAttrFreq(int wordId, int attrId) const
    {
        const FreqInfo * info = getFreqInfo(wordId);
        if(info){
            return info->getAttrValue(attrId);
        }
        return 0;
    }

    int getWordFreq(const string & word) const
    {
        const FreqInfo * pInfo = getFreqInfo(word);
        if(pInfo != NULL) {
            return pInfo->sum();
        }

        return 0;
    }

    int getWordFreq(int wordid) const
    {
        const FreqInfo * pInfo = getFreqInfo(wordid);
        if(pInfo != NULL) {
            return pInfo->sum();
        }

        return 0;
    }

    bool hasPrefix(const string & prefix) const
    {
        return datrie.hasPrefix(prefix.c_str(),0, prefix.length());
    }

    bool hasPrefix(const string & prefix, int start, int end) const
    {
        return datrie.hasPrefix(prefix.c_str(),start, end);
    }

    bool exist(const string & word) const
    {
        return datrie.find(word.c_str(),0,word.length());
    }

    bool exist(const string & word, int start, int end) const
    {
        return datrie.find(word.c_str(),start,end);
    }

    inline string getName() const
    {
        return name;
    }
    inline void setName(const string & dictname)
    {
        this->name = dictname;
    }
    inline string getPath() const
    {
        return this->filepath;
    }
    bool open(const string & path)
    {
        ifstream inf(path.c_str(),ios::binary|ios::in);
        if (!inf.good()) {
            cerr<<"can't open dict:"<<path<<endl;
            return false;
        }
        filepath = path;
        
        if(!inf.read(reinterpret_cast<char *> (&max_word_id), sizeof(int))) {
            return false;
        }
        int sz;
        if(!inf.read(reinterpret_cast<char *> (&sz), sizeof(int))) {
            return false;
        }
        if (sz) {
            char buf[MAX_WORD_LEN];
            if(!inf.read(buf, sz)) {
                return false;
            }
            name = buf;
        }
        if(!inf.read(reinterpret_cast<char *> (&sz), sizeof(int))) {
            return false;
        }
        WordInfo * p = reinterpret_cast<WordInfo*>(mempool.allocAligned(sz*sizeof(WordInfo)));
        if(!inf.read(reinterpret_cast<char *> (p), sz*sizeof(WordInfo))) {
            return false;
        }
        
        for (int i = 0; i < sz; i++) {
            words_info_table[p[i].id] = p+i;
        }
        int bytes;
        if(!inf.read(reinterpret_cast<char *> (&bytes), sizeof(int))) {
            return false;
        }
        char * str = mempool.allocStr(NULL,bytes);
        if(!inf.read(reinterpret_cast<char *> (str), bytes)) {
            return false;
        }
        for (int i = 0; i < sz; i++) {
            if(p[i].word){
                p[i].word = str+sizeof(int);
                str += (*((int*)(str)))+sizeof(int);
            }
            if(p[i].info){
                p[i].info = FreqInfo::read(str);
                str += p[i].info->bytes();
            }
        }
        return datrie.read(inf);

    }
    bool save(const string & path)
    {
        ofstream outf(path.c_str(),ios::binary|ios::out);
        if (!outf.good()) {
            cerr<<"can't open file:"<<path<<endl;
            return false;
        }
        filepath = path;
        
        if(!outf.write(reinterpret_cast<char *>(&max_word_id), sizeof(int))) {
            return false;
        }
        int sz = name.length();
        if (sz != 0) {
            sz ++;
        }
        if(!outf.write(reinterpret_cast<char *>(&sz), sizeof(int))) {
            return false;
        }
        if (sz) {
            if(!outf.write(name.c_str(), sz)) {
                return false;
            }
        }
        sz = words_info_table.size();
        if(!outf.write(reinterpret_cast<char *>(&sz), sizeof(int))) {
            return false;
        }
        // write all WordInfo together, then loading can be more faster.
        for (CWIT it = words_info_table.begin(); it != words_info_table.end(); it++) {
            if(!outf.write(reinterpret_cast<char *>(const_cast<WordInfo *>(it->second)), sizeof(WordInfo))) {
                return false;
            }
        }
        
        // write word
        int total_bytes = 0;
        for (CWIT it = words_info_table.begin(); it != words_info_table.end(); it++) {
            if (it->second->word) {
                int sz = Length<char>()(it->second->word) + 1;
                total_bytes += sz + sizeof(int);
            }
            if (it->second->info) {
                total_bytes+=it->second->info->bytes();
            }
        }
        if(!outf.write(reinterpret_cast<char *>(&total_bytes), sizeof(int))) {
            return false;
        }
        for (CWIT it = words_info_table.begin(); it != words_info_table.end(); it++) {
            if (it->second->word) {
                int sz = Length<char>()(it->second->word) + 1;
                if(!outf.write(reinterpret_cast<char *>(&sz), sizeof(int))) {
                    return false;
                }
                if(!outf.write(it->second->word, sz)) {
                    return false;
                }
            }
            if (it->second->info) {
                FreqInfo::write(outf,it->second->info);
            }
        }
        // write info
        return datrie.write(outf);
    }
private:
    Dictionary(const Dictionary &);
    void clear()
    {
        for (WIT it = words_info_table.begin(); it != words_info_table.end(); it++) {
            WordInfo *info = it->second;
            if (info->info){
                FreqInfo::collect(info->info);
            }
        }
        words_info_table.clear();
        max_word_id = 0;
    }
};

}

