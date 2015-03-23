#!/bin/sh
rm mseg.tar
tar cvf mseg.tar ./seg/ ./util/ ./test/*cpp ./test/makefile ./server/ ./py/*cpp ./py/*py ./py/makefile ./java/*cpp ./java/makefile ./java/*sh ./pack.sh ./dict/*cpp ./dict/*hpp ./dict/*txt 

sz mseg.tar
