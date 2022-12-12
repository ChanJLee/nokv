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
#include <memory>

namespace nokv {
    Lock *gLock;
    std::string gWs;

    void KV::flush() {
        map_.sync();
        ::msync(buf_, map_.capacity(), MS_SYNC);
        meta_.next(map_.capacity());
        // todo remove to trans
        crc_ = map_.crc();
    }

    void KV::close() {
        munmap(buf_, map_.capacity());
        ::close(fd_);
    }

    int KV::check_kv(KV *kv) {
        if (kv->map_.size() == 0) {
            return 0;
        }

        byte_t *begin = kv->map_.begin();
        uint32_t crc = crc32(0, begin, kv->map_.size());
        LOGD("prev crc: 0x%x, current crc: 0x%x", kv->map_.crc(), crc);
        return crc != kv->map_.crc();
    }

    void fill_zero(int fd, uint32_t start, size_t len) {
        if (len == 0) {
            return;
        }

        lseek(fd, start, SEEK_SET);
        constexpr static const size_t _buf_size = 4096;
        byte_t buf[_buf_size] = {0};

        uint64_t actual = (len + _buf_size - 1) & ~(_buf_size - 1); /* avoid overflow */
        uint64_t size = actual / _buf_size;

        uint32_t left = len;
        for (uint64_t i = 0; i < size; ++i) {
            ::write(fd, buf, left < _buf_size ? left : _buf_size);
            left -= _buf_size;
        }
        lseek(fd, 0, SEEK_SET);
        fsync(fd);
    }

    KV *KV::create(const char *name) {
        if (gLock == nullptr) {
            LOGD("init module first!");
            return nullptr;
        }

        ScopedLock<nokv::Lock, false> create_lock(*gLock);

        std::stringstream ss;
        ss << gWs << "/";
        if (name[0] == '.') {
            ss << '_';
        }
        ss << name << ".nokv";
        const std::string path = ss.str();
        const char *file = path.c_str();

        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return nullptr;
        }

        std::unique_ptr<Lock> file_lock(new Lock(fd));
        ScopedLock<nokv::Lock, false> lock(*file_lock.get());
        struct stat st{};
        if (stat(file, &st) != 0) {
            return nullptr;
        }

        bool new_file = st.st_size == 0;
        if (new_file) {
            size_t size = getpagesize();
            st.st_size = size;
            // avoid BUS error
            fill_zero(fd, 0, size);
        }

        void *mem = mmap(nullptr, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("mmap %s failed", file);
            return nullptr;
        }

        KVMeta meta = {};
        meta.update(fd, st);
        KV *kv = new KV(fd, name, file_lock.release(), meta);
        if (new_file) {
            // todo support unit test only once
            LOGD("init buf");
            kv->init_buf(mem, st.st_size);
            return kv;
        }

        LOGD("bind buf");
        kv->bind_buf(mem, st.st_size);
        if (check_kv(kv)) {
            LOGD("check kv failed: %s", name);
            kv->close();
            lock.~ScopedLock<nokv::Lock, false>();
            delete kv;
            ::remove(file);
            // todo remove meta
            return nullptr;
        }

        return kv;
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        kv->flush();
        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const kv_string_t &, Entry *)> &fnc) {
        return map_.read_all(fnc);
    }

    int KV::contains(const char *const key) {
        kv_string_t kv_key{
                .size_ = (uint32_t) strlen(key),
                .str_ = key
        };
        return map_.contains(kv_key) ? 0 : ERROR_NOT_FOUND;
    }

    int KV::remove_all() {
        int code = map_.remove_all();
        if (code != 0) {
            return code;
        }
        /* ignore rtn code */
        // resize(getpagesize());
        return 0;
    }

    int KV::remove(const char *const key) {
        kv_string_t kv_key{
                .size_ = (uint32_t) strlen(key),
                .str_ = key
        };
        return map_.remove(kv_key);
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
        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return -1;
        }

        gLock = new Lock(fd);
        return 0;
    }

