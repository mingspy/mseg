#include <iostream>
#include "knife.hpp"
#include "dict.hpp"
#include <vector>
#include "sparse.hpp"
#include "estimator.hpp"
#include "utils.hpp"
#include "builder.hpp"

using namespace std;
using namespace mingspy;
const int DATA_SIZE = 15;
string testdata[] = {
        "软件和服务",
        "www.sina.com.cn是个好网站",
        "ISNUMBER函数是office办公软件excel中的一种函数\n是脚本语言",
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

string speedstr =  "在所有字符集中，最知名的可能要数被称为ASCII的7位字符集了。它是美国标准信息交换代码（American Standard Code for Information Interchange）的缩写, 为美国英语通信所设计。它由128个字符组成，包括大小写字母、数字0-9、标点符号、非打印字符（换行符、制表符等4个）以及控制字符（退格、响铃等）组成。\n 但是，由于他是针对英语设计的，当处理带有音调标号（形如汉语的拼音）的亚洲文字时就会出现问题。因此，创建出了一些包括255个字符的由ASCII扩展的字符集。其中有一种通常被称为IBM字符集，它把值为128-255之间的字符用于画图和画线，以及一些特殊的欧洲字符。另一种8位字符集是ISO 8859-1Latin 1，也简称为ISOLatin-1。它把位于128-255之间的字符用于拉丁字母表中特殊语言字符的编码，也因此而得名。\n ASCII码格式 欧洲语言不是地球上的唯一语言，因此亚洲和非洲语言并不能被8位字符集所支持。仅汉语字母表（或pictograms）就有80000以上个字符。但是把汉语、日语和越南语的一些相似的字符结合起来，在不同的语言里，使不同的字符代表不同的字，这样只用2个字节就可以编码地球上几乎所有地区的文字。因此，创建了UNICODE编码。它通过增加一个高字节对ISO Latin-1字符集进行扩展，当这些高字节位为0时，低字节就是ISO Latin-1字符。UNICODE支持欧洲、非洲、中东、亚洲（包括统一标准的东亚象形汉字和韩国象形文字）。但是，UNICODE并没有提供对诸如Braille,Cherokee, Ethiopic, Khmer, Mongolian, Hmong, Tai Lu, Tai Mau文字的支持。同时它也不支持如Ahom, Akkadian, Aramaic, Babylonian Cuneiform, Balti, Brahmi, Etruscan, Hittite, Javanese, Numidian, Old Persian Cuneiform, Syrian之类的古老文字。\n 事实证明，对可以用ASCII表示的字符使用UNICODE并不高效，因为UNICODE比ASCII占用大一倍的空间，而对ASCII来说高字节的0对他毫无用处。为了解决这个问题，就出现了一些中间格式的字符集，他们被称为通用转换格式，即UTF（Universal Transformation Format）。常见的UTF格式有：UTF-7, UTF-7.5, UTF-8,UTF-16, 以及 UTF-32。"; 
void testKnife(IKnife & seg)
{
    cout<<"testing "<<seg.getName()<<" start"<<endl;
    vector<Chip> vec;
    Timer t;
    for(int i = 0; i < DATA_SIZE; i++){
        seg.split(testdata[i],vec);
        print(testdata[i],vec);
        vec.clear();
    }
    cout<<"used "<<t.elapsed()<<"s"<<endl;
    cout<<"---------------------------------"<<endl;
}

void testUtf8len(){
    string ss = "abc中国人哈2";
    cout<<ss.length()<<endl;
    for(int i = 0; i < ss.length(); ){
        cout<<i<<" ";
        i += utf8_char_len(ss[i]);
    }
    cout<<endl;
}

void testSegs(){
    Timer timer;
    Dictionary dict;
    dict.open("./core.dic");
    Flycutter fc(&dict);
    Dictionary inversedict;
    inversedict.open("./inverse.dic");
    Renda rd(&inversedict);
    Paoding pao(&dict);
    Unigram ug(&dict);
    Mixture mix(&dict);
    double load_used = timer.elapsed();
    cout<<"load dicts used" << load_used<<endl;

    /*
    testKnife(fc);
    testKnife(rd);
    testKnife(ug);
    testKnife(mix);
    testKnife(pao);
*/
    vector<Chip> chips;
    vector<string> words;
    vector<string> tags;
    ug.split(testdata[0],chips);
    substrs(testdata[0],chips,words);
    Tagger tagger(dict);
    tagger.tagging(words, tags);
    for(int i = 0; i < chips.size(); i ++){
        cout<<words[i]<<"/"<<tags[i]<<" ";
    }
    cout<<endl;
/*
    Estimator est("./est.txt");
    est.estimate(fc);
    est.estimate(rd);
    est.estimate(ug);
    est.estimate(mix);
    est.estimate(pao);
*/
    cout<<"testing  used" << timer.elapsed() - load_used<<endl;
    cout<<"total  used" << timer<<endl;
}
void testGenGraph(){
    Dictionary dict;
    dict.open("./core.dic");
    Timer t;
    double size = speedstr.length();
    Graph g;
    for(int i = 0; i < 1024; i ++){
        //vector<vector<Chip> > m;
        //map<int,int> rows;
        //genWordGraph(dict, speedstr,m,rows);
        genWordGraph(dict, speedstr,g);
        NShortPath npath(g,10);
        npath.calc();
    }
    cout<<"speed is "<<size/t.elapsed()/1024<<endl;
}

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

int main(int argc, char ** argv)
{
    //testUtf8len();
    if (argc > 1){
        prepair_estimate_data(argv[1]);
    }else{
        prepair_estimate_data();
    }
    testSegs();
    //testGenGraph();
}

