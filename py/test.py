#!/bin/env python2.6
#coding:utf-8
import json
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import tokenizer

class TokenizerHandler:
    pid2confs = {}
    def __init__(self):
        pass
    def tokenize(self,content,pid=0):
        conf = TokenizerHandler.pid2confs.get(pid,'/data0/home/liuting1/workspace/tokenizer_keywords/weibo_tokenizer/interface/da/conf/tokenizer.conf')
        return tokenizer.split(conf,content)
if __name__=='__main__':
    th = TokenizerHandler()
    res=th.tokenize('测试代码 中华人民共和国\n刘挺')
    print 'refcount of res %d' % sys.getrefcount(res)
    print len(res)
    print res
    print json.dumps(res,indent=4,ensure_ascii=False)
    while True:
        res=TokenizerHandler().tokenize(raw_input(""))
        print json.dumps(res,indent=4,ensure_ascii=False)
    tokenizer.clear()
