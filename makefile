INCLUDE=-I/opt/apps_install/jdk-1.6.0_25/include -I/opt/apps_install/jdk-1.6.0_25/include/linux
#LIBDIR=-L/opt/sogou_seg_with_tagger/lib64
#LIB=-lencoding -lssplatform -lIQSegmentor -lCoreSegmentor -lunicode-encoding -lsgtagger -lbasic_util -lboost_iostreams
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR)
#CC=g++ -O3 $(INCLUDE)
#CC=g++ -O3 
CC=g++ -g 

all:test builder testseg
.PHONY:all

builder:builder.hpp builder.cpp dict.hpp utils.hpp
	$(CC) builder.cpp -o builder 
test:testda.cpp dict.hpp datrie.hpp sparse.hpp utils.hpp
	$(CC) testda.cpp -o test
testseg:testseg.cpp dict.hpp datrie.hpp sparse.hpp utils.hpp knife.hpp
	$(CC) testseg.cpp -o testseg
clean:
	rm test builder testseg

#libJSegJNI.so:JSegJNI.cpp
#	$(CC) -fPIC -shared JSegJNI.cpp -o libJSegJNI.so 
