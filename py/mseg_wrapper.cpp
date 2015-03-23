#include <Python.h>
#include <iostream>
#include <string>

#include "mseg.cpp"

using namespace std;

#define uki_DICT_ADD_NEW(dict,key,valtype,val) \
{\
    PyObject *obj = Py_BuildValue(valtype,val); \
    PyDict_SetItemString(dict,key,obj);\
    Py_DECREF(obj); \
}
#define uki_DICT_ADD(dict,key,obj) \
{\
    PyDict_SetItemString(dict,key,obj);\
    Py_DECREF(obj); \
}
#define uki_LIST_APPEND_NEW(list,valtype,val) \
{\
    PyObject *obj = Py_BuildValue(valtype,val); \
    PyList_Append(list,obj);\
    Py_DECREF(obj);\
}
#define uki_LIST_APPEND(list,obj) \
{\
    PyList_Append(list,obj);\
    Py_DECREF(obj);\
}

//build dict object for one token
PyObject *formatToken(const string &str,const Token &t)
{
    PyObject *wd = PyDict_New();
    string s = str.substr(t.start, t.end - t.start);
    uki_DICT_ADD_NEW(wd,"start","i",t.start);
    uki_DICT_ADD_NEW(wd,"end","i",t.end);
    uki_DICT_ADD_NEW(wd,"pos","i",t.pos);
    uki_DICT_ADD_NEW(wd,"word","s",s.c_str());
    return wd;
}

//function to convert the tokenzer results to python object
void toPythonTokenList(const string & str,Token *tokenArr,int len, PyObject *res)
{
    for(int i = 0; i < len; i++)
    {
        PyObject *tp = formatToken(str,tokenArr[i]);
        if(tp!=NULL){
            uki_LIST_APPEND(res,tp);
        }
    }
}
typedef int (*pfunc)(const char *,Token * ,int );

PyObject * CALL_MSEG_METHOD(PyObject *args, pfunc func ) {
    PyObject *res = PyList_New(0);
    const char * str; 
    if (!PyArg_ParseTuple(args, "s", &str)) 
        return res; 
    mingspy::Token result[MIN_TOKEN_BUFSIZE]; 
    int len = func(str, result, MIN_TOKEN_BUFSIZE); 
    toPythonTokenList(str, result, len,res); 
    return res; 
}


static PyObject * init(PyObject * self, PyObject *args){
    const char *conf;
    if (!PyArg_ParseTuple(args, "s", &conf))
        return NULL;
    if (conf && fileExist(conf))
        mseg_config_init(conf);
    cout<<"config setted, now init mseg modulus."<<endl;
    mseg_init();
    cout<<"mseg modulus init finished."<<endl;
    PyObject *obj = Py_BuildValue("s","success"); 
    return obj;
}

static PyObject * forward_split(PyObject * self, PyObject *args){
    return CALL_MSEG_METHOD(args,mseg_forward_split);
}
static PyObject * backward_split(PyObject * self, PyObject *args){
    return CALL_MSEG_METHOD(args,mseg_backward_split);
}
static PyObject * smart_split(PyObject * self, PyObject *args){
    return CALL_MSEG_METHOD(args,mseg_smart_split);
}
static PyObject * full_split(PyObject * self, PyObject *args){
    return CALL_MSEG_METHOD(args,mseg_full_split);
}

static PyObject * tagging(PyObject * self, PyObject *args){
    return CALL_MSEG_METHOD(args,mseg_tagging);
}

static PyMethodDef msegMethods[] =
{
    {"init", init, METH_VARARGS, "init and load dicts"},
    {"forward_split", forward_split, METH_VARARGS, "forward split!"},
    {"backward_split", backward_split, METH_VARARGS, "backward split!"},
    {"smart_split", smart_split, METH_VARARGS, "smart split!"},
    {"full_split", full_split, METH_VARARGS, "full split!"},
    {"tagging", tagging, METH_VARARGS, "split and pos tagging!"},
    {NULL, NULL,}
};

extern "C" void initmseg()
{
    PyObject* m= Py_InitModule("mseg", msegMethods);
    if (m == NULL)
        return;
}

