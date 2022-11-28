//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"
#include <cstring>
#include <functional>

namespace nokv {
    class KV {
        Lock lock_;
        int fd_;
        Map *map_;

        KV(int fd, size_t capacity, void *mem) : lock_(fd), fd_(fd),
                                map_(reinterpret_cast<Map *>(mem)) {
            map_->capacity_ = capacity;
        }

        static int check_kv(KV *kv);

        void resize(size_t size);

    public:
        void lock() { lock_.lock(); }

        void unlock() { lock_.unlock(); }

        void flush();

        void close();

        size_t size() const { return map_->size(); }

        int read_all(
                const std::function<void(const char *const, Entry *)> &fnc);

        bool contains(const char *const key);

        static KV *create(const char *file);

        static void destroy(KV *kv);

        static int init(const char *meta_file);

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
