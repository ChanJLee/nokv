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

    const int ERROR_OVERFLOW = -1;
    const int ERROR_NOT_FOUND = -2;
    const int ERROR_TYPE_ERROR = -3;
    const int VALUE_NULL = 1;

    typedef bool kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    struct kv_string_t {
        size_t size_;
        const char *str_;

        static int from_stream(byte *stream, kv_string_t *str);
    };

    struct kv_array_t {
        size_t capacity_;
        byte *end_;
        /* 1u + 4u + elements */
        byte *begin_;
        const bool free_;

        kv_array_t(size_t len) : free_(true) {
            begin_ = new byte[len];
            capacity_ = len;
            begin_[0] = TYPE_ARRAY;
            end_ = begin_ + 5;
        }

        ~kv_array_t() {
            if (free_) {
                delete[] begin_;
            }
        }

        kv_array_t() : free_(false) {}

        // todo put 的时候 begin 内的报文可以直接写

        static int from_stream(byte *stream, kv_array_t *array);
    };

    class Map {
        char magic_[4];
        uint16_t order;
        uint16_t version_;
        uint32_t crc_;
        uint32_t size_;

    public:
        byte *begin() { return (byte *) this + sizeof(Map); }

        byte *end() { return begin() + size_; }

        uint32_t size() const { return size_; }

        int put_boolean(const char *const, const kv_boolean_t &);

        int put_int32(const char *const, const kv_int32_t &);

        int put_int64(const char *const, const kv_int64_t &);

        int put_float(const char *const, const kv_float_t &);

        int put_string(const char *const, const kv_string_t &);

        int put_array(const char *const, const kv_array_t &);

        int put_null(const char *const);

    private:
        int get_value(const char *const, byte **entry);

        int put_value(const char *const, kv_type_t, byte *value, size_t len);
    } __attribute__ ((aligned (4)));
}

#endif //NKV_IO_H
