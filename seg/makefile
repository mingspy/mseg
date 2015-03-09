INCLUDE=-I../util
#LIBDIR=-L/opt/sogou_seg_with_tagger/lib64
#LIB=-lencoding -lssplatform -lIQSegmentor -lCoreSegmentor -lunicode-encoding -lsgtagger -lbasic_util -lboost_iostreams
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR)
#CC=g++ -O3 $(INCLUDE)
CC= g++ $(INCLUDE) -O3 
#CC=g++ -DDEBUG -g 

#all:builder libmseg.so
all:builder
.PHONY:all

builder:builder.hpp builder.cpp dict.hpp
	$(CC) builder.cpp -o builder 
libmseg.so:mseg.cpp
	$(CC) -fPIC -shared mseg.cpp -o libmseg.so 
clean:
	rm builder libmseg.so
