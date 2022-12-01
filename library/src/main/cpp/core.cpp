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
#include "meta.h"
#include <sstream>

namespace nokv {
    Lock *gLock;
    std::string gWs;

    void KV::flush() {
        ::msync(buf_, map_.capacity(), MS_SYNC);
    }

    void KV::close() {
        munmap(buf_, map_.capacity());
        ::close(fd_);
    }

    int KV::check_kv(KV *kv) {
        ScopedLock<KV> lock(*kv);
        if (!kv->reload_if()) {
            return ERROR_MAP_FAILED;
        }

        if (kv->map_.size() == 0) {
            return 0;
        }

        byte_t *begin = kv->map_.begin();
        uint32_t crc = crc32(0, begin, kv->map_.size());
        LOGD("prev crc: 0x%x, current crc: 0x%x", kv->map_.crc(), crc);
        return crc != kv->map_.crc();
    }

    void fill_zero(int fd, size_t len) {
        if (len == 0) {
            return;
        }

        constexpr static const size_t _buf_size = 4096;
        byte_t buf[_buf_size] = {0};

        uint64_t actual = (len + _buf_size - 1) & ~(_buf_size - 1); /* avoid overflow */
        uint64_t size = actual / _buf_size;

        uint32_t left = len;
        for (uint64_t i = 0; i < size; ++i) {
            ::write(fd, buf, left < _buf_size ? left : _buf_size);
            left -= _buf_size;
        }
    }

    KV *KV::create(const char *name) {
        if (gLock == nullptr) {
            LOGD("init module first!");
            return nullptr;
        }

        ScopedLock<nokv::Lock> lock(*gLock);

        std::stringstream ss;
        ss << gWs << "/";
        if (name[0] == '.') {
            ss << '_';
        }
        ss << name << ".nokv";
        const std::string path = ss.str();
        const char *file = path.c_str();

        struct stat st{};
        bool new_file = stat(file, &st) != 0;
        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return NULL;
        }

        KVMeta *meta = KVMeta::get(name);
        if (meta == nullptr) {
            LOGI("open %s's meta file failed", file);
            return nullptr;
        }

        if (new_file) {
            size_t size = getpagesize();
            st.st_size = size;
            // avoid BUS error
            fill_zero(fd, size);
            fsync(fd);
        }

        void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("mmap %s failed", file);
            return nullptr;
        }

        KV *kv = new KV(fd, meta);
        if (new_file) {
            kv->init_buf(mem, st.st_size);
            return kv;
        }

        LOGD("bind buf");
        kv->bind_buf(mem, st.st_size);
        if (check_kv(kv)) {
            kv->close();
            delete kv;
            // todo
//            ::remove(file);
            // todo remove meta
            return nullptr;
        }

        return kv;
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        munmap(kv->buf_, kv->map_.capacity());
        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const char *const, Entry *)> &fnc) {
        if (!reload_if()) {
            return ERROR_MAP_FAILED;
        }

        return map_.read_all(fnc);
    }

    int KV::contains(const char *const key) {
        if (!reload_if()) {
            return ERROR_MAP_FAILED;
        }

        return map_.contains(key) ? 0 : ERROR_NOT_FOUND;
    }

    int KV::remove_all() {
        if (!reload_if()) {
            return ERROR_MAP_FAILED;
        }

        int code = map_.remove_all();
        if (code != 0) {
            return code;
        }
        resize(getpagesize());
        return 0;
    }

    int KV::remove(const char *const key) {
        if (!reload_if()) {
            return ERROR_MAP_FAILED;
        }

        return map_.remove(key);
    }

    int KV::init(const char *ws) {
        struct stat st = {0};
        if (stat(ws, &st) != 0 && mkdir(ws, 0700) != 0) {
            LOGD("create ws failed");
            return -1;
        }

        gWs = ws;
        std::string path = ws;
        path += "/.kv.lock";

        const char *file = path.c_str();
        bool new_file = stat(file, &st) != 0;
        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return -1;
        }

        if (new_file) {
            size_t size = getpagesize() * 16;
            st.st_size = size;
            // avoid BUS error
            fill_zero(fd, size);
            fsync(fd);
        }

        void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("mmap %s failed", file);
            return -1;
        }

        KVMeta::init(mem, st.st_size);
        gLock = new Lock(fd);
        return 0;
    }

#define DEFINE_PUT(type) \
    int KV::put_##type(const char *const key, const kv_##type##_t &v) { \
        if (!reload_if()) {  \
            return ERROR_MAP_FAILED; \
        } \
        \
        int code = map_.put_##type(key, v); \
        if (code == 0) { \
            return 0; \
        } \
        \
        if (code == ERROR_OVERFLOW) { \
            resize(map_.capacity() * 2); \
        } \
        \
        return map_.put_##type(key, v); \
    }

    DEFINE_PUT(boolean)

    DEFINE_PUT(int32)

    DEFINE_PUT(int64)

    DEFINE_PUT(float)

    DEFINE_PUT(string)

    DEFINE_PUT(array)

    int KV::put_null(const char *const key) {
        if (!reload_if()) {
            return ERROR_MAP_FAILED;
        }

        int code = map_.put_null(key);
        if (code == 0) {
            return 0;
        }

        if (code == ERROR_OVERFLOW) {
            resize(map_.capacity() * 2);
        }

        return map_.put_null(key);
    }

#define DEFINE_GET(type) \
    int KV::get_##type(const char *const key, kv_##type##_t &ret) { \
        if (!reload_if()) {                                         \
            return ERROR_MAP_FAILED;             \
        } \
        return map_.get_##type(key, ret); \
    }

    DEFINE_GET(boolean)

    DEFINE_GET(int32)

    DEFINE_GET(int64)

    DEFINE_GET(float)

    DEFINE_GET(string)

    DEFINE_GET(array)

    int KV::resize(size_t size) {
        // todo
        return 0;
    }

    int KV::put_string(const char *const key, const char *str) {
        nokv::kv_string_t s = {
                .str_ = str
        };
        return put_string(key, s);
    }

    void KV::init_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.init(buf_, size);
    }

    void KV::bind_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.bind(buf_, size);
    }

    bool KV::reload_if() {
        uint32_t seq = meta_->seq();
        if (seq == seq_) {
            return true;
        }

        struct stat st = {0};
        if (fstat(fd_, &st)) {
            return false;
        }

        void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            return false;
        }

        munmap(buf_, map_.capacity());
        buf_ = static_cast<byte_t *>(mem);
        map_.bind(buf_, st.st_size);
        seq_ = seq;
        return true;
    }
}
