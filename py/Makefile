CC=g++
INC=-I/usr/include/python2.6 -I/usr/lib/python2.6/config -I../seg -I../util
COMMON=-O3 -fPIC -DNDEBUG -lpthread
#COMMON=-g -fPIC -Wall -lpthread
SRC=mseg_wrapper.cpp
EXE=mseg
SO=$(EXE).so
all: so

#exe:
#	$(CC) $(COMMON) $(SRC) $(INC) $(LIBS) /usr/lib64/python2.6/config/libpython2.6.so -o $(EXE)
so:
	$(CC) $(COMMON) $(SRC) $(INC) $(LIBS)  -shared -o $(SO)  
