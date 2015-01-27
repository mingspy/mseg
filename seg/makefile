INCLUDE=-I/opt/apps_install/jdk-1.6.0_25/include -I/opt/apps_install/jdk-1.6.0_25/include/linux
#LIBDIR=-L/opt/sogou_seg_with_tagger/lib64
#LIB=-lencoding -lssplatform -lIQSegmentor -lCoreSegmentor -lunicode-encoding -lsgtagger -lbasic_util -lboost_iostreams
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR)
#CC=g++ -O3 $(INCLUDE)
CC=g++ -O3 
#CC=g++ -DDEBUG -g 

all:builder
.PHONY:all

builder:builder.hpp builder.cpp dict.hpp
	$(CC) builder.cpp -o builder 
clean:
	rm builder

#libJSegJNI.so:JSegJNI.cpp
#	$(CC) -fPIC -shared JSegJNI.cpp -o libJSegJNI.so 
