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
#include <climit>
#include "datrie.hpp"
#include "sparse.hpp"

using namespace std;

namespace mingspy
{

const int MAX_WORD_LEN = 200;
const string UDF = "UDF";
const string INNER_CODING = "utf-8"; // inner coding using utf-8.
class Dictionary
{
public:
	typedef SparseVector<int> WordInfo;
	typedef map<string, int>::iterator NIT;
	typedef map<int, SparseVector<int>>::iterator WIT;	
protected:
    CharTrie datrie;
//    vector<string> natures;
//    map<string, int> nature_index_table;
	map<int, WordInfo> words_info_table;
	double total_freq;
	string name;
	string filepath;
    bool readonly;
	int max_word_id;

public:
    explicit Dictionary():readonly(false),max_word_id(0),total_freq(0.0)
    {
    }
	
	inline bool addWord(const string & word)
    {
		int id = 0;
		if( datrie.find(word.c_str(),&id){
			return false;
		}
        return datrie.add(word, ++max_word_id);
    }
	
	inline int getWordId(const string & word)
    {
		int id = 0;
		if( datrie.find(word.c_str(),&id){
			return id;
		}
        return -1;
    }
	
	bool addWordInfo(const string & word, const WordInfo & info)
    {
		int id = getWordId(word);
		if (id <= 0){
			if(!addWord(word)){
				return false;
			}
			id = getWordId(word);
		}
        WIT it = words_info_table.find(id);
		if(it != words_info_table.end()){
			it->second.merge(info);
			return true;
		}
		words_info_table[id] = info;
		total_freq += info.sum();
		return true;
    }

    WordInfo * getWordInfo(const string & word)
    {
        int id = 0;
		if( datrie.find(word.c_str(),&id){
			WIT it = words_info_table.find(id);
			if(it != words_info_table.end()){
				return &(it->second);
			}
		}
		
		return NULL;
    }
	
	int addAttrFreq(const string & word, int attrId, int freq) {
        int id = getWordId(word);
		if (id <= 0){
			if(!addWord(word)){
				return INT_MIN;
			}
			id = getWordId(word);
		}
		total_freq += freq;
        return words_info_table[id].addAttrFreq(attrId,freq);
    }
	
	int getAttrFreq(const string & word, int attrId) const {
        const WordInfo * pInfo = getWordInfo(word);
        if(pInfo != NULL) {
            return pInfo->getAttrValue(attrId);
        }

        return 0;
    }

    int getWordTotalFreq(const string & word) const
    {
        const WordInfo * pInfo = getWordInfo(word);
        if(pInfo != NULL) {
            return pInfo->sum();
        }

        return 0;
    }

    double getWordProb(const string & word) const
    {
        return (getTotalFreq(word) + 1.0) / (total_freq + 10000);
    }
	
    bool hasPrefix(const string & prefix) const
    {
        return datrie.hasPrefix(prefix);
    }
	
/*
    int addNature(const string& nature)
    {
		NIT it = nature_index_table.find(nature);
        if( it != nature_index_table.end()) {
            return it->second;
        }
        natures.push_back(nature);
        nature_index_table[nature] = natures.size() - 1;
        return natures.size() - 1;
    }

    int getNatureIndex(const string &nature) const
    {
		NIT it = nature_index_table.find(nature);
        if(it != nature_index_table.end()) {
            return it->second;
        }
        return -1;
    }

    wstring getNature(int index) const
    {
        if(index < natures.size()) {
            return natures[index];
        }
        return UDF;
    }

    bool writeToFile(const string & file)
    {
    }
*/
private:
    Dictionary(const Dictionary &);
};

#if 0
class NatureProbTable:public Dictionary
{
public:
    NatureProbTable():Dictionary()
    {
        genUnknownNauture();
    }

    NatureProbTable(const string & file):Dictionary(file)
    {
        genUnknownNauture();
    }

    ~NatureProbTable()
    {
        if(_unknownNature) {
            delete _unknownNature;
        }
    }

    int getNatureTotal(int natureIndex) const
    {
        if(natureIndex < natures.size()) {
            return getTotalFreq(natures[natureIndex]);
        }
        return 0;
    }

    double getCoProb(int from, int to) const
    {
        if(from < natures.size() && to < natures.size()) {
            const WordNature * fromInfo = getWordInfo(natures[from]);
            if(fromInfo) {
                double toFreq = fromInfo->getAttrValue(to) + 1.0;
                double FromTotal = fromInfo->sumOfValues() + 44.0;
                return toFreq / FromTotal;
            }
        }

        return 3.0/TOTAL_FREQ;
    }

    const WordNature * getUnknownNature() const
    {
        return _unknownNature;
    }

private:
    NatureProbTable(const NatureProbTable &);
    void genUnknownNauture()
    {
        _unknownNature = new WordNature();
        for(int i = 0; i < natures.size(); i++ ) {
            _unknownNature->setAttrValue(i, 1);
        }
    }

private:
    WordNature * _unknownNature;
};

class UserDict:public Dictionary
{
public:
    UserDict(const string & file):Dictionary(file)
    {
    }

    virtual const WordNature * getWordInfo(const string & word) const
    {
        const WordNature * pinfo =  (const WordNature *)datrie.retrieve(word.c_str());
        if(pinfo == NULL) {
            pinfo = (const WordNature *)user_datrie.retrieve(word.c_str());
        }

        return pinfo;
    }

    virtual bool existPrefix(const string & prefix) const
    {
        if(datrie.containsPrefix(prefix)) {
            return true;
        }

        return user_datrie.containsPrefix(prefix);
    }

    void loadUserDict(const vector<string> & files)
    {
        int udf_idx = getNatureIndex(NATURE_UNDEF);
        if(udf_idx < 0) {
            addNature(NATURE_UNDEF);
            udf_idx = getNatureIndex(NATURE_UNDEF);
        }

        int count = 0;
        for(int i = 0; i < files.size(); i++) {

            cout<<"\rloading user dictionary:"<<files[i].c_str();
            UTF8FileReader reader(files[i]);
            string * line;
            while((line = reader.getLine())) {
                string::size_type wordIndex = line->find_first_of(wordSeperator);
                string word;
                string wordinfo;
                if(wordIndex == string::npos) {
                    word = *line;
                } else {
                    word = line->substr(0, wordIndex);
                    if(wordIndex < line->length()) {
                        wordinfo = line->substr(wordIndex + 1);
                    }
                }
                if(wordinfo.empty()) {
                    if(!getWordInfo(word)) {
                        WordNature *nature = new WordNature();
                        nature->setAttrValue(udf_idx, 1);
                        user_datrie.add(word, nature);
                        count ++;
                    }
                } else {
                    WordNature * info = new WordNature();
                    vector<string> infos;
                    split(wordinfo, natureSeperator, infos);
                    for(int i = 0; i < infos.size(); i++) {
                        string::size_type freqIndex = infos[i].find_first_of(freqSeperator);
                        string nature;
                        int d_freq = 1;
                        if(freqIndex == string::npos) {
                            nature = infos[i];
                        } else {
                            nature = infos[i].substr(0,freqIndex);
                            string freq = infos[i].substr(freqIndex + 1);
                            d_freq = wcstol(freq.c_str(), NULL, 10);
                        }

                        int index = getNatureIndex(nature);
                        if(index == -1) {
                            wcerr<<L"The nature not exist in nature list of the file header:"
                                 <<nature<<" line:"<<*line<<endl;
                            addNature(nature);
                            index = getNatureIndex(nature);
                        }
                        info->setAttrValue(index, d_freq);
                    }

                    WordNature * natures = (WordNature *)getWordInfo(word);
                    if(natures == NULL) {
                        user_datrie.add(word, info);
                    } else {
                        for(int i = 0; i< info->numValues(); i++) {
                            int freq = info->valueAt(i) + natures->getAttrValue(info->attrAt(i));
                            natures->setAttrValue(info->attrAt(i), freq);
                        }
                        delete info;
                    }
                    count++;
                }

                if(count % 1000 == 0) {
                    cout<<"\r added -> "<<count;
                }
            }

        }
        cout<<endl;
    }
private:
    UserDict(const UserDict &);
protected:
    DATrie user_datrie;
};
#endif 
}
