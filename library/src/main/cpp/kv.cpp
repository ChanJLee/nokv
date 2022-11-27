//
// Created by chan on 2022/11/25.
//

#include "kv.h"
#include <cstring>

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
    public:
        Entry() : type_(TYPE_NULL) {}

        bool is_null() const { return type_ == TYPE_NULL; }

        kv_type_t type() const { return type_; }

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
            return nokv::kv_string_t::from_stream(stream, &entry->data_.string_);
        }

        if (entry->type_ == TYPE_ARRAY) {
            return nokv::kv_array_t::from_stream(stream, &entry->data_.array_);
        }

        int size = get_entry_size(stream);
        if (size < 0) {
            return -1;
        }

        memcpy(&entry->data_, stream + 1, size);
        return 0;
    }

    int nokv::kv_array_t::from_stream(nokv::byte *stream, nokv::kv_array_t *array) {
        if (array == nullptr) {
            return -1;
        }

        memcpy(&array->byte_size_, ++stream, 4);
        stream += 4;
        memcpy(&array->elem_size_, ++stream, 4);
        stream += 4;
        array->data_ = stream;
        return 0;
    }

    int nokv::kv_array_t::to_stream(nokv::byte *stream, nokv::kv_array_t *array) {
        *stream = TYPE_ARRAY;
        memcpy(++stream, &array->byte_size_, 4);
        stream += 4;
        memcpy(++stream, &array->byte_size_, 4);
        stream += 4;
        memcpy(stream, array->data_, array->byte_size_);
        return 0;
    }

    int nokv::kv_string_t::from_stream(nokv::byte *stream, nokv::kv_string_t *str) {
        if (str == nullptr) {
            return -1;
        }
        str->str_ = (const char *) (stream + 1);
        str->size_ = strlen(str->str_);
        return 0;
    }

    int nokv::Map::put_array(const char *const key, const nokv::kv_array_t &array) {
        return put_value(key, TYPE_ARRAY, array.begin_, array.end_ - array.begin_);
    }

    int Map::put_string(const char *const key, const kv_string_t &str) {
        return put_value(key, TYPE_STRING, (byte *) str.str_, str.size_ + 1);
    }

    int Map::put_boolean(const char *const key, const kv_boolean_t &v) {
        put_value(key, TYPE_BOOLEAN, (byte *) &v, 1);
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
}