#!/bin/env python
#coding:utf-8
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
    def smart_split(self,content):
        return mseg.smart_split(content)
    def forward_split(self,content):
        return mseg.forward_split(content)
    def backward_split(self,content):
        return mseg.backward_split(content)
    def tagging(self,content):
        return mseg.tagging(content)
    def full_split(self,content):
        return mseg.full_split(content)
