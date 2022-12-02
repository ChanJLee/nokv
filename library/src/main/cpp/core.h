//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"
#include "meta.h"
#include <cstring>
#include <functional>

namespace nokv {
    class KV {
        Lock *lock_;
        int fd_;
        Map map_;
        byte_t *buf_;
        uint32_t seq_;
        KVMeta *meta_;

        KV(int fd, Lock *lock, nokv::KVMeta *meta) : lock_(lock), fd_(fd),
                                                     map_(), seq_(meta->seq()),
                                                     meta_(meta) {
        }

        ~KV() { delete lock_; }

        static int check_kv(KV *kv);

        int resize(size_t size);

        bool reload_if();

        void init_buf(void *buf, size_t size);

        void bind_buf(void *buf, size_t size);

    public:
        void lock() { lock_->lock(); }

        void unlock() { lock_->unlock(); }

        void flush();

        void close();

        size_t size() const { return map_.size(); }

        int read_all(
                const std::function<void(const char *const, Entry *)> &fnc);

        int contains(const char *const key);

        static KV *create(const char *name);

        static void destroy(KV *kv);

        static int init(const char *ws);

        int remove_all();

        int remove(const char *const key);

        int put_boolean(const char *const, const kv_boolean_t &);

        int put_int32(const char *const, const kv_int32_t &);

        int put_int64(const char *const, const kv_int64_t &);

        int put_float(const char *const, const kv_float_t &);

        int put_array(const char *const, const kv_array_t &);

        int put_null(const char *const);

        int get_boolean(const char *const, kv_boolean_t &);

        int get_int32(const char *const, kv_int32_t &);

        int get_int64(const char *const, kv_int64_t &);

        int get_float(const char *const, kv_float_t &);

        int get_string(const char *const, kv_string_t &);

        int get_array(const char *const, kv_array_t &);

        int put_string(const char *const, const char *);

        int put_string(const char *const, const kv_string_t &);
    };
}

#endif //NKV_CORE_H
