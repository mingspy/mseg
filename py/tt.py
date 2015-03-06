#!/bin/env python2.6
#coding:utf-8
import json
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import tokenizer

if __name__=='__main__':
    res=tokenizer.split('测试代码 中华人民共和国\n刘挺')
    print 'refcount of res %d' % sys.getrefcount(res)
    print res[0],'|res[0] is a str|refcount= %d' % sys.getrefcount(res[0])
    print res[1],'|res[1] is a dict|,refcount= %d' % sys.getrefcount(res[1])
    print res[1]['id'],'|res[1][\'id\'] is a string |refcount= %d' % sys.getrefcount(res[1]['id'])
    print res[2],'|res[2] is a list|refcount= %d' % sys.getrefcount(res[2])
    print res[2][0],'|res[2][0] is a str|refcount= %d' % sys.getrefcount(res[2][0])
