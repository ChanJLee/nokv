CC = g++
CFLAGS = -DNKV_UNIT_TEST -std=c++17 -Wunused-result -g -rdynamic

all: clean build

build: nokv.o kv_lock.o main.o multi_process.o kv_map.o kv_meta.o kv_cache.o read_all.o
	$(CC) kv_lock.o nokv.o main.o kv_map.o kv_meta.o kv_cache.o -lz -lpthread -o main
	$(CC) kv_lock.o nokv.o multi_process.o kv_map.o kv_meta.o kv_cache.o -lz -lpthread -o multi_process
	$(CC) kv_lock.o nokv.o read_all.o kv_map.o kv_meta.o kv_cache.o -lz -lpthread -o read_all

read_all.o:
	$(CC) $(CFLAGS) -c read_all.cpp

kv_meta.o:
	$(CC) $(CFLAGS) -D st_mtim=st_mtimespec -c ../../main/cpp/kv_meta.cpp

multi_process.o:
	$(CC) $(CFLAGS) -c multi_process.cpp

kv_cache.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/kv_cache.cpp

nokv.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/nokv.cpp

kv_lock.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/kv_lock.cpp

kv_map.o:
	$(CC) $(CFLAGS) -c ../../main/cpp/kv_map.cpp

main.o:
	$(CC) $(CFLAGS) -c main.cpp

clean:
	-rm *.o
	-rm main