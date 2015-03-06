#include <Python.h>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "tokenizer.h"

using namespace std;
using namespace weibo::tokenizer;

//key is config file path, value is tokenizer pointer
static map<string,Tokenizer*> tokenizers;
Tokenizer *getTokenizer(const char *conf)
{
//    Tokenizer* pTokenizer = Tokenizer::CreateTokenizer();
    map<string,Tokenizer*>::iterator itr = tokenizers.find(conf);
    if(itr==tokenizers.end())
    {
        Tokenizer* pTokenizer = Tokenizer::CreateTokenizer();
        if(NULL==pTokenizer)
        {
            cerr << "Cannot create instance of tokenizer! exit.." << endl;
            return NULL;//Py_BuildValue("i", sts);
        }
        int ret = pTokenizer->Init(conf);
        if(ret<0)
        {
            cerr << "Tokenizer init failed! " << endl;
            return NULL;
        }
//        tokenizers.insert(make_pair(conf,pTokenizer));
        tokenizers[conf] = pTokenizer;
        return pTokenizer;
    }
    return itr->second;
}


//build dict object for one token
PyObject *formatToken(Token *t)
{
//    cout<<int(t->byteLength)<<endl;
    if(NULL==t)
        return NULL;
    PyObject *wd = PyDict_New();
    string s = string(t->pWord,t->byteLength);
//    如果是空格，下面这句在c++的测试程序中会core dump
    PyDict_SetItemString(wd,"id",Py_BuildValue("i",t->wordId));
    PyDict_SetItemString(wd,"prop",Py_BuildValue("i",t->property));
    PyDict_SetItemString(wd,"pos_tag",Py_BuildValue("i",t->posTagId));
    PyDict_SetItemString(wd,"word",Py_BuildValue("s",s.c_str()));
    PyDict_SetItemString(wd,"score",Py_BuildValue("i",int(t->weight)));
    PyDict_SetItemString(wd,"len",Py_BuildValue("i",int(t->charCount)));
    PyDict_SetItemString(wd,"offset",Py_BuildValue("i",int(t->charOffset)));
    PyDict_SetItemString(wd,"sem_tag_num",Py_BuildValue("i",t->semTagNum));
    PyObject *semTags = PyList_New(0);
    for(int i=0;i<t->semTagNum;i++)
        PyList_Append(semTags,Py_BuildValue("i",t->pSemTagId[i]));
    PyDict_SetItemString(wd,"sem_tags",semTags);
//    cout<<"id:"<<t->wordId<<" "<<"word:"<<s<<endl;
    PyDict_SetItemString(wd,"small_word_num",Py_BuildValue("i",t->childNum));
    if(t->childNum>0)
    {
        PyObject *smallWords = PyList_New(0);
        for(int i=0;i<t->childNum;++i)
        {
            PyList_Append(smallWords,Py_BuildValue("s",string(t->pChild[i].pWord,t->pChild[i].byteLength).c_str()));
        }
        PyDict_SetItemString(wd,"small_words",smallWords);
    }

    return wd;
}
//function to convert the tokenzer results to python object
void formatResult(SegResult *pSegResult,PyObject *res)
{
    if(NULL==pSegResult)
        return;
    Token* cur = pSegResult->FirstToken();
    int wcount = 0;
    while(cur!=NULL)
    {
        PyObject *tp = formatToken(cur);
        if(tp!=NULL)
            PyList_Append(res,tp);
        wcount += 1;
        cur = cur->pNext;
    }
//    cout<<"word count "<<wcount<<endl;
}

static PyObject *tokenizer_split(PyObject *self, PyObject *args)
{
    const char *conf;
    const char *content;
    PyObject *res = PyList_New(0);
    if (!PyArg_ParseTuple(args, "ss", &conf,&content))
        return res;
    Tokenizer* pTokenizer = getTokenizer(conf);
    if (NULL==pTokenizer)
        return res;
    SegResult* pSegResult = pTokenizer->CreateSegResult();
    int ret = pTokenizer->Tokenize(content,strlen(content),pSegResult,UTF_8,SMALL_WORD|NORMAL_WORD);
    if(ret<0)
    {
        cerr<<"Tokenize() failed!"<<endl;
        return NULL;
    }
    formatResult(pSegResult,res);
    delete pSegResult;
    return res;
}

static PyObject *tokenizer_clear(PyObject *self,PyObject *args)
{
    
    for(map<string,Tokenizer*>::iterator itr = tokenizers.begin();itr!=tokenizers.end();++itr)
        delete itr->second;
    tokenizers.clear();
    return Py_BuildValue("i",0);
}

static PyMethodDef tokenizerMethods[] =
{
    {"split", tokenizer_split, METH_VARARGS, "weibo toeknizer wrapper!"},
    {"clear", tokenizer_clear, METH_VARARGS, "clear toeknizer objects!"},
    {NULL, NULL}
};

extern "C" void inittokenizer()
{
    PyObject* m= Py_InitModule("tokenizer", tokenizerMethods);
    if (m == NULL)
        return;
}

void test()
{
    tokenizer_split(NULL,Py_BuildValue("ss","/data0/home/liuting1/workspace/tokenizer_keywords/weibo_tokenizer/interface/da/conf/tokenizer.conf","测试代码 中华人民共和国。"));
}
int main(int argc, char** argv)
{
    test();
}

