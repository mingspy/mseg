# mseg
chinese segmentor for linux version, support lucene, elasticsearch
    C++实现的中文分词器，同时支持java、python调用，并且可以支持luncene以及elasticsearch。c++内部使用UTF8编码格式，而对于java通过JNI调用，把unicode转换成UTF8然后调用c++分词器，最后再转换会java内部使用的unicode，对于python直接使用UTF8格式。该分词器支持的算法包括前向最大切分，后向最大切分，全切分，一元统计分词。其中：
    	速度(m/s)	准确率	召回率	F1
前向最大切分	11.9691	0.943345	0.922851	0.932985
后向最大切分	10.4729	0.94745	0.924768	0.935971
一元	2.40362	0.982346	0.967586	0.97491
全切分	8.49976	0.766791	0.935181	0.842656
viterbi标注器	0.67799	0.95129	0.95129	0.95129

## 环境设置
分词器需要知道词典位置，需要通过分词器配置文件告诉分词器词典路径。设置分词器配置文件位置的方法有两种，一是通过环境变量告诉分词器位置，如在.bash_profile中设置 export MSEG_CONF_PATH=/data0/home/xiulei/workspace/mseg/dict/mseg.conf。二是通过显示调用接口告诉分词器词典位置：seg/mseg.cpp/mseg_config_init(const char * config_path)。python和java版本都对这些接口进行了重新封装。

## 词典编译
解压data.tar.gz
dict 目录下运行make
运行命令 ./builder 即可生成核心词典core.dic
运行命令 ./builder inverse 即可生成逆向词典

## 目录结构
dict/   词典训练代码目录
seg/    分词实现目录
util/   工具集目录
java/   java版本的封装，jar文件为java/mseg-0.0.1.jar(或通过maven重新编译) 该jar会自动从jar包中解压出运行需要的so,存入一个临时文件，然后load JNI版本的libmsegjni.so 具体使用方法参考java/src/main/java/com/mingspy/Test.java
py/     python版本的封装，具体使用方法参考 py/test.py

## C++
通过调用IKnife::split(const string & utf8Str,Token * tokenArr, int tokenArrLen, int flag = 0)来获取分词结果。
共有4个分词器继承自IKnife，并实现了实际的切分操作。
IKnife
|__Flycutter  前向分词器，从前向后进行最大词典匹配切分
   |___Renda  后向分词器，从后向前进行最大词典匹配切分
|__Unigram    一元分词器，使用最短路径计算最优分词策略
|__Paoding    全切分，切出所有可能的词

在mseg.cpp中实现了一个分词模块的封装，建议使用该模块中的接口来保证可移植性。

## python
支持的方法有：
mseg.init(config_path)
mseg.forward_split(utf8_str)
mseg.backward_split(utf8_str)
mseg.smart_split(utf8_str)
mseg.full_split(utf8_str)
mseg.tagging(utf8_str)
具体参考py/test.py

## java
支持的方法定义在MsegJNI.java:
  public static native List<Token> ForwardSplit(String str);
	public static native List<Token> FullSplit(String str);
	public static native List<Token> SmartSplit(String str);
	public static native List<Token> BackwardSplit(String str);
	public static native List<Token> Tagging(String str);
具体使用方法参考 com/mingspy/Test.java 

## elasticsearch支持：
把mseg-0.0.1.jar 放在 elasticsearch-1.4.2/plugins/analysis-mseg/目录下
把词典放在 elasticsearch-1.4.2/config/mseg/目录下
修改 elasticsearch-1.4.2/config/elasticsearch.conf, 加入mseg分词设置:

index:
  analysis:
    analyzer:
      mseg_full:  
        type: org.elasticsearch.index.analysis.MsegAnalyzerProvider
        method: full
      mseg_smart:  
        type: org.elasticsearch.index.analysis.MsegAnalyzerProvider
        method: smart 
      mseg_backward:  
        type: org.elasticsearch.index.analysis.MsegAnalyzerProvider
        method: backward


contact us:
mingspy@163.com qq:65983281 tel:18600806440 xiuleili
