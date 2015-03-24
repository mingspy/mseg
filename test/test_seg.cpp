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

void testKnife(IKnife & seg, const Dictionary * core)
{
    cout<<"testing "<<seg.getName()<<" start"<<endl;
    Token vec[10000];
    Timer t;
    for(int i = 0; i < DATA_SIZE; i++) {
        int len = seg.split(testdata[i],vec,10000, true);
        print(testdata[i],vec, len, core);
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
    fc.setPosDict(core_dict);
    rd.setPosDict(core_dict);
    pao.setPosDict(core_dict);
    ug.setPosDict(core_dict);

    testKnife(fc, core_dict);
    testKnife(rd, core_dict);
    testKnife(ug, core_dict);
    testKnife(pao, core_dict);
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
        NShortPath::getBestPath(g,result,6);
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
        ShortPath::getBestPath(g,result);
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
        mseg_init();
        Token tokens[1000];
        string str = " 河北省大厂回族自治县评剧团河北省科协山西省太原市委宣传部山西省曲艺团山西省新华书店内蒙古自治区哲里木盟奈曼旗科协内蒙古自治区赤峰市敖汉旗乌兰牧骑内蒙古自治区医院辽宁省沈阳市委宣传部辽宁省大连市委宣传部辽宁省计生委吉林省农业科学院吉林省民间艺术团吉林省科委黑龙江省肇东市哈尔滨医科大学附属第二医院黑龙江省五常市文化局上海科学普及出版社上海滑稽剧团上海医科大学附属中山医院江苏省新华书店江苏省歌舞剧院江苏省南京市卫生局江苏省中医院山东省莱州市委宣传部山东歌舞剧院山东工业大学团委安徽省合肥市委宣传部安徽省宁国市文化局安徽省合肥市卫生局浙江省科委社会发展处浙江省杭州市农业局科教处浙江医科大学江西农业大学江西省峡江县新华书店福建省委宣传部福建省龙岩市汉剧团福建医科大学附属第一医院河南省农业科学院河南省鄢陵县豫剧团湖北省科委湖北医科大学附属第二医院湖北省武汉市图书馆湖北省楚剧团湖南花鼓戏剧院湖南省农科院湖南省长沙市卫生局湖南省新华书店广东省广州市委宣传部广东省话剧院广西壮族自治区科委社会发展与科学普及处广西壮族自治区文化厅广西壮族自治区人民医院海南省农业科学研究院海南省琼剧院四川省科委四川省营山县百花剧团四川省泸州医学院附属医院华西医科大学附属第一医院成都中医药大学附属医院重庆市科委科技干部处重庆市少年儿童图书馆重庆医科大学附属第二医院云南省委宣传部宣传教育处云南省科委农村科技处贵州省贵阳市委宣传部贵州省新华书店贵州省贵阳市妇幼保健院西藏自治区科委农牧处西藏自治区日喀则地区人民医院西藏自治区藏剧团陕西省科协陕西省西安市委宣传部甘肃省曲艺团甘肃定西行署科技处";
        int len = mseg_smart_split(str.c_str(),tokens, 1000);
        print(str,tokens,len);
        /*
        Estimator est;
        for (int i = 0; i < est.est_data.size(); i ++){
            len = mseg_smart_split(est.est_data[i].c_str(),tokens, 1000);
            vector<string> vec;
            substrs(est.est_data[i], tokens, len,vec);
        }
        */
    }

    //mseg_full_split("ddd",tks,1000);
    return 0;
}

