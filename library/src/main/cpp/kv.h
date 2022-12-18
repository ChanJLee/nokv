//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <inttypes.h>
#include <functional>
#include <cstring>
#include <unordered_map>

namespace nokv {
    typedef unsigned char byte_t;
    typedef byte_t kv_type_t;
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';
    const int TYPE_ARRAY = 'A';
    const int TYPE_NULL = 'N';

    /* 必须是负值 */
    const int ERROR_OVERFLOW = -1;
    const int ERROR_NOT_FOUND = -2;
    const int ERROR_TYPE_ERROR = -3;
    const int ERROR_INVALID_STATE = -4;
    const int ERROR_MAP_FAILED = -5;
    const int ERROR_INVALID_ARGUMENTS = -6;
    const int VALUE_NULL = 1;

    typedef bool kv_boolean_t;
    typedef float kv_float_t;
    typedef int32_t kv_int32_t;
    typedef int64_t kv_int64_t;

    struct kv_string_t {
        typedef uint32_t kv_string_size_t;

        kv_string_size_t size_;
        const char *str_;

        int to_stream(byte_t *stream) const;

        static int from_stream(byte_t *stream, kv_string_t &str);

        static int from_c_str(const char *s, kv_string_t &str);

        /* with bound check */
        static int from_stream_safe(byte_t *stream, kv_string_t &str, byte_t *end);

        size_t byte_size() const { return size_ + sizeof(size_) + 1; }

        static size_t get_entry_size(byte_t *);
    };

    struct Entry;

    struct kv_array_t {
        typedef uint32_t kv_array_size_t;

        kv_array_size_t capacity_;
        byte_t *end_;
        /* 1u + 4u + elements */
        byte_t *begin_;

        static int from_stream(byte_t *stream, kv_array_t &array);

        int to_stream(byte_t *stream) const;

        static int create(kv_array_t &array);

        static int free(kv_array_t &array);

        int put_string(const char *);

        int put_null();

        size_t byte_count() const {
            return end_ - begin_ + 4;
        }

        class iterator {
            byte_t *begin_;
            byte_t *end_;

            iterator(byte_t *begin, byte_t *end) : begin_(begin), end_(end) {};
        public:
            bool next(Entry *entry);

            friend class kv_array_t;
        };

        iterator it() const { return iterator(begin_, end_); }

        static size_t get_entry_size(byte_t *);

    private:
        int put_string(const kv_string_t &);

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

        const kv_string_t &as_string() { return data_.string_; }

        const kv_array_t &as_array() { return data_.array_; }

        static int from_stream(byte_t *stream, Entry *entry);

        static size_t get_entry_size(byte_t *entry);
    };

    struct Header {
        char magic_[4] = {'n', 'o', 'k', 'v'};
        uint16_t order_ = 0x1234;
        uint16_t version_ = 0x0100;
        uint32_t crc_ = 0;
        uint32_t size_ = 0;
    } __attribute__ ((aligned (4)));

    class Map {
        Header header_;
        uint32_t capacity_;
        byte_t *begin_;
        byte_t *buf_;

        template<class _Tp>
        struct predicate : public std::binary_function<_Tp, _Tp, bool> {
            bool operator()(const _Tp &__x, const _Tp &__y) const {
                return __x == __y || strcmp(__x, __y) == 0;
            }
        };

        struct hash {
            size_t operator()(const char *str) const {
                int seed = 31;
                size_t hash = 0;
                while (*str) {
                    hash = (hash * seed) + (*str);
                    str++;
                }
                return hash;
            }
        };

        std::unordered_map<const char *, byte_t *, hash, predicate<const char *>> lru_;
    public:
        // 初始化一块内存
        void init(byte_t *buf, uint32_t size) {
            ::memcpy(buf, &header_, sizeof(Header));
            capacity_ = size - sizeof(Header);
            begin_ = buf + sizeof(Header);
            buf_ = buf;
            lru_.clear();
        }

        // 和一块内存绑定
        void bind(byte_t *buf, uint32_t size) {
            ::memcpy(&header_, buf, sizeof(Header));
            capacity_ = size - sizeof(Header);
            begin_ = buf + sizeof(Header);
            buf_ = buf;
            lru_.clear();
            build_lru_cache(this->begin(), this->end());
        }

        byte_t *begin() { return begin_; }

        byte_t *end() { return begin_ + header_.size_; }

        uint32_t size() const { return header_.size_; }

        uint32_t capacity() const { return capacity_; }

        uint32_t crc() const { return header_.crc_; }

        int put_boolean(const kv_string_t &, const kv_boolean_t &);

        int put_int32(const kv_string_t &, const kv_int32_t &);

        int put_int64(const kv_string_t &, const kv_int64_t &);

        int put_float(const kv_string_t &, const kv_float_t &);

        int put_string(const kv_string_t &, const kv_string_t &);

        int put_array(const kv_string_t &, const kv_array_t &);

        int put_null(const kv_string_t &);

        int get_boolean(const kv_string_t &, kv_boolean_t &);

        int get_int32(const kv_string_t &, kv_int32_t &);

        int get_int64(const kv_string_t &, kv_int64_t &);

        int get_float(const kv_string_t &, kv_float_t &);

        int get_string(const kv_string_t &, kv_string_t &);

        int get_array(const kv_string_t &, kv_array_t &);

        bool contains(const kv_string_t &key);

        int read_all(
                const std::function<void(const kv_string_t &, Entry *entry)> &fnc);

        int remove(const kv_string_t &key);

        int remove_all();

        void sync();

    private:
        int get_value(const kv_string_t &, byte_t **ret);

        int
        put_value(const kv_string_t &, kv_type_t, const std::function<void(byte_t *)> &,
                  size_t len);

        int read_all(
                const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc);

        int read_all(
                byte_t *begin, byte_t *end,
                const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc);

        void build_lru_cache(byte_t *begin, byte_t *end);
    };
}

#endif //NKV_IO_H
