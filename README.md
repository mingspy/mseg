# mseg<br />
chinese segmentor for linux version, support lucene, elasticsearch<br />
&nbsp;&nbsp;&nbsp; C++实现的中文分词器，同时支持java、python调用，并且可以支持luncene以及elasticsearch。c++内部使用UTF8编码格式，而对于java通过JNI调用，把unicode转换成UTF8然后调用c++分词器，最后再转换会java内部使用的unicode，对于python直接使用UTF8格式。该分词器支持的算法包括前向最大切分，后向最大切分，全切分，一元统计分词。其中：<br />
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;速度(m/s)&nbsp;&nbsp; &nbsp;准确率&nbsp;&nbsp; &nbsp;召回率&nbsp;&nbsp; &nbsp;F1<br />
前向最大切分&nbsp;&nbsp; &nbsp;11.9691&nbsp;&nbsp; &nbsp;0.943345&nbsp;&nbsp; &nbsp;0.922851&nbsp;&nbsp; &nbsp;0.932985<br />
后向最大切分&nbsp;&nbsp; &nbsp;10.4729&nbsp;&nbsp; &nbsp;0.94745&nbsp;&nbsp; &nbsp;0.924768&nbsp;&nbsp; &nbsp;0.935971<br />
一元&nbsp;&nbsp; &nbsp;2.40362&nbsp;&nbsp; &nbsp;0.982346&nbsp;&nbsp; &nbsp;0.967586&nbsp;&nbsp; &nbsp;0.97491<br />
全切分&nbsp;&nbsp; &nbsp;8.49976&nbsp;&nbsp; &nbsp;0.766791&nbsp;&nbsp; &nbsp;0.935181&nbsp;&nbsp; &nbsp;0.842656<br />
viterbi标注器&nbsp;&nbsp; &nbsp;0.67799&nbsp;&nbsp; &nbsp;0.95129&nbsp;&nbsp; &nbsp;0.95129&nbsp;&nbsp; &nbsp;0.95129<br />
<br />
## 环境设置<br />
分词器需要知道词典位置，需要通过分词器配置文件告诉分词器词典路径。设置分词器配置文件位置的方法有两种，一是通过环境变量告诉分词器位置，如在.bash_profile中设置 export MSEG_CONF_PATH=/data0/home/xiulei/workspace/mseg/dict/mseg.conf。二是通过显示调用接口告诉分词器词典位置：seg/mseg.cpp/mseg_config_init(const char * config_path)。python和java版本都对这些接口进行了重新封装。<br />
<br />
## 词典编译<br />
解压data.tar.gz<br />
dict 目录下运行make<br />
运行命令 ./builder 即可生成核心词典core.dic<br />
运行命令 ./builder inverse 即可生成逆向词典<br />
<br />
## 目录结构<br />
dict/&nbsp;&nbsp; 词典训练代码目录<br />
seg/&nbsp;&nbsp;&nbsp; 分词实现目录<br />
util/&nbsp;&nbsp; 工具集目录<br />
java/&nbsp;&nbsp; java版本的封装，jar文件为java/mseg-0.0.1.jar(或通过maven重新编译) 该jar会自动从jar包中解压出运行需要的so,存入一个临时文件，然后load JNI版本的libmsegjni.so 具体使用方法参考java/src/main/java/com/mingspy/Test.java<br />
py/&nbsp;&nbsp;&nbsp;&nbsp; python版本的封装，具体使用方法参考 py/test.py<br />
<br />
## C++<br />
通过调用IKnife::split(const string &amp; utf8Str,Token * tokenArr, int tokenArrLen, int flag = 0)来获取分词结果。<br />
共有4个分词器继承自IKnife，并实现了实际的切分操作。<br />
IKnife<br />
|__Flycutter&nbsp; 前向分词器，从前向后进行最大词典匹配切分<br />
&nbsp;&nbsp; |___Renda&nbsp; 后向分词器，从后向前进行最大词典匹配切分<br />
|__Unigram&nbsp;&nbsp;&nbsp; 一元分词器，使用最短路径计算最优分词策略<br />
|__Paoding&nbsp;&nbsp;&nbsp; 全切分，切出所有可能的词<br />
<br />
在mseg.cpp中实现了一个分词模块的封装，建议使用该模块中的接口来保证可移植性。<br />
<br />
## python<br />
支持的方法有：<br />
mseg.init(config_path)<br />
mseg.forward_split(utf8_str)<br />
mseg.backward_split(utf8_str)<br />
mseg.smart_split(utf8_str)<br />
mseg.full_split(utf8_str)<br />
mseg.tagging(utf8_str)<br />
具体参考py/test.py<br />
<br />
## java<br />
支持的方法定义在MsegJNI.java:<br />
&nbsp; public static native List&lt;Token&gt; ForwardSplit(String str);<br />
&nbsp;&nbsp; &nbsp;public static native List&lt;Token&gt; FullSplit(String str);<br />
&nbsp;&nbsp; &nbsp;public static native List&lt;Token&gt; SmartSplit(String str);<br />
&nbsp;&nbsp; &nbsp;public static native List&lt;Token&gt; BackwardSplit(String str);<br />
&nbsp;&nbsp; &nbsp;public static native List&lt;Token&gt; Tagging(String str);<br />
具体使用方法参考 com/mingspy/Test.java <br />
<br />
## elasticsearch支持：<br />
把mseg-0.0.1.jar 放在 elasticsearch-1.4.2/plugins/analysis-mseg/目录下<br />
把词典放在 elasticsearch-1.4.2/config/mseg/目录下<br />
修改 elasticsearch-1.4.2/config/elasticsearch.conf, 加入mseg分词设置:<br />
<br />
index:<br />
&nbsp; analysis:<br />
&nbsp;&nbsp;&nbsp; analyzer:<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; mseg_full: &nbsp;<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; type: org.elasticsearch.index.analysis.MsegAnalyzerProvider<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; method: full<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; mseg_smart: &nbsp;<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; type: org.elasticsearch.index.analysis.MsegAnalyzerProvider<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; method: smart <br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; mseg_backward: &nbsp;<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; type: org.elasticsearch.index.analysis.MsegAnalyzerProvider<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; method: backward<br />
<br />
<br />
## contact us:<br />
xiuleili&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; mail: mingspy@163.com&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; qq:65983281&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; tel:18600806440<br />
YaDongCheng&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; mail: 971596136@qq.com&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; qq:971596136&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; tel:18810079344<br />
GengHongChun.&nbsp; mail: xinshou_2008@qq.com<br />
<br />
