//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"
#include <functional>

namespace nkv {
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';

    class KV {
        Lock lock_;
        int fd_;
        Map *map_;
        int capacity_;

        KV(int fd, int capacity, void *mem) : lock_(fd), fd_(fd),
                                              map_(reinterpret_cast<Map *>(mem)),
                                              capacity_(capacity) {
        }

    public:
        void flush();

        void close();

        int write_int32(const char *const key, int32_t v);

        int write_float(const char *const key, float v);

        int write_int64(const char *const key, int64_t v);

        int write_boolean(const char *const key, bool v);

        int write_string(const char *const key, const char *const v);

        int read_int32(const char *const key, int32_t &v);

        int read_float(const char *const key, float &v);

        int read_int64(const char *const key, int64_t &v);

        int read_boolean(const char *const key, bool &v);

        int read_string(const char *const key, char **v);

        size_t size() const { return map_->size_; }

        int read_all(
                const std::function<void(const char *const, const byte *, byte, size_t size)> &fnc);

        bool contains(const char *const key);

        int read(const char *const key, byte **value);
    private:

        int write(const char *const key, byte *value, byte type, size_t size);

        static int check_kv(KV* kv);
    public:
        static KV *create(const char *file);

        static void destroy(KV *kv);
    };

    int init(const char *meta_file);
}

#endif //NKV_CORE_H
