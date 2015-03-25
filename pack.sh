#!/bin/sh
rm mseg.tar
tar cvf mseg.tar ./seg/ ./util/ ./test/*cpp ./test/makefile ./server/ ./py/ ./java/ ./pack.sh ./dict/*cpp ./dict/*hpp ./dict/*txt ./dict/mseg.conf ./README.md

sz mseg.tar