#define DEFINE_PUT(type) \
    int KV::put_##type(const char *const key, const kv_##type##_t &v) { \
        kv_string_t kv_key{ \
            .size_ =(uint32_t) strlen(key), \
            .str_ = key \
        };  \
        \
        int code = map_.put_##type(kv_key, v); \
        if (code == 0) { \
            return 0; \
        } \
        \
        if (code == ERROR_OVERFLOW) { \
            if (resize(map_.capacity() * 2)) {  \
                return ERROR_OVERFLOW; \
            } \
        } \
        \
        return map_.put_##type(kv_key, v); \
    }

    DEFINE_PUT(boolean)

    DEFINE_PUT(int32)

    DEFINE_PUT(int64)

    DEFINE_PUT(float)

    DEFINE_PUT(string)

    DEFINE_PUT(array)

    int KV::put_null(const char *const key) {
        kv_string_t kv_key{
                .size_ = (uint32_t) strlen(key),
                .str_ = key
        };
        int code = map_.put_null(kv_key);
        if (code == 0) {
            return 0;
        }

        if (code == ERROR_OVERFLOW) {
            if (resize(map_.capacity() * 2)) {
                return ERROR_OVERFLOW;
            }
        }

        return map_.put_null(kv_key);
    }

#define DEFINE_GET(type) \
    int KV::get_##type(const char * key, kv_##type##_t &ret) { \
        kv_string_t kv_key { \
            .size_ =(uint32_t)strlen(key), \
            .str_ = key \
        };  \
        return map_.get_##type(kv_key, ret); \
    }

    DEFINE_GET(boolean)

    DEFINE_GET(int32)

    DEFINE_GET(int64)

    DEFINE_GET(float)

    DEFINE_GET(array)

    int KV::resize(size_t size) {
        struct stat st = {};
        if (fstat(fd_, &st)) {
            LOGD("resize get file state failed");
            return ERROR_INVALID_STATE;
        }

        if (st.st_size < size) {
            fill_zero(fd_, st.st_size, size - st.st_size);
        } else if (st.st_size > size) {
            // TODO return direct
            ftruncate(fd_, size);
        } else {
            LOGD("do noting, size is equal to %d", size);
        }

        if (fstat(fd_, &st) || st.st_size != size) {
            LOGD("resize check file state failed");
            return ERROR_INVALID_STATE;
        }

        munmap(buf_, map_.capacity());
        void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("resize mmap failed");
            return ERROR_MAP_FAILED;
        }

        bind_buf(mem, st.st_size);
        LOGD("resize to %d", size);
        return 0;
    }

    int KV::put_string(const char *const key, const char *str) {
        nokv::kv_string_t val = {
                .size_ = (uint32_t) strlen(str),
                .str_ = str
        };
        return put_string(key, val);
    }

    void KV::init_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.init(buf_, size);
        crc_ = map_.crc();
    }

    void KV::bind_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.bind(buf_, size);
        crc_ = map_.crc();
    }

    bool KV::reload_if() {
        KVMeta meta = KVMeta::seq(fd_);
        if (meta == meta_) {
            return true;
        }

        if (crc_ == map_.crc()) {
            meta_ = meta;
            return true;
        }

        struct stat st = {0};
        if (fstat(fd_, &st)) {
            return false;
        }

        // todo check if mem leak
        void *mem = mmap(buf_, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            return false;
        }

        LOGD("reload");
        bind_buf(mem, st.st_size);
        meta_.update(fd_, st);
        return true;
    }

    int KV::get_string(const char *key, const char *&str) {
        kv_string_t kv_key{
                .size_ = (uint32_t) strlen(key),
                .str_ = key
        };
        kv_string_t ret = {};
        int code = map_.get_string(kv_key, ret);
        if (code != 0) {
            return code;
        }

        str = ret.str_;
        return 0;
    }
}
