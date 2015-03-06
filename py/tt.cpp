#include <Python.h>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <list>


using namespace std;

static PyObject *tokenizer_split(PyObject *self, PyObject *args)
{
    PyObject *res = PyList_New(0);
    PyList_Append(res,Py_BuildValue("s","c str"));
    PyObject *wd = PyDict_New();
    PyList_Append(res,wd);
    PyDict_SetItemString(wd,"id",Py_BuildValue("s","str in dict"));
    PyObject *semTags = PyList_New(0);
    PyList_Append(semTags,Py_BuildValue("s","str in list"));
    PyList_Append(res,semTags);

    return res;
}


static PyMethodDef tokenizerMethods[] =
{
    {"split", tokenizer_split, METH_VARARGS, "weibo toeknizer wrapper!"},
    {NULL, NULL}
};

extern "C" void inittokenizer()
{
    PyObject* m= Py_InitModule("tokenizer", tokenizerMethods);
    if (m == NULL)
        return;
}

