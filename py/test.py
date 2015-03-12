#!/bin/env python2.6
#coding:utf-8
import json
import sys
import thread
reload(sys)
sys.setdefaultencoding('utf-8')
import mseg

class MsegAnalyzer:
    initialized = False
    __lock = thread.allocate_lock()
    def __init__(self,path = '/data0/home/xiulei/workspace/mseg/dict/mseg.conf'):
        if not MsegAnalyzer.initialized:
            MsegAnalyzer.__lock.acquire()
            if not MsegAnalyzer.initialized: 
                mseg.init(path)
            MsegAnalyzer.__lock.release()
            MsegAnalyzer.initialized = True
    def split(self,content):
        return mseg.smart_split(content)
if __name__=='__main__':
    th = MsegAnalyzer()
    res=th.split('测试代码 中华人民共和国')
    print 'refcount of res %d' % sys.getrefcount(res)
    print len(res)
    print res
    print json.dumps(res,indent=4,ensure_ascii=False)
