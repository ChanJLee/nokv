//
// Created by chan on 2022/11/25.
//

#include "kv.h"
#include <cstring>
#include <zlib.h>

namespace nokv {

    class Entry {
        kv_type_t type_;
        union data {
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

        data &data() { return data_; }

        static int from_stream(byte *stream, Entry *entry);

        static int get_entry_size(byte *entry);
    };

    int nokv::Entry::get_entry_size(nokv::byte *entry) {
        switch (entry[0]) {
            case nokv::TYPE_INT32:
            case nokv::TYPE_FLOAT:
                return 4;
            case nokv::TYPE_BOOLEAN:
                return 1;
            case nokv::TYPE_INT64:
                return 8;
            case nokv::TYPE_STRING:
                return strlen((char *) entry + 1) + 1;
            case nokv::TYPE_NULL:
                return 0;
            case nokv::TYPE_ARRAY: {
                int32_t size = 0;
                memcpy(&size, entry + 1, sizeof(size));
                return size;
            }
        }

        return -1;
    }

    int nokv::Entry::from_stream(nokv::byte *stream, nokv::Entry *entry) {
        entry->type_ = stream[0];
        if (entry->type_ == TYPE_NULL) {
            return 0;
        }

        if (entry->type_ == TYPE_STRING) {
            entry->data_.string_.str_ = (const char *) (stream + 1);
            return 0;
        }

        if (entry->type_ == TYPE_ARRAY) {
            return nokv::kv_array_t::from_stream(stream, &entry->data_.array_);
        }

        int size = get_entry_size(stream);
        if (size < 0) {
            return ERROR_INVALID_STATE;
        }

        memcpy(&entry->data_, stream + 1, size);
        return 0;
    }

    int nokv::kv_array_t::from_stream(nokv::byte *stream, nokv::kv_array_t *array) {
        array->begin_ = stream + 1;
        memcpy(&array->capacity_, array->begin_, 4);
        array->end_ = array->begin_ + array->capacity_;
        return 0;
    }

    int kv_array_t::create(kv_array_t &array) {
        array.capacity_ = 1024;
        array.begin_ = new byte[array.capacity_];
        array.begin_[0] = TYPE_ARRAY;
        array.end_ = array.begin_ + 5 /* 1u(type) + 4u(len) */;
        return 0;
    }

    int kv_array_t::free(kv_array_t &array) {
        delete[] array.begin_;
        return 0;
    }

    int kv_array_t::put_string(const kv_string_t &str) {
        size_t str_len = strlen(str.str_) + 1;
        if (end_ + str_len > begin_ + capacity_) {
            resize();
        }
        end_[0] = TYPE_STRING;
        memcpy(end_ + 1, str.str_, str_len);
        end_ = begin_ + str_len + 1;
        return 0;
    }

    int kv_array_t::put_null() {
        if (end_ + 1 > begin_ + capacity_) {
            resize();
        }
        end_[0] = TYPE_NULL;
        end_ = begin_ + 1;
        return 0;
    }

    void kv_array_t::resize() {
        size_t new_size = capacity_ * 2;
        byte *buf = new byte[new_size];

        size_t size = end_ - begin_;
        memcpy(buf, begin_, size);

        delete[] begin_;

        begin_ = buf;
        end_ = begin_ + size;
        capacity_ = new_size;
    }

    int nokv::Map::put_array(const char *const key, const nokv::kv_array_t &array) {
        return put_value(key, TYPE_ARRAY, array.begin_, array.end_ - array.begin_);
    }

    int Map::put_string(const char *const key, const kv_string_t &str) {
        return put_value(key, TYPE_STRING, (byte *) str.str_, strlen(str.str_) + 1);
    }

    int Map::put_boolean(const char *const key, const kv_boolean_t &v) {
        return put_value(key, TYPE_BOOLEAN, (byte *) &v, 1);
    }

    int Map::put_null(const char *const key) {
        return put_value(key, TYPE_NULL, nullptr, 0);
    }

    int Map::put_int32(const char *const key, const kv_int32_t &v) {
        byte buf[4] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_INT32, buf, sizeof(buf));
    }

