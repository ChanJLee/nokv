CC = g++
CFLAGS = -DNKV_UNIT_TEST -std=c++11 -O3

all: clean build

build: core.o lock.o main.o multi_process.o
	$(CC) lock.o core.o main.o -lz -o main
	$(CC) lock.o core.o multi_process.o -lz -o multi_process

multi_process.o:
	$(CC) $(CFLAGS) -c multi_process.cpp

core.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/core.cpp

lock.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/lock.cpp

main.o:
	$(CC) $(CFLAGS) -c main.cpp

clean:
	-rm *.o
	-rm main