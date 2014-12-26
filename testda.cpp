#include "datrie.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <ctime>

using namespace mingspy;
using namespace std;

//注意：当字符串为空时，也会返回一个空字符串  
void split(const std::string& s, const std::string& delim,std::vector< std::string >& ret)  
{  
    size_t last = 0;  
    size_t index=s.find_first_of(delim,last);  
    while (index!=std::string::npos)  
    {  
        ret.push_back(s.substr(last,index-last));  
        last=index+1;  
        index=s.find_first_of(delim,last);  
    }  
    if (index-last>0)  
    {  
       ret.push_back(s.substr(last,index-last));  
    }  
}  
void test_da() {

    string str = "中国人";
    DoubleArray<char, unsigned char, int, unsigned int> da;

    int s = da.getRoot();
    const char * p = str.c_str();
    for(;*p;p++){
        if (da.walk(&s,*p)){
            cout<<"can walk"<<endl;
        }else{
            s = da.insertBranch(s, *p);
            cout<<"insert"<<endl;
        }
    }


    cout<<"checking"<<str<<endl;
    s = da.getRoot();
    p = str.c_str();
    for(;*p;p++){
        if (da.walk(&s,*p)){
            cout<<"can walk"<<endl;
        }else{
            s = da.insertBranch(s, *p);
            cout<<"insert"<<endl;
        }
    }
    if (!*p){
        cout<<"finished"<<endl;
    }

    cout<<"insert str2"<<endl;
    wstring str2 = L"中国的";
    s = da.getRoot();
    p = (const char *)str2.c_str();
    cout<<"sizeof wchar_t"<<sizeof(wchar_t)<<" "<<str2.length()<<endl;
    for(int i = 0; i < str2.length() * sizeof(wchar_t);i++){
        cout<<0+p[i]<<" ";
    }
    cout<<endl;

    for(;*p;p++){
        if (da.walk(&s,*p)){
            cout<<"can walk"<<endl;
        }else{
            s = da.insertBranch(s, *p);
            cout<<"insert"<<endl;
        }
    }

    cout<<"checking str2"<<endl;
    s = da.getRoot();
    p = (const char *)(str2.c_str());
    for(;*p;p++){
        if (da.walk(&s,*p)){
            cout<<"can walk"<<endl;
        }else{
            s = da.insertBranch(s, *p);
            cout<<"insert"<<endl;
        }
    }
    if (!*p){
        cout<<"finished"<<endl;
    }
}

void test_datrie(){
    CharTrie trie;
    cout<<"add 1"<<endl;
    trie.add("中国", 1);
    cout<<"add 1"<<endl;
    trie.add("中国", 1);
    cout<<"add 2"<<endl;
    trie.add("中国", 2);
    cout<<"add 3"<<endl;
    trie.add("中国的", 3);
    cout<<"add 4"<<endl;
    trie.add("中国的人", 4);
    cout<<"add 5"<<endl;
    trie.add("中华民族", 5);
    cout<<"add 6"<<endl;
    trie.add("民族", 6);
    int idx;
    cout<<"finding 中国"<<endl;
    if(!trie.find("中国",&idx)){
        cerr<<"can't find 中国"<<endl;
    }
    if (idx != 2){
        cerr<<"result != 2"<<endl;
    }

    cout<<"finding 中国的"<<endl;
    if(!trie.find("中国的",&idx)){
        cerr<<"can't find 中国的"<<endl;
    }
    if (idx != 3){
        cerr<<"result != 3"<<endl;
    }

    cout<<"finding 中国的人"<<endl;
    if(!trie.find("中国的人",&idx)){
        cerr<<"can't find 中国的人"<<endl;
    }
    if (idx != 4){
        cerr<<"result != 4"<<endl;
    }

    cout<<"finding 中华民族"<<endl;
    if(!trie.find("中华民族",&idx)){
        cerr<<"can't find 中华民族"<<endl;
    }
    if (idx != 5){
        cerr<<"result != 5"<<endl;
    }
    cout<<"finding 民族"<<endl;
    if(!trie.find("民族",&idx)){
        cerr<<"can't find 民族"<<endl;
    }
    if (idx != 6){
        cerr<<"result != 6"<<endl;
    }

    if (! trie.remove("中国")){
        cerr<<"del 中国 failed"<<endl;
    }
    if(!trie.find("中国的",&idx)){
        cerr<<"can't find 中国的"<<endl;
    }
    if (idx != 3){
        cerr<<"result != 3"<<endl;
    }
    if(!trie.find("中国的人",&idx)){
        cerr<<"can't find 中国的人"<<endl;
    }
    if (idx != 4){
        cerr<<"result != 4"<<endl;
    }
    if (! trie.remove("中国的人")){
        cerr<<"del 中国的人 failed"<<endl;
    }
    if(!trie.find("中国的",&idx)){
        cerr<<"can't find 中国的"<<endl;
    }
    if (idx != 3){
        cerr<<"result != 3"<<endl;
    }
    cout<<"test succeed"<<endl;
}

void press_test(const string &filepath){
    ifstream inf(filepath.c_str());
    string line;
    map<string, int> words;
    vector<string> ws;
    int id = 0;
    while(getline(inf,line)){
        ws.clear();
        split(line," ", ws);
        for(int i = 0; i < ws.size(); i++){
            if(!ws[i].empty() && ws[i]!=" "){
                if (words.find(ws[i]) == words.end()){
                    words[ws[i]] = id++;
                }
            }
            if (id % 1000 == 0){
                cout<<"\r loaded "<<id;
            }
        }
    }
    cout<<endl;

    int add_errors = 0;
    CharTrie trie;
    int cnt = 0;
    clock_t start = clock();
    for(map<string,int>::iterator it = words.begin(); it != words.end(); it++){
       if(! trie.add(it->first.c_str(), it->second)){
           add_errors ++;
           cerr<<"failded add "<<it->first<<endl;
       }
    }
    clock_t end = clock();
    int add_time = end - start;
    cout<<"adding errors = "<<add_errors<<endl;

    int find_errors = 0;
    start = clock();
    for(map<string,int>::iterator it = words.begin(); it != words.end(); it++){
       if(! trie.find(it->first.c_str(), &id) || id != it->second){
           find_errors ++;
           cerr<<"failed find "<<it->first<<endl;
       }
    }
    end = clock();
    int find_time = end -start;
    cout<<"finding errors = "<<find_errors<<endl;
    cout<<"add items :"<<words.size()
        <<" used:" <<(add_time/CLOCKS_PER_SEC)
        <<" find used:"<<(find_time/CLOCKS_PER_SEC)<<endl;
	ofstream outf("./tt.da", ios::binary|ios::out);
    trie.save(outf);
	outf.close();
    CharTrie trie2;
    cout<<"loading trie"<<endl;
	ifstream inf("./tt.da", ios::binary|ios::in);
    trie2.open(inf);
    find_errors = 0;
    start = clock();
    for(map<string,int>::iterator it = words.begin(); it != words.end(); it++){
       if(! trie2.find(it->first.c_str(), &id) || id != it->second){
           find_errors ++;
           cerr<<"failed find "<<it->first<<endl;
       }
    }
    end = clock();
    find_time = end -start;
    cout<<"finding errors = "<<find_errors<<endl;
}

int main(){
//    test_da();
//    test_datrie();
    press_test("./words.txt");
}