    int Map::put_int64(const char *const key, const kv_int64_t &v) {
        byte buf[4] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_INT64, buf, sizeof(buf));
    }

    int Map::put_float(const char *const key, const kv_float_t &v) {
        byte buf[4] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_FLOAT, buf, sizeof(buf));
    }

    int Map::put_value(const char *const key, kv_type_t type, byte *value, size_t len) {
        byte *begin = this->begin();
        byte *end = this->end();

        byte *write_ptr = nullptr;
        get_value(key, &write_ptr);

        // 新值
        if (write_ptr == nullptr) {
            int code = put_value(end, key, type, value, len);
            if (code != 0) {
                return code;
            }
            size_ = size_ + len + 1 /* type */ + strlen(key) + 1;
            crc_ = crc32(0, begin, size_);
            return 0;
        }

        // 旧值
        int prev_size = Entry::get_entry_size(write_ptr);
        if (prev_size < 0) {
            /* invalid state */
            return ERROR_INVALID_STATE;
        }

        if (prev_size == len + 1) {
            int code = put_value(write_ptr - strlen(key) - 1, key, type, value, len);
            if (code != 0) {
                return code;
            }
            crc_ = crc32(0, begin, size_);
            return 0;
        }

        // 长度发生了变化就要重排
        size_t key_len = strlen(key) + 1;
        size_t offset_size = end - write_ptr - prev_size;
        memcpy(write_ptr - key_len, write_ptr + prev_size, offset_size);
        write_ptr = write_ptr - key_len + offset_size;

        int code = put_value(write_ptr, key, type, value, len);
        if (code != 0) {
            return code;
        }
        size_ = size_ - prev_size + len + 1;
        crc_ = crc32(0, begin, size_);
        return 0;
    }

    int Map::get_value(const char *const key, byte **ret) {
        byte *begin = this->begin();
        byte *end = this->end();

        while (begin < end) {
            char *entry_key = reinterpret_cast<char *>(begin);
            size_t key_len = strlen(entry_key);
            byte *data = begin + key_len + 1;
            if (strcmp(entry_key, key) == 0) {
                *ret = data;
                return 0;
            }

            int entry_size = Entry::get_entry_size(data);
            if (entry_size < 0) {
                /* invalid state */
                return ERROR_INVALID_STATE;
            }

            begin = data + entry_size;
        }

        /* not found */
        return ERROR_NOT_FOUND;
    }

    int Map::put_value(byte *where, const char *key, kv_type_t type, byte *value, size_t len) {
        size_t key_len = strlen(key);
        byte *begin = this->begin();
        byte *end = this->end();
        if (begin + key_len + 1 + 1 + len >= end) {
            return ERROR_OVERFLOW;
        }

        memcpy(where, key, key_len + 1);
        where = where + key_len + 1;
        where[0] = type;
        memcpy(where + 1, value, len);
        return 0;
    }

    int Map::get_boolean(const char *const key, kv_boolean_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() != TYPE_BOOLEAN) {
            return ERROR_TYPE_ERROR;
        }

        rtn = entry.data().boolean_;
        return 0;
    }

    int Map::get_int32(const char *const key, kv_int32_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() == TYPE_ARRAY ||
            entry.type() == TYPE_FLOAT ||
            entry.type() == TYPE_INT64 ||
            entry.type() == TYPE_STRING) {
            return ERROR_TYPE_ERROR;
        }

        if (entry.type() == TYPE_BOOLEAN) {
            rtn = entry.data().boolean_;
        } else {
            rtn = entry.data().int32_;
        }
        return 0;
    }

    int Map::get_int64(const char *const key, kv_int64_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() == TYPE_ARRAY ||
            entry.type() == TYPE_FLOAT ||
            entry.type() == TYPE_STRING) {
            return ERROR_TYPE_ERROR;
        }

        if (entry.type() == TYPE_BOOLEAN) {
            rtn = entry.data().boolean_;
        } else if (entry.type() == TYPE_INT32) {
            rtn = entry.data().int32_;
        } else {
            rtn = entry.data().int64_;
        }
        return 0;
    }

    int Map::get_float(const char *const key, kv_float_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() == TYPE_ARRAY ||
            entry.type() == TYPE_INT64 ||
            entry.type() == TYPE_STRING) {
            return ERROR_TYPE_ERROR;
        }

        if (entry.type() == TYPE_BOOLEAN) {
            rtn = entry.data().boolean_;
        } else if (entry.type() == TYPE_INT32) {
            rtn = (kv_float_t) entry.data().int32_;
        } else {
            rtn = entry.data().float_;
        }
        return 0;
    }

    int Map::get_string(const char *const key, kv_string_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() != TYPE_STRING) {
            return ERROR_TYPE_ERROR;
        }

        rtn = entry.data().string_;
        return 0;
    }

    int Map::get_array(const char *const key, kv_array_t &rtn) {
        byte *ptr = nullptr;
        int code = get_value(key, &ptr);
        if (code < 0) {
            return code;
        }

        Entry entry;
        code = Entry::from_stream(ptr, &entry);
        if (code != 0) {
            return code;
        }

        if (entry.is_null()) {
            return VALUE_NULL;
        }

        if (entry.type() != TYPE_ARRAY) {
            return ERROR_TYPE_ERROR;
        }

        rtn = entry.data().array_;
        return 0;
    }

    bool kv_array_t::iterator::next(Entry *entry) {
        if (it_ >= end_) {
            return false;
        }

        int code = Entry::from_stream(it_, entry);
        if (code != 0) {
            return false;
        }

        it_ += Entry::get_entry_size(it_);
        return true;
    }
}