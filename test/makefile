#INCLUDE=-I/opt/apps_install/jdk-1.6.0_25/include -I/opt/apps_install/jdk-1.6.0_25/include/linux
INCLUDE=-I../seg/ -I../util/ -I../dict/
#LIBDIR=-L/data0/home/xiulei/workspace/mseg/seg
#LIB=-lmseg
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR) -Wall
#CC=g++ -DNSHORTPATH -O3 $(INCLUDE)  -Wall
CC=g++  $(INCLUDE) -O3 -Wall
#CC=g++ -DDEBUG -g  $(INCLUDE) -Wall
#CC=g++ -g  $(INCLUDE) -Wall

all:uda useg
.PHONY:all

uda:test_datrie.cpp ../seg/dict.hpp ../util/datrie.hpp
	$(CC) test_datrie.cpp -o uda
useg:test_seg.cpp ../seg/dict.hpp ../seg/knife.hpp
	$(CC) test_seg.cpp -o useg
#uepoll:test_epoll.cpp
#	$(CC) test_epoll.cpp -o uepoll
#uevent:test_event.cpp
#	$(CC) -levent test_event.cpp -o uevent
clean:
	rm u*
