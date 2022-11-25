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

    struct kv_array_t {
        size_t byte_size_;
        size_t elem_size_;
        byte *data_;

        /* 字节长度 */
        size_t byte_size() { return byte_size_; }

        /* 元素长度 */
        size_t elem_size() { return elem_size_; }

        static int from_stream(byte *stream, kv_array_t *array);

        static int to_stream(byte *stream, kv_array_t *array);
    };

    struct kv_string_t {
        size_t size_;
        const char *str_;

        static int from_stream(byte *stream, kv_string_t *str);

        static int to_stream(byte *stream, kv_string_t *array);
    };

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

    class Entry {
        kv_type_t type_;
        union {
            kv_boolean_t boolean_;
            kv_float_t float_;
            kv_int32_t int32_;
            kv_int64_t int64_;
            kv_string_t string_;
            kv_array_t array_;
        } data_;
    public:
        Entry() : type_(TYPE_NULL) {}

        bool is_null() const { return type_ == TYPE_NULL; }

        kv_type_t type() const { return type_; }

        static int from_stream(byte *stream, Entry *entry);

        static int get_entry_size(byte *entry);
    };
}

#endif //NKV_IO_H
