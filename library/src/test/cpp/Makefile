CC = g++
CFLAGS = -DANR_UNIT_TEST -std=c++11 -O3

all: clean build

build: core.o lock.o main.o
	$(CC) lock.o core.o main.o -o main

core.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/core.cpp

lock.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/lock.cpp

main.o:
	$(CC) $(CFLAGS) -c main.cpp

clean:
	-rm *.o
	-rm main