#INCLUDE=-I/opt/apps_install/jdk-1.6.0_25/include -I/opt/apps_install/jdk-1.6.0_25/include/linux
INCLUDE=-I../seg/ -I../util/
#LIBDIR=-L/opt/sogou_seg_with_tagger/lib64
#LIB=-lencoding -lssplatform -lIQSegmentor -lCoreSegmentor -lunicode-encoding -lsgtagger -lbasic_util -lboost_iostreams
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR)
#CC=g++ -DNSHORTPATH -O3 $(INCLUDE) 
CC=g++ -DBREAKDOWN -O3 $(INCLUDE) 
#CC=g++ -DDEBUG -g $(INCLUDE)

all:uda useg uepoll uevent
.PHONY:all

uda:test_datrie.cpp ../seg/dict.hpp ../util/datrie.hpp
	$(CC) test_datrie.cpp -o uda
useg:test_seg.cpp ../seg/dict.hpp ../seg/knife.hpp
	$(CC) test_seg.cpp -o useg
uepoll:test_epoll.cpp
	$(CC) test_epoll.cpp -o uepoll
uevent:test_event.cpp
	$(CC) -levent test_event.cpp -o uevent
clean:
	rm u*
