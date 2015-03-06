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
        genWordGraph(*core_dict, speedstr,g);
    }
    cout<<"speed is "<<size/t.elapsed()/1024<<endl;
}

void testNShortPath(const Dictionary * core_dict)
{
    cout<<"testNShortPath"<<endl;
    Timer t;
    double size = speedstr.length();
	Token result[10000];
    for(int i = 0; i < 1024; i ++) {
        Graph g;
        genWordGraph(*core_dict, speedstr,g);
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
    for(int i = 0; i < 1024; i ++) {
        Graph g;
        genWordGraph(*core_dict, speedstr,g);
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
    core_dict.open("./core.dic");
    Dictionary inverse_dict;
    inverse_dict.open("./inverse.dic");
    Dictionary person_dict;
    person_dict.open("./person.dic");
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
        Tagger tagger(&core_dict);
        vector<string> str;
        str.push_back("中国");
        vector<string> tags;
        tagger.tagging(str,tags);
    }

    //mseg_full_split("ddd",tks,1000);
    return 0;
}

