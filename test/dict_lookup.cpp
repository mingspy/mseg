#include <iostream>
#include "dict.hpp"
#include <string>
#include <string.h>

using namespace std;
using namespace mingspy;

int main(int argc, char * argv[]){
    if (argc < 2){
        cout<<"usage:\t"<<argv[0]<<" dict_file_path"<<endl;
        return -1;
    }
   
    Dictionary dict;
    dict.open(argv[1]);
    string input;
    cout<<"Enter the word to query, enter \'exit\' to exit"<<endl;
    while(true){
        cout<<"> ";
        getline(cin,input);
        if (input == "exit"){
            break;
        }
        const Dictionary::WordInfo * info = dict.getWordInfo(input);
        if(info){
            cout<<info->word<<"(id="<<info->id<<", total="<<dict.getWordFreq(input)<<") ";
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
