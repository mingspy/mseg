#include <iostream>
#include "knife.hpp"
#include "dict.hpp"
#include <vector>
#include "sparse.hpp"
#include "estimator.hpp"
#include "utils.hpp"
#include "builder.hpp"
#include "mseg.cpp"

using namespace std;
using namespace mingspy;
const int DATA_SIZE = 15;
string testdata[] = {
    "软件和服务",
    "www.sina.com.cn是个好网站",
    "我们都会知道还是都不会知道这是一个长句子测试。在所有字符集中最知名的可能要数被称为ASCII的7位字符集了。它是美国标准信息交换代码的缩写",
    "中华人民共和国简称中国",
    "结婚的和尚未结婚的",
    "他说的确实在理",
    "把手抬起来",
    "邓颖超生前使用过的物品",
    "阿拉斯加遭强暴风雪袭击致xx人死亡",
    "今后三年中将翻两番",
    "乒乓球拍卖完了",
    "费孝通向人大常委会提交书面报告",
    "粮食不卖给八路军",
    "吴江西陵印刷厂",
    "叔叔亲了我妈妈也亲了我",//15
};

string speedstr =  "在所有字符集中，最知名的可能要数被称为ASCII的7位字符集了。它是美国标准信息交换代码（American Standard Code for Information Interchange）的缩写, 为美国英语通信所设计。它由128个字符组成，包括大小写字母、数字0-9、标点符号、非打印字符";

void testKnife(IKnife & seg)
{
    cout<<"testing "<<seg.getName()<<" start"<<endl;
    Token vec[10000];
    Timer t;
    for(int i = 0; i < DATA_SIZE; i++) {
        int len = seg.split(testdata[i],vec,10000);
        print(testdata[i],vec, len);
    }
    cout<<"used "<<t.elapsed()<<"s"<<endl;
    cout<<"---------------------------------"<<endl;
}

void testUtf8len()
{
    string ss = "abc中国人哈2";
    cout<<ss.length()<<endl;
    for(int i = 0; i < ss.length(); ) {
        cout<<i<<" ";
        i += utf8_char_len(ss[i]);
    }
    cout<<endl;
}

void printSegs(const Dictionary * core_dict, const Dictionary * inverse_dict)
{
    Flycutter fc(core_dict);
    Renda rd(inverse_dict);
    Paoding pao(core_dict);
    Unigram ug(core_dict);

    testKnife(fc);
    testKnife(rd);
    testKnife(ug);
    testKnife(pao);
}

void testSegs(const Dictionary * core_dict, const Dictionary * inverse_dict)
{
    Flycutter fc(core_dict);
    Renda rd(inverse_dict);
    Paoding pao(core_dict);
    Unigram ug(core_dict);

    Estimator est;
    est.estimate(fc);
    est.estimate(rd);
    est.estimate(ug);
    est.estimate(pao);
}

void testGenGraph(const Dictionary * core_dict)
{
    cout<<"testing GenGraph speed "<<endl;
    Timer t;
    double size = speedstr.length();
    Graph g;
    for(int i = 0; i < 1024; i ++) {
        g.gen(*core_dict, speedstr,0,size);
    }
    cout<<"speed is "<<size/t.elapsed()/1024<<endl;
}

void testNShortPath(const Dictionary * core_dict)
{
    cout<<"testNShortPath"<<endl;
    Timer t;
    double size = speedstr.length();
	Token result[10000];
    Graph g;
    for(int i = 0; i < 1024; i ++) {
        g.gen(*core_dict, speedstr,0,size);
        NShortPath npath(g,10);
        npath.calc();
        npath.getBestPath(0,result);
    }
    cout<<"speed is "<<size/t.elapsed()/1024<<endl;
}
void testShortPath(const Dictionary * core_dict)
{
    cout<<"testShortPath"<<endl;
    Timer t;
    double size = speedstr.length();
	Token result[10000];
    Graph g;
    for(int i = 0; i < 1024; i ++) {
        g.gen(*core_dict, speedstr,0,size);
        ShortPath path(g);
        path.getBestPath(result);
    }
    cout<<"speed is "<<size/t.elapsed()/1024<<endl;
}
void testTagger(const Dictionary * core_dict)
{
    TaggerEstimator est;
    Tagger tagger(core_dict);
    est.estimate(tagger);
}
void test_metrics(const Dictionary * core_dict, const Dictionary * inverse_dict)
{
    testGenGraph(core_dict);
    testNShortPath(core_dict);
    testShortPath(core_dict);
    testSegs(core_dict, inverse_dict);
    testTagger(core_dict);
}

void test_ner(const Dictionary * core_dict, const Dictionary * person_dict)
{
    string str = "李修磊、李修修、李磊磊、李修磊磊、王石在中国北京sina上班ing,想到天龙八部中没有杨过与小龙女，春光灿烂猪九妹与春光灿烂猪八戒";
    Flycutter fc(core_dict);
    Paoding pao(core_dict);
    Unigram ug(core_dict);
    RoleTagger tagger(person_dict);
    NamedEntityRecognizer ner(person_dict, 8080, 100);
    Token tks[10000];
    cout<<"Enter the sentence to test ner. Enter \'exit\' to exit"<<endl;
    while(true){
        cout<<"> ";
        getline(cin,str);
        if(str == "exit"){
            break;
        }
        cout<<"-------split---------"<<endl;
        int len = fc.split(str,tks,1000); 
        print(str,tks,len);
        cout<<"-------tagging---------"<<endl;
        tagger.tagging(str,tks,len);
        print(str,tks,len);
        cout<<"-------ner---------"<<endl;
        len = ner.recognize(str,tks,len);
        print(str,tks,len);
    }

}
int main(int argc, char ** argv)
{
    if(argc < 2){
        cout<<"Usage:"<<argv[0]<<" option"<<endl;
        cout<<"options:"<<endl;
        cout<<"knife\t test knives output"<<endl;
        cout<<"metrics\t test knives perforemance"<<endl;
        cout<<"ner\t test named entities recognizer"<<endl;
        return -1;
    }
    Timer timer;
    Dictionary core_dict;
    core_dict.open("../dict/core.dic");
    Dictionary inverse_dict;
    inverse_dict.open("../dict/inverse.dic");
    Dictionary person_dict;
    person_dict.open("../dict/person.dic");
    double load_used = timer.elapsed();
    cout<<"load dicts used" << load_used<<endl;
    string option = argv[1];
    if(option == "knife"){
        printSegs(&core_dict, & inverse_dict);
    }else if(option == "metrics"){
        test_metrics(&core_dict, &inverse_dict);
    }else if(option == "ner"){
        test_ner(&core_dict, &person_dict);
    }
    else{
        string s = "中华人民共和国简称中国";
        s =  "我们都会知道还是都不会知道这是一个长句子测试。在所有字符集中最知名的可能要数被称为ASCII的7位字符集了。它是美国标准信息交换代码的缩写";
        cout<<s.c_str()<<endl;
        Unigram r(&core_dict);
        Token vec[10000];
        int len = r.split(s,vec,10000);
        cout<<"len of result:"<<len<<endl;
        print(vec,100);
        print(s,vec,len);
        cout<<flush;

    }

    //mseg_full_split("ddd",tks,1000);
    return 0;
}

