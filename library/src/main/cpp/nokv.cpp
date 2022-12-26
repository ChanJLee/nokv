//
// Created by chan on 2022/11/20.
//

#include "kv_lock.h"
#include "nokv.h"
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "kv_log.h"
#include <zlib.h>
#include <cstddef>
#include "kv_meta.h"
#include <sstream>
#include <memory>

namespace nokv {
    Lock *gLock;
    std::string gWs;

    void KV::flush() {
        map_.sync();
        ::msync(buf_, map_.capacity(), MS_SYNC);
        meta_ = nokv::KVMeta::next_seq(fd_);
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
        uint32_t crc = crc32(0, begin, kv->map_.byte_size());
        LOGD("prev crc: 0x%x, current crc: 0x%x", kv->map_.crc(), crc);
        return crc != kv->map_.crc();
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
            ftruncate(fd, st.st_size);
        }

        void *mem = mmap(nullptr, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("mmap %s failed", file);
            return nullptr;
        }

        KVMeta meta = {};
        meta.update(fd, st);
        KV *kv = new KV(fd, file_lock.release(), meta);
        if (new_file) {
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
            return nullptr;
        }

        return kv;
    }

    void KV::destroy(KV *kv) {
        if (kv == nullptr) {
            return;
        }

        kv->close();
        delete kv;
    }

    int
    KV::read_all(const std::function<void(const kv_string_t &, Entry *)> &fnc) {
        return map_.read_all(fnc);
    }

    int KV::contains(const char *const key) {
        kv_string_t kv_key{
        };
        kv_string_t::from_c_str(key, kv_key);
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
        };
        kv_string_t::from_c_str(key, kv_key);
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
        };  \
        kv_string_t::from_c_str(key, kv_key);\
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
        };
        kv_string_t::from_c_str(key, kv_key);
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
        }; \
        kv_string_t::from_c_str(key, kv_key); \
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

        if (st.st_size != size) {
            ftruncate(fd_, size);
        } else {
            LOGD("do noting, size is equal to %d", size);
        }

        if (fstat(fd_, &st) || st.st_size != size) {
            LOGD("resize check file state failed");
            return ERROR_INVALID_STATE;
        }

        void *mem = mmap(buf_, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mem == MAP_FAILED || mem == nullptr) {
            LOGI("resize mmap failed");
            return ERROR_MAP_FAILED;
        }

        bind_buf(mem, st.st_size);
#ifdef NKV_UNIT_TEST
        LOGD("resize to %d, process %d did it", size, getpid());
#endif
        return 0;
    }

    int KV::put_string(const char *const key, const char *str) {
        nokv::kv_string_t val = {
        };
        kv_string_t::from_c_str(str, val);
        return put_string(key, val);
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
        KVMeta meta = KVMeta::get_seq(fd_);
        if (meta == meta_) {
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

#ifdef NKV_UNIT_TEST
            LOGD("process %d reload, size %d", getpid(), st.st_size);
#else
        LOGD("reload");
#endif
        bind_buf(mem, st.st_size);
        meta_.update(fd_, st);
        return true;
    }

    int KV::get_string(const char *key, const char *&str) {
        kv_string_t kv_key{
        };
        kv_string_t::from_c_str(key, kv_key);
        kv_string_t ret = {};
        int code = map_.get_string(kv_key, ret);
        if (code != 0) {
            return code;
        }

        str = ret.str_;
        return 0;
    }
}
