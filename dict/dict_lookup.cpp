#include <iostream>
#include "dict.hpp"
#include <string>
#include <string.h>
#include "utils.hpp"

using namespace std;
using namespace mingspy;

int main(int argc, char * argv[]){
    if (argc < 2){
        cout<<"usage:\t"<<argv[0]<<" dict_file_path"<<endl;
        return -1;
    }

    bool inversed = false;
    if(argc > 2&& strcmp(argv[2],"inverse")== 0){
        inversed = true;
    }
   
    Timer t;
    Dictionary dict;
    dict.open(argv[1]);
    cout<<t<<endl;
    string word;
    cout<<"Enter the word to query, enter \'exit\' to exit"<<endl;
    while(true){
        cout<<"> ";
        getline(cin,word);
        if (word == "exit"){
            break;
        }
        if(inversed){
            word = reverse_utf8(word);
        }
        const Dictionary::WordInfo * info = dict.getWordInfo(word);
        if(info){
            cout<<info->word<<"(id="<<info->id<<", total="<<dict.getWordFreq(word)<<") ";
            if (info->info){
                for( int i = 0; i < info->info->size(); i++){
                    const char * word = dict.getWord(info->info->getId(i));
                    if (word) cout<<word;
                    else cout<<info->info->getId(i);
                    cout<< ":"<<info->info->getVal(i)<<" ";
                }
            }else{
                cout<<"null";
            }
            cout<<endl;
        }
        else{
            cout<<"not found in dict"<<endl;
        }
    }
    return 0;
}
