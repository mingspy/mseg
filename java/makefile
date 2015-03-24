INCLUDE=-I/usr/java/jdk1.7.0_51/include -I/usr/java/jdk1.7.0_51/include/linux -I../seg -I../util
#LIBDIR=-L/opt/sogou_seg_with_tagger/lib64
#LIB=-lencoding -lssplatform -lIQSegmentor -lCoreSegmentor -lunicode-encoding -lsgtagger -lbasic_util -lboost_iostreams
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR) -Wall
CC=g++ -O3 $(INCLUDE) -Wall

libmsegjni.so:mseg_jni.cpp ../seg/knife.hpp ../seg/mseg.cpp
	$(CC) -fPIC -shared  mseg_jni.cpp -o libmsegjni.so 

clean:
	rm libmsegjni.so

