#INCLUDE=-I/opt/apps_install/jdk-1.6.0_25/include -I/opt/apps_install/jdk-1.6.0_25/include/linux
INCLUDE=-I../seg/ -I../util/ -I../dict/
#LIBDIR=-L/data0/home/xiulei/workspace/mseg/seg
#LIB=-lmseg
#CC=g++ -O3 $(INCLUDE) $(LIB) $(LIBDIR) -Wall
#CC=g++ -DNSHORTPATH -O3 $(INCLUDE)  -Wall
CC=g++  $(INCLUDE) -O3 -Wall
#CC=g++ -DDEBUG -g  $(INCLUDE) -Wall
#CC=g++ -g  $(INCLUDE) -Wall

all:wseg_server
.PHONY:all

wseg_server:wseg_server.c
	g++ -o wseg_server  wseg_server.c   -I../seg/ -I../util/ -I../dict/ -I/usr/local/include -L/usr/local/lib  -lpthread
clean:
	rm u*
