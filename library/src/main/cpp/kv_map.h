//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <functional>
#include <cstring>
#include "kv_types.h"
#include "kv_cache.h"

namespace nokv {
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

    class Map {
        struct Header {
            char magic_[4] = {'n', 'o', 'k', 'v'};
            uint16_t order_ = 0x1234;
            uint16_t version_ = 0x0100;
            uint32_t crc_ = 0;
            uint32_t size_ = 0;
        } __attribute__ ((aligned (4)));

        Header header_;
        uint32_t capacity_;
        byte_t *begin_;
        byte_t *buf_;
        MemCache mem_cache_;
    public:
        // 初始化一块内存
        void init(byte_t *buf, uint32_t size) {
            ::memcpy(buf, &header_, sizeof(Header));
            capacity_ = size - sizeof(Header);
            begin_ = buf + sizeof(Header);
            buf_ = buf;
            mem_cache_.clear();
        }

        // 和一块内存绑定
        void bind(byte_t *buf, uint32_t size) {
            ::memcpy(&header_, buf, sizeof(Header));
            capacity_ = size - sizeof(Header);
            begin_ = buf + sizeof(Header);
            buf_ = buf;
            mem_cache_.clear();
            build_mem_cache(this->begin(), this->end());
        }

        byte_t *begin() { return begin_; }

        byte_t *end() { return begin_ + header_.size_; }

        uint32_t size() const { return mem_cache_.size(); }

        uint32_t byte_size() const { return header_.size_; }

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
        int get_value(const kv_string_t &, byte_t *&ret);

        int
        put_value(const kv_string_t &, kv_type_t, const std::function<void(byte_t *)> &,
                  size_t len);

        int read_all(
                const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc);

        int read_all(
                byte_t *begin, byte_t *end,
                const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc);

        void build_mem_cache(byte_t *begin, byte_t *end);

        void invalid_mem_cache(const byte_t *const dirty, int offset);
    };
}

#endif //NKV_IO_H
