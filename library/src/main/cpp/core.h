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
    typedef byte kv_type_t;
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';

    typedef const char *kv_string_t;
    typedef bool kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    template<typename T>
    class Entry {

    };

    template<>
    class Entry<kv_int32_t> {
    public:
        using type_t = kv_int32_t;
    private:
        type_t v_;
    public:
        Entry(type_t v) : v_(v) {}

        const static kv_type_t kv_type = TYPE_INT32;

        static size_t size() { return 4; }

        byte *value() { return reinterpret_cast<byte *>(&v_); }

        static bool compat(kv_type_t type) { return type == TYPE_INT32 || type == TYPE_BOOLEAN; }
    };

    template<>
    class Entry<kv_float_t> {
    public:
        using type_t = kv_float_t;
    private:
        type_t v_;
    public:
        Entry(type_t v) : v_(v) {}

        const static kv_type_t kv_type = TYPE_FLOAT;

        static size_t size() { return 4; }

        byte *value() { return reinterpret_cast<byte *>(&v_); }

        static bool compat(kv_type_t type) { return type == TYPE_FLOAT; }
    };

    template<>
    class Entry<kv_int64_t> {
    public:
        using type_t = kv_int64_t;
    private:
        type_t v_;
    public:
        Entry(type_t v) : v_(v) {}

        const static kv_type_t kv_type = TYPE_INT64;

        static size_t size() { return 8; }

        byte *value() { return reinterpret_cast<byte *>(&v_); }

        static bool compat(kv_type_t type) {
            return type == TYPE_INT32 || type == TYPE_BOOLEAN || type == TYPE_INT64;
        }
    };

    template<>
    class Entry<kv_string_t> {
    public:
        using type_t = kv_string_t;
    private:
        type_t v_;
    public:
        Entry(type_t v) : v_(v) {}

        const static kv_type_t kv_type = TYPE_STRING;

        size_t size() { return strlen(v_) + 1; }

        byte *value() { return (byte *) (v_); }

        static bool compat(kv_type_t type) { return type == TYPE_STRING; }
    };

    template<>
    class Entry<kv_boolean_t> {
    public:
        using type_t = kv_boolean_t;
    private:
        type_t v_;
    public:
        Entry(type_t v) : v_(v) {}

        const static kv_type_t kv_type = TYPE_BOOLEAN;

        static size_t size() { return 1; }

        byte *value() { return reinterpret_cast<byte *>(&v_); }

        static bool compat(kv_type_t type) { return type == TYPE_BOOLEAN; }
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

        int read(const char *const key, byte **value);

        int write(const char *const key, byte *value, byte type, size_t size);

        static int check_kv(KV *kv);

        template<class T, class O>
        int cast_stream(byte *ptr, O &ret) {
            /* type compat */
            if (!Entry<T>::compat(ptr[0])) {
                return -1;
            }

            T tmp;
            memcpy(&tmp, ptr + 1, Entry<T>::size());
            ret = (T) tmp;
            return 0;
        }

    public:
        template<class T>
        int read(const char *const key, T &ret) {
            static_assert(Entry<T>::kv_type,
                          "only support kv_boolean_t/kv_string_t/kv_int32_t/kv_int64_t/kv_float_t");
            byte *ptr = nullptr;
            if (read(key, &ptr)) {
                return -1;
            }

            switch (ptr[0]) {
                case TYPE_INT32:
                    return cast_stream<kv_int32_t, T>(ptr, ret);
                case TYPE_FLOAT:
                    return cast_stream<kv_float_t, T>(ptr, ret);
                case TYPE_BOOLEAN:
                    return cast_stream<kv_boolean_t, T>(ptr, ret);
                case TYPE_INT64:
                    return cast_stream<kv_int64_t, T>(ptr, ret);
            }

            return -1;
        }

        template<class T = kv_string_t>
        int read(const char *const key, kv_string_t &ret) {
            byte *ptr = nullptr;
            if (read(key, &ptr)) {
                return -1;
            }

            if (ptr[0] == TYPE_STRING) {
                ret = (char *) ptr + 1;
                return 0;
            }

            return -1;
        }

        template<class T>
        int write(const char *const key, T v) {
            static_assert(Entry<T>::kv_type,
                          "only support kv_boolean_t/kv_string_t/kv_int32_t/kv_int64_t/kv_float_t");
            Entry<T> entry(v);
            return write(key,
                         entry.value(),
                         Entry<T>::kv_type,
                         entry.size()
            );
        }

        void lock() { lock_.lock(); }

        void unlock() { lock_.unlock(); }

        void flush();

        void close();

        size_t size() const { return map_->size_; }

        int read_all(
                const std::function<void(const char *const, const byte *, byte, size_t size)> &fnc);

        bool contains(const char *const key);

        static KV *create(const char *file);

        static void destroy(KV *kv);
    };

    int init(const char *meta_file);
}

#endif //NKV_CORE_H
