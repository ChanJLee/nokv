//
// Created by chan on 2022/11/20.
//

#include "lock.h"
#include "core.h"
#include <cstring>
#include <unistd.h>
#include <fstream>
#include "bctl.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace nkv {
    static const int TYPE_INT32 = 'I';
    static const int TYPE_FLOAT = 'F';
    static const int TYPE_INT64 = 'L';
    static const int TYPE_BOOLEAN = 'B';
    static const int TYPE_STRING = 'S';

    static inline byte *mem_begin(Map *map) { return reinterpret_cast<byte *>(map + sizeof(Map)); }

    static inline byte *mem_end(Map *map) { return mem_begin(map) + map->size_; }

    Lock *gLock;

    static size_t get_entry_size(byte *mem) {
        switch (mem[0]) {
            case TYPE_INT32:
            case TYPE_FLOAT:
                return 5;
            case TYPE_BOOLEAN:
                return 2;
            case TYPE_INT64:
                return 9;
            case TYPE_STRING:
                return strlen((char *) mem + 1) + 1;
        }

        return 0;
    }

    int16_t crc(byte *begin, byte *end) {
        return 0;
    }

    int KV::write(const char * const key, byte *value, byte type, size_t size) {
        // todo resize

        byte *begin = mem_begin(&map_);
        byte *end = mem_end(&map_);

        byte *write_ptr = nullptr;
        read(key, &write_ptr);

        // 重新插入
        if (write_ptr == nullptr) {
            end[0] = type;
            memcpy(end + 1, value, size);
            map_.size_ = map_.size_ + size + 1;
            map_.crc_ = crc(begin, begin + map_.size_);
            return 0;
        }

        size_t prev_size = get_entry_size(write_ptr);
        if (prev_size == 0) {
            /* invalid state */
            return -1;
        }

        if (prev_size == size + 1) {
            write_ptr[0] = type;
            memcpy(write_ptr + 1, value, size);
            map_.crc_ = crc(begin, end);
            return 0;
        }

        // 长度发生了变化就要重排
        size_t offset_size = end - write_ptr - prev_size;
        memcpy(write_ptr, write_ptr + prev_size, offset_size);
        write_ptr[offset_size] = type;
        memcpy(write_ptr + offset_size + 1, value, size);
        map_.size_ = map_.size_ - prev_size + size + 1;
        map_.crc_ = crc(begin, begin + map_.size_);
        return 0;
    }

    int KV::read(const char * const key, byte **value) {
        byte *begin = mem_begin(&map_);
        byte *end = mem_end(&map_);

        while (begin < end) {
            char *entry_key = reinterpret_cast<char *>(begin);
            size_t key_len = strlen(entry_key);
            byte *data = begin + key_len + 1;
            if (strcmp(entry_key, key) == 0) {
                *value = data;
                return 0;
            }

            size_t data_len = get_entry_size(data);
            if (data_len == 0) {
                /* invalid state */
                return -2;
            }

            begin = data + data_len;
        }

        /* not found */
        return -1;
    }

    void KV::flush() {
        ScopedLock lock(lock_);
        fsync(fd_);
    }

    void KV::close() {
        ScopedLock lock(lock_);
        ::close(fd_);
    }

    int KV::write_int32(const char * const key, int32_t v) {
        ScopedLock lock(lock_);
        return write(key, (byte *) &v, TYPE_INT32, 4);
    }

    int KV::write_float(const char * const key, float v) {
        ScopedLock lock(lock_);
        return write(key, (byte *) &v, TYPE_FLOAT, 4);
    }

    int KV::write_int64(const char * const key, int64_t v) {
        ScopedLock lock(lock_);
        return write(key, (byte *) &v, TYPE_INT64, 8);
    }

    int KV::write_boolean(const char * const key, bool v) {
        ScopedLock lock(lock_);
        return write(key, (byte *) &v, TYPE_BOOLEAN, 1);
    }

    int KV::write_string(const char * const key, const char *const v) {
        ScopedLock lock(lock_);
        return write(key, (byte *) v, TYPE_STRING, strlen(v) + 1);
    }

    int KV::read_int32(const char * const key, int32_t &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_float(const char * const key, float &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_int64(const char * const key, int64_t &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_boolean(const char * const key, bool &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_string(const char * const key, char **v) {
        ScopedLock lock(lock_);
        return 0;
    }

    KV *KV::create(const char *file) {
        if (gLock == nullptr) {
            return nullptr;
        }

        ScopedLock lock(*gLock);

        struct stat st{};
        bool new_file = stat(file, &st) != 0;
        int fd = open(file, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            return NULL;
        }

        // avoid BUS error
        if (new_file) {
            st.st_size = _SC_PAGE_SIZE;
            byte buf[_SC_PAGE_SIZE] = {0};
            ::write(fd, buf, _SC_PAGE_SIZE);
        }

        void *mem = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (mem == nullptr) {
            return nullptr;
        }

        return new(mem) KV(fd, st.st_size);
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        kv->flush();
        munmap(kv, kv->capacity_);
        kv->close();
    }

    int init(const char *meta_file) {
        int fd = open(meta_file, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            return -1;
        }

        gLock = new Lock(fd);
        return 0;
    }
}