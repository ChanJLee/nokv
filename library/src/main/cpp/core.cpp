//
// Created by chan on 2022/11/20.
//

#include "lock.h"
#include "core.h"
#include <cstring>
#include <unistd.h>

namespace nkv {
    static const int TYPE_INT32 = 'I';
    static const int TYPE_FLOAT = 'F';
    static const int TYPE_INT64 = 'L';
    static const int TYPE_BOOLEAN = 'B';
    static const int TYPE_STRING = 'S';

    static inline byte *mem_begin(Map *map) { return reinterpret_cast<byte *>(map + sizeof(Map)); }

    static inline byte *mem_end(Map *map) { return mem_begin(map) + map->size_; }

    static size_t get_entry_size(byte *mem) {
        switch (mem[0]) {
            case TYPE_INT32:
            case TYPE_FLOAT:
                return 5;
            case TYPE_BOOLEAN:
                return 2;
                break;
            case TYPE_INT64:
                return 9;
                break;
            case TYPE_STRING:
                return strlen((char *) mem + 1) + 1;
        }

        return 0;
    }

    int16_t crc(byte *begin, byte *end) {
        return 0;
    }

    int KV::write(char *key, byte *value, size_t size) {
        byte *begin = mem_begin(&map_);
        byte *end = mem_end(&map_);

        byte *write_ptr = nullptr;
        read(key, &write_ptr);

        // 重新插入
        if (write_ptr == nullptr) {
            memcpy(end, value, size);
            map_.crc_ = crc(begin, end);
            map_.size_ += size;
            return 0;
        }

        size_t prev_size = get_entry_size(write_ptr);
        if (prev_size == 0) {
            /* invalid state */
            return -1;
        }

        if (prev_size == size) {
            memcpy(end, value, size);
            map_.crc_ = crc(begin, end);
            return 0;
        }

        // 长度发生了变化就要重排
        size_t offset_size = end - write_ptr - prev_size;
        memcpy(write_ptr, write_ptr + prev_size, offset_size);
        memcpy(write_ptr + offset_size, value, size);
        map_.size_ = map_.size_ - prev_size + size;
        map_.crc_ = crc(begin, begin + map_.size_);
        return 0;
    }

    int KV::read(char *key, byte **value) {
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

    int KV::write_int32(char *key, int32_t v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::write_float(char *key, float v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::write_int64(char *key, int64_t v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::write_boolean(char *key, bool v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::write_string(char *key, const char *const v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_int32(char *key, int32_t &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_float(char *key, float &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_int64(char *key, int64_t &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_boolean(char *key, bool &v) {
        ScopedLock lock(lock_);
        return 0;
    }

    int KV::read_string(char *key, char **v) {
        ScopedLock lock(lock_);
        return 0;
    }
}