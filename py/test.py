#!/bin/env python
#coding:utf-8
import json
import sys
reload(sys)
sys.path.append('../data')
sys.setdefaultencoding('utf-8')
from mseg_analyzer import MsegAnalyzer
from file_utils import parseEntities
import time

if __name__=='__main__':
    seg = MsegAnalyzer()
    '''
    res=seg.smart_split('测试代码 中华人民共和国')
    print 'refcount of res %d' % sys.getrefcount(res)
    print len(res)
    print res
    print json.dumps(res,indent=4,ensure_ascii=False)
    '''
    data = [] 
    length = 0
    inf = open('../data/people/199801.txt','r')
    outf = open('../test_data.txt','w')
    for line in inf:
        line = line.strip().split(' ')
        entities = parseEntities(line)
        d = ''
        for ent in entities:
            if ent.isCompose:
                for t in ent.sub:
                    d += t.word
            else:
                d+=ent.word
        data.append(d)
        length += len(d)
        outf.writelines('%s\n'%d)
    outf.close()
    print 'total loaded data %d bytes'%length
    start_time = time.time()
    for d in data:
        seg.forward_split(d)
    end_time = time.time()
    secs = end_time - start_time
    print 'forward used %f, speed is %f'%(secs, length / secs / 1024.0/1024.0)

    start_time = time.time()
    for d in data:
        seg.backward_split(d)
    end_time = time.time()
    secs = end_time - start_time
    print 'backward used %f, speed is %f'%(secs, length / secs / 1024.0/1024.0)

    start_time = time.time()
    for d in data:
        seg.smart_split(d)
    end_time = time.time()
    secs = end_time - start_time
    print 'smart used %f, speed is %f'%(secs, length / secs / 1024.0/1024.0)

    start_time = time.time()
    for d in data:
        seg.full_split(d)
    end_time = time.time()
    secs = end_time - start_time
    print 'full used %f, speed is %f'%(secs, length / secs / 1024.0/1024.0)

    '''
    start_time = time.time()
    for d in data:
        seg.tagging(d)
    end_time = time.time()
    secs = end_time - start_time
    print 'tagging used %f, speed is %f'%(secs, length / secs / 1024.0/1024.0)
    '''
