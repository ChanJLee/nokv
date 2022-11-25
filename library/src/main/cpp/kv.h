//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <inttypes.h>

namespace nokv {
    typedef unsigned char byte;
    typedef byte kv_type_t;
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';
    const int TYPE_ARRAY = 'A';
    const int TYPE_NULL = 'N';

    struct kv_null_t {
    };

    struct kv_array_t {

    };

    typedef const char *kv_string_t;
    typedef bool kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    struct Map {
        char magic_[4];
        uint16_t order;
        uint16_t version_;
        uint32_t crc_;
        uint32_t size_;
    } __attribute__ ((aligned (4)));

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

    template<>
    class Entry<kv_null_t> {
    public:
        using type_t = kv_null_t;
    private:
    public:
        Entry(type_t) {}

        const static kv_type_t kv_type = TYPE_NULL;

        static size_t size() { return 0; }

        byte *value() { return nullptr; }

        static bool compat(kv_type_t type) { return false; }
    };
}

#endif //NKV_IO_H
