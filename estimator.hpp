#include <iostream>
#include <string>
#include <cstdlib>
#include "knife.hpp"
#include "dict.hpp"
#include <vector>
#include "sparse.hpp"
#include "utils.hpp"
#include "builder.hpp"

using namespace std;
namespace mingspy{

void prepair_estimate_data(const char * path = "../data/people/199806.txt"){
    LineFileReader reader(path);
    string * line;
    ofstream outf("./est.txt");
    while(line = reader.getLine()){
        string data = trim(*line);
        if(data.empty()) continue;
        vector<string> tokens;
        split(data," ",tokens);
        vector<PeopleEntity> vec;
        string testdata;
        string result;
        try {
            parsePeopleEntities(tokens,vec);
            for(int i = 0; i < vec.size(); i++) {
                PeopleEntity & ent = vec[i];
                if (ent.isCompose) {
                    for(int j = 0; j < ent.sub.size(); j++) {
                        testdata+=ent.sub[j].word;
                        if(!result.empty()) result += " ";
                        result+=ent.sub[j].word;
                    }
                } else {
                    testdata+=ent.word;
                    if(!result.empty()) result += " ";
                    result+=ent.word;
                }
            }
            outf<<"@testdata"<<endl<<testdata<<endl<<"@testresult"<<endl<<result<<endl;
        }catch(parse_error e) {
            cerr<<"parse error"<<endl;
        }
    }
}

class Estimator{
private:
    vector<string> testdata;
    vector<vector<string> > testresult;
public:
    Estimator(const string & testdatafile){
        LineFileReader reader(testdatafile);
        string *line;
        bool is_test_data = false;
        bool is_test_result = false;
        while(line = reader.getLine()){
            string data = trim(*line);
            if (data.empty()){ continue; }
            if (startswith(data,"@testdata")){
                is_test_data = true;
                is_test_result = false;
                continue;
            }else if(startswith(data,"@testresult")){
                is_test_data = false;
                is_test_result = true;
                continue;
            }
            if (is_test_data){
                testdata.push_back(data);
            }
            else if(is_test_result){
                testresult.push_back(vector<string>());
                split(data, " ",testresult[testresult.size()-1]);
            }
        }
        assert(testdata.size() == testresult.size());
    }

    void estimate(const IKnife & knife){
        cout<<"-------------------------------------------"<<endl;
        cout<<"estimating"<<knife.getName()<<endl;
        assert(testdata.size() == testresult.size());
        vector<vector<string> > splited;
        vector<Chip> chips;
        double tsize = 0;
        int times = 1024;
        if (testdata.size() > 1000){
            times = 1;
        }
        Timer timer;
        for(int j = 0; j< times; j++){
            splited.clear();
            for(int i = 0; i < testdata.size(); i++){
                chips.clear();
                splited.push_back(vector<string>());
                tsize += testdata[i].length();
                knife.split(testdata[i],chips);
                substrs(testdata[i],chips,splited[i]);
            }
        }
        double elapsed = timer.elapsed();
        int total = 0;
        int ttotal = 0;
        int correct = 0;
        for(int i = 0; i < testresult.size(); i++){
            bool * visited = new bool [testresult[i].size()];
            memset(visited, 0, sizeof(bool) * testresult[i].size());
            for(int k = 0; k < testresult[i].size(); k ++){
                if (isPunc(testresult[i][k])){
                    visited[k] = true;
                }else{
                    ttotal ++;
                }
            }

            for(int j = 0; j < splited[i].size(); j++){
                if (isPunc(splited[i][j])) continue;
                total ++;
                bool found = false;
                for(int k = 0; k < testresult[i].size(); k ++){
                    if(!visited[k] && splited[i][j] == testresult[i][k]){
                        found = true;
                        visited[k] = true;
                        break;
                    }
                } 
                if (found){
                    correct ++;
                }
            }
            delete [] visited;
        }

        double precision = (correct/(total + 0.0000001));
        double recall = (correct/(ttotal + 0.0000001));
        double f1 = 2 * precision * recall / (precision + recall);
        cout<<"test data "<< (tsize/1024.0)<<"kb, used"
            << elapsed << "s, speed(m/s) "<<(tsize/1024.0/1024.0/elapsed)<<endl 
            //<< "test words = "<< ttotal<<" splited words = " << total << " correct words = " << correct<<endl 
            << "precision " << precision << " recall " << recall<<" f1 " <<f1<<endl;
        cout<<"-------------------------------------------"<<endl;
    }
};

class TaggerEstimator{
private:
    vector<vector<string> >  _data;
    vector<vector<string> >  _result;
public:
    TaggerEstimator(const string & path = "../data/people/199801.txt"){
        LineFileReader reader(path);
        string * line;
        while(line = reader.getLine()){
            string data = trim(*line);
            if(data.empty()) continue;
            vector<string> tokens;
            split(data," ",tokens);
            vector<PeopleEntity> vec;
            try {
                parsePeopleEntities(tokens,vec);
                _data.push_back(vector<string>());
                _result.push_back(vector<string>());
                vector<string> &d = _data[_data.size() - 1];
                vector<string> &r = _result[_result.size() - 1];
                for(int i = 0; i < vec.size(); i++) {
                    PeopleEntity & ent = vec[i];
                    if (ent.isCompose) {
                        for(int j = 0; j < ent.sub.size(); j++) {
                            d.push_back(ent.sub[j].word);
                            r.push_back(ent.sub[j].pos);
                        }
                    } else {
                        d.push_back(ent.word);
                        r.push_back(ent.pos);
                    }
                }
            }catch(parse_error e) {
                cerr<<"parse error"<<endl;
            }
        }
    }

    void estimate(const Tagger & tagger){
        cout<<"------------estimating tagger------------------"<<endl;
        assert(_data.size() == _result.size());
        vector<vector<string> > tagged;
        Timer timer;
        for(int i = 0; i < _data.size(); i++){
            tagged.push_back(vector<string>());
            tagger.tagging(_data[i],tagged[i]);
        }
        double elapsed = timer.elapsed();
        int tsize = 0;
        int total = 0;
        int correct = 0;
        for(int i = 0; i < _result.size(); i++){
            for (int j = 0; j < _result[i].size();j++){
                total += 1;
                tsize += _data[i][j].length();
                if(_result[i][j] == tagged[i][j]){
                    correct ++;
                }
            }
        }

        double precision = (correct/(total + 0.0000001));
        double recall = (correct/(total + 0.0000001));
        double f1 = 2 * precision * recall / (precision + recall);
        cout<<"test data "<< tsize<<", used"
            << elapsed << "s, speed(m/s) "<<(tsize/1024.0/1024.0/elapsed)<<endl 
            << "precision " << precision << " recall " << recall<<" f1 " <<f1<<endl;
        cout<<"-------------------------------------------"<<endl;
    }

};

}
