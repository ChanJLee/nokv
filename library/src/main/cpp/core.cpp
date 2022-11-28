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

namespace nokv {

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

    void KV::flush() {
        ::msync(map_, capacity_, MS_SYNC);
    }

    void KV::close() {
        ::close(fd_);
    }

    int KV::check_kv(KV *kv) {
        if (kv == nullptr) {
            return -1;
        }

        if (kv->map_->size() == 0) {
            return 0;
        }

        byte *begin = kv->map_->begin();
        uint32_t crc = crc32(0, begin, kv->map_->size());
        LOGD("prev crc: 0x%x, current crc: 0x%x", kv->map_->crc(), crc);
        return crc != kv->map_->crc();
    }

    KV *KV::create(const char *file) {
        if (gLock == nullptr) {
            LOGD("init module first!");
            return nullptr;
        }

        ScopedLock<nokv::Lock> lock(*gLock);

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
        if (mem == MAP_FAILED) {
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
            ::remove(file);
            return nullptr;
        }
        return kv;
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        munmap(kv->map_, kv->capacity_);
        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const char *const, Entry *)> &fnc) {
        return map_->read_all(fnc);
    }

    bool KV::contains(const char *const key) {
        return map_->contains(key);
    }

    int KV::remove_all() {
        // todo
        return 0;
    }

    int KV::remove(const char *const key) {
        // todo
        return 0;
    }

    int KV::init(const char *meta_file) {
        int fd = open(meta_file, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            return -1;
        }

        gLock = new Lock(fd);
        return 0;
    }

#define DEFINE_PUT(type) \
    int KV::put_##type(const char *const key, const kv_##type##_t &v) { \
        int code = map_->put_##type(key, v); \
        if (code == 0) { \
            return 0; \
        } \
        \
        if (code == ERROR_OVERFLOW) { \
            resize(capacity_ * 2); \
        } \
        \
        return map_->put_##type(key, v); \
    }

    DEFINE_PUT(boolean)

    DEFINE_PUT(int32)

    DEFINE_PUT(int64)

    DEFINE_PUT(float)

    DEFINE_PUT(string)

    DEFINE_PUT(array)

    int KV::put_null(const char *const key) {
        int code = map_->put_null(key);
        if (code == 0) {
            return 0;
        }

        if (code == ERROR_OVERFLOW) {
            resize(capacity_ * 2);
        }

        return map_->put_null(key);
    }

#define DEFINE_GET(type) \
    int KV::get_##type(const char *const key, kv_##type##_t &ret) { \
        return map_->get_##type(key, ret); \
    }

    DEFINE_GET(boolean)

    DEFINE_GET(int32)

    DEFINE_GET(int64)

    DEFINE_GET(float)

    DEFINE_GET(string)

    DEFINE_GET(array)

    void KV::resize(size_t size) {
        // todo
    }

    int KV::put_string(const char *const key, const char *str) {
        nokv::kv_string_t s = {
                .str_ = str
        };
        return put_string(key, s);
    }
}
