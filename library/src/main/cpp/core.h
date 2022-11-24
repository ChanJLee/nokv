//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"
#include <cstring>
#include <functional>

namespace nkv {
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';

    typedef const char *kv_string_t;
    typedef byte kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    template<typename T>
    struct Entry {

    };

    template<>
    struct Entry<kv_int32_t> {
        using type_t = kv_int32_t;

        const static byte kv_type = TYPE_INT32;

        static size_t size(type_t v) { return 4; }

        static byte *value(type_t &v) { return reinterpret_cast<byte *>(&v); }
    };

    template<>
    struct Entry<kv_float_t> {
        using type_t = kv_float_t;

        const static byte kv_type = TYPE_FLOAT;

        static size_t size(type_t v) { return 4; }

        static byte *value(type_t &v) { return reinterpret_cast<byte *>(&v); }
    };

    template<>
    struct Entry<kv_int64_t> {
        using type_t = kv_int64_t;

        const static byte kv_type = TYPE_INT64;

        static size_t size(type_t v) { return 8; }

        static byte *value(type_t &v) { return reinterpret_cast<byte *>(&v); }
    };

    template<>
    struct Entry<kv_string_t> {
        using type_t = kv_string_t;

        const static byte kv_type = TYPE_STRING;

        static size_t size(kv_string_t v) { return strlen(v) + 1; }

        static byte *value(type_t v) { return (byte *) (v); }
    };

    template<>
    struct Entry<kv_boolean_t> {
        using type_t = kv_boolean_t;

        const static byte kv_type = TYPE_BOOLEAN;

        static size_t size(type_t v) { return 1; }

        static byte *value(type_t &v) { return &v; }
    };

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

        int read_int32(const char *const key, kv_int32_t &v);

        int read_float(const char *const key, float &v);

        int read_int64(const char *const key, kv_int64_t &v);

        int read_boolean(const char *const key, bool &v);

        int read_string(const char *const key, char **v);

        size_t size() const { return map_->size_; }

        int read_all(
                const std::function<void(const char *const, const byte *, byte, size_t size)> &fnc);

        bool contains(const char *const key);

        int read(const char *const key, byte **value);

        Lock &lock() { return lock_; }

    private:

        int write(const char *const key, byte *value, byte type, size_t size);

        static int check_kv(KV *kv);

    public:
        template<class T>
        int write(const char *const key, T v) {
            static_assert(Entry<T>::kv_type,
                          "only support kv_boolean_t/kv_string_t/kv_int32_t/kv_int64_t/kv_float_t");
            return write(key,
                         Entry<T>::value(v),
                         Entry<T>::kv_type,
                         Entry<T>::size(v)
            );
        }

        static KV *create(const char *file);

        static void destroy(KV *kv);
    };

    int init(const char *meta_file);
}

#endif //NKV_CORE_H
