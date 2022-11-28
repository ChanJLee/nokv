//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <inttypes.h>
#include <functional>

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
    const int ERROR_INVALID_STATE = -4;
    const int VALUE_NULL = 1;

    typedef bool kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    struct kv_string_t {
        const char *str_;
    };

    struct Entry;

    struct kv_array_t {
        size_t capacity_;
        byte *end_;
        /* 1u + 4u + elements */
        byte *begin_;

        static int from_stream(byte *stream, kv_array_t *array);

        static int create(kv_array_t &array);

        static int free(kv_array_t &array);

        int put_string(const kv_string_t &);

        int put_null();

        class iterator {
            byte *begin_;
            byte *end_;
            byte *it_;

            iterator(byte *begin, byte *end) : begin_(begin), end_(end), it_(begin) {};
        public:
            bool next(Entry *entry);

            friend class kv_array_t;
        };

        iterator it() { return iterator(begin_ + 5, end_); }

    private:
        void resize();
    };

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

        friend class Map;
    public:
        Entry() : type_(TYPE_NULL) {}

        bool is_null() const { return type_ == TYPE_NULL; }

        kv_type_t type() const { return type_; }

        kv_boolean_t as_boolean() { return data_.boolean_; }

        kv_float_t as_float() { return data_.float_; }

        kv_int32_t as_int32() { return data_.int32_; }

        kv_int64_t as_int64() { return data_.int64_; }

        const kv_string_t& as_string() { return data_.string_; }

        const kv_array_t &as_array() { return data_.array_; }

        static int from_stream(byte *stream, Entry *entry);

        static int get_entry_size(byte *entry);
    };

    class Map {
        char magic_[4];
        uint16_t order;
        uint16_t version_;
        uint32_t crc_;
        uint32_t size_;
        size_t capacity_;

    public:
        byte *begin() { return (byte *) this + sizeof(Map); }

        byte *end() { return begin() + size_; }

        uint32_t size() const { return size_; }

        uint32_t capacity() const { return capacity_; }

        uint32_t crc() const { return crc_; }

        int put_boolean(const char *const, const kv_boolean_t &);

        int put_int32(const char *const, const kv_int32_t &);

        int put_int64(const char *const, const kv_int64_t &);

        int put_float(const char *const, const kv_float_t &);

        int put_string(const char *const, const kv_string_t &);

        int put_array(const char *const, const kv_array_t &);

        int put_null(const char *const);

        int get_boolean(const char *const, kv_boolean_t &);

        int get_int32(const char *const, kv_int32_t &);

        int get_int64(const char *const, kv_int64_t &);

        int get_float(const char *const, kv_float_t &);

        int get_string(const char *const, kv_string_t &);

        int get_array(const char *const, kv_array_t &);

        bool contains(const char *const key);

        int read_all(
                const std::function<void(const char *const, Entry *entry)> &fnc);

    private:
        int get_value(const char *const, byte **ret);

        int put_value(const char *const, kv_type_t, byte *value, size_t len);

        int put_value(byte *where, const char *, kv_type_t, byte *value, size_t len);

        friend class KV;
    } __attribute__ ((aligned (4)));
}

#endif //NKV_IO_H
