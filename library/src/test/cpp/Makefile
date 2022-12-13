CC = g++
CFLAGS = -DNKV_UNIT_TEST -std=c++17 -Wunused-result -g -rdynamic

all: clean build

build: core.o lock.o main.o multi_process.o kv.o meta.o
	$(CC) lock.o core.o main.o kv.o meta.o -lz -lpthread -o main
	$(CC) lock.o core.o multi_process.o kv.o meta.o -lz -lpthread -o multi_process

meta.o:
	$(CC) $(CFLAGS) -D st_mtim=st_mtimespec -c ../../main/cpp/meta.cpp

multi_process.o:
	$(CC) $(CFLAGS) -c multi_process.cpp

core.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/core.cpp

lock.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/lock.cpp

kv.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/kv.cpp

main.o:
	$(CC) $(CFLAGS) -c main.cpp

clean:
	-rm *.o
	-rm main