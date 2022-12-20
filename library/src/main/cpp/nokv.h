//
// Created by chan on 2022/11/20.
//

#ifndef NKV_NOKV_H
#define NKV_NOKV_H

#include "kv_lock.h"
#include "kv_map.h"
#include "kv_meta.h"
#include <string>
#include <functional>

namespace nokv {
    class KV {
        Lock *lock_;
        int fd_;
        Map map_;
        byte_t *buf_;
        KVMeta meta_;

        KV(int fd, Lock *lock, const nokv::KVMeta &meta)
                : lock_(lock), fd_(fd),
                  map_(),
                  meta_(meta) {
        }

        ~KV() { delete lock_; }

        static int check_kv(KV *kv);

        int resize(size_t size);

        void init_buf(void *buf, size_t size);

        void bind_buf(void *buf, size_t size);

        int put_string(const char *const, const kv_string_t &);

    public:
        void lock(bool share) {
            lock_->lock(share);
        }

        void unlock(bool share) {
            lock_->unlock(share);
        }

        bool reload_if();

        void flush();

        void close();

        size_t size() const { return map_.size(); }

        int read_all(
                const std::function<void(const kv_string_t &, Entry *)> &fnc);

        int contains(const char *const key);

        static KV *create(const char *name);

        static void destroy(KV *kv);

        static int init(const char *ws);

        int remove_all();

        int remove(const char *);

        int put_boolean(const char *, const kv_boolean_t &);

        int put_int32(const char *, const kv_int32_t &);

        int put_int64(const char *, const kv_int64_t &);

        int put_float(const char *, const kv_float_t &);

        int put_array(const char *, const kv_array_t &);

        int put_null(const char *);

        int get_boolean(const char *, kv_boolean_t &);

        int get_int32(const char *, kv_int32_t &);

        int get_int64(const char *, kv_int64_t &);

        int get_float(const char *, kv_float_t &);

        int get_string(const char *, const char *&);

        int get_array(const char *, kv_array_t &);

        int put_string(const char *, const char *);
    };
}

#endif //NKV_NOKV_H
