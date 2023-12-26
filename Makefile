CFLAGS=-O3 -std=c++11
#CFLAGS=-g

all: compressedlut.o
	g++ $(CFLAGS) -o compressedlut compressedlut.o

compressedlut.o: compressedlut.cpp compressedlut.h
	g++ $(CFLAGS) -c compressedlut.cpp

clean: 
	rm *.o compressedlut.exe
