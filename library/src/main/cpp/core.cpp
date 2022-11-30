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
#include <map>
#include <sstream>

namespace nokv {

    Lock *gLock;
    std::string gWs;
    std::map<const char *, KVMeta *> gMetaTable;

    KVMeta *get_meta(const char *name) {
        const auto &it = gMetaTable.find(name);
        if (it != gMetaTable.end()) {
            return it->second;
        }

        std::stringstream ss;
        ss << gWs << "/meta/" << name << ".meta";
        const std::string path = ss.str();
        const char *file = path.c_str();
        KVMeta *meta = KVMeta::create(file);
        gMetaTable[name] = meta;
        return meta;
    }

    void KV::flush() {
        ::msync(buf_, map_.capacity(), MS_SYNC);
    }

    void KV::close() {
        ::close(fd_);
    }

    int KV::check_kv(KV *kv) {
        if (kv == nullptr) {
            return -1;
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
        ss << gWs << "/kv/" << name << ".kv";
        const std::string path = ss.str();
        const char *file = path.c_str();

        struct stat st{};
        bool new_file = stat(file, &st) != 0;
        int fd = open(file, O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("open %s failed", file);
            return NULL;
        }

        KVMeta *meta = get_meta(name);
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
        if (kv->bind_buf(mem, st.st_size)) {
            kv->close();
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

        munmap(kv->buf_, kv->map_.capacity());
        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const char *const, Entry *)> &fnc) {
        return map_.read_all(fnc);
    }

    bool KV::contains(const char *const key) {
        return map_.contains(key);
    }

    int KV::remove_all() {
        int code = map_.remove_all();
        if (code != 0) {
            return code;
        }
        resize(getpagesize());
        return 0;
    }

    int KV::remove(const char *const key) {
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
        path += "kv.lock";
        int fd = open(path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd < 0) {
            LOGI("create global lock failed");
            return -2;
        }

        gLock = new Lock(fd);
        return 0;
    }

#define DEFINE_PUT(type) \
    int KV::put_##type(const char *const key, const kv_##type##_t &v) { \
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
        return map_.get_##type(key, ret); \
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

    void KV::init_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.init(buf_, size);
    }

    int KV::bind_buf(void *buf, size_t size) {
        buf_ = static_cast<byte_t *>(buf);
        map_.bind(buf_, size);
        return check_kv(this);
    }
}
