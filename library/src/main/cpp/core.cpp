//
// Created by chan on 2022/11/20.
//

#include "lock.h"
#include "core.h"
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "log.h"
#include <zlib.h>
#include <stddef.h>

namespace nkv {

    static inline byte *mem_begin(Map *map) { return (byte *) map + sizeof(Map); }

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
                return strlen((char *) mem + 1) + 2;
        }

        return 0;
    }

    int KV::write(const char *const key, byte *value, byte type, size_t size) {
        // todo resize

        byte *begin = mem_begin(map_);
        byte *end = mem_end(map_);

        byte *write_ptr = nullptr;
        read(key, &write_ptr);

        // 新值
        if (write_ptr == nullptr) {
            size_t key_len = strlen(key) + 1;
            memcpy(end, key, key_len);
            end += key_len;
            end[0] = type;
            memcpy(end + 1, value, size);
            map_->size_ = map_->size_ + size + 1 /* type */ + key_len;
            map_->crc_ = crc32(0, begin, map_->size_);
            return 0;
        }

        // 旧值
        size_t prev_size = get_entry_size(write_ptr);
        if (prev_size == 0) {
            /* invalid state */
            return -1;
        }

        if (prev_size == size + 1) {
            write_ptr[0] = type;
            memcpy(write_ptr + 1, value, size);
            map_->crc_ = crc32(0, begin, map_->size_);
            return 0;
        }

        // 长度发生了变化就要重排
        size_t key_len = strlen(key) + 1;
        size_t offset_size = end - write_ptr - prev_size;
        memcpy(write_ptr - key_len, write_ptr + prev_size, offset_size);
        write_ptr = write_ptr - key_len + offset_size;
        memcpy(write_ptr, key, key_len);
        write_ptr[key_len] = type;
        memcpy(write_ptr + key_len + 1, value, size);
        map_->size_ = map_->size_ - prev_size + size + 1;
        map_->crc_ = crc32(0, begin, map_->size_);
        return 0;
    }

    int KV::read(const char *const key, byte **value) {
        byte *begin = mem_begin(map_);
        byte *end = mem_end(map_);

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
        ::msync(map_, capacity_, MS_SYNC);
    }

    void KV::close() {
        ::close(fd_);
    }

    int KV::check_kv(KV *kv) {
        if (kv == nullptr) {
            return -1;
        }

        if (kv->map_->size_ == 0) {
            return 0;
        }

        byte *begin = mem_begin(kv->map_);
        int crc = crc32(0, begin, kv->map_->size_);
        LOGD("prev crc: 0x%x, current crc: 0x%x", kv->map_->crc_, crc);
        return crc != kv->map_->crc_;
    }

    KV *KV::create(const char *file) {
        if (gLock == nullptr) {
            LOGD("init module first!");
            return nullptr;
        }

        ScopedLock lock(*gLock);

        struct stat st{};
        bool new_file = stat(file, &st) != 0;
        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return NULL;
        }

        // avoid BUS error
        if (new_file) {
            size_t size = getpagesize();
            st.st_size = size;
            byte *buf = (byte *) malloc(size);
            buf[0] = 'n';
            buf[1] = 'k';
            buf[2] = 'v';
            buf[3] = '\n';
            // order
            uint16_t magic = 0x1234;
            memcpy(buf + 4, &magic, sizeof(magic));
            // version
            magic = 0x0100;
            memcpy(buf + 4 + sizeof(magic), &magic, sizeof(magic));
            ::write(fd, buf, size);
            free(buf);
            fsync(fd);
        }

        void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == nullptr) {
            LOGI("mmap %s failed", file);
            return nullptr;
        }

#ifdef NKV_UNIT_TEST
        ptrdiff_t ptr = reinterpret_cast<ptrdiff_t>(mem);
        LOGD("mem align %lu, %td, %lu", ptr % sizeof(Map), ptr, sizeof(Map));
#endif

        KV *kv = new KV(fd, st.st_size, mem);
        if (!new_file && check_kv(kv)) {
            LOGD("check crc failed");
            kv->close();
            delete kv;
            remove(file);
            return nullptr;
        }
        return kv;
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        kv->flush();
        munmap(kv->map_, kv->capacity_);
        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const char *const, const byte *, byte, size_t)> &fnc) {
        ScopedLock lock(lock_);
        byte *begin = mem_begin(map_);
        byte *end = mem_end(map_);

        while (begin < end) {
            char *entry_key = reinterpret_cast<char *>(begin);
            size_t key_len = strlen(entry_key);
            byte *data = begin + key_len + 1;
            size_t data_len = get_entry_size(data);
            if (data_len == 0) {
                /* invalid state */
                return -2;
            }

            fnc((const char *const) begin, data + 1, data[0], data_len - 1);

            begin = data + data_len;
        }
        return 0;
    }

    bool KV::contains(const char *const key) {
        ScopedLock lock(lock_);
        byte *ptr = nullptr;
        if (read(key, &ptr)) {
            return -1;
        }
        return ptr != nullptr;
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