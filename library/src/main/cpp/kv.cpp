//
// Created by chan on 2022/11/25.
//

#include "kv.h"
#include "log.h"
#include <cstring>
#include <zlib.h>

namespace nokv {

    int nokv::Entry::get_entry_size(nokv::byte_t *entry) {
        switch (entry[0]) {
            case nokv::TYPE_INT32:
            case nokv::TYPE_FLOAT:
                return 5;
            case nokv::TYPE_BOOLEAN:
                return 2;
            case nokv::TYPE_INT64:
                return 9;
            case nokv::TYPE_STRING:
                return strlen((char *) entry + 1) + 2;
            case nokv::TYPE_NULL:
                return 1;
            case nokv::TYPE_ARRAY: {
                int32_t size = 0;
                memcpy(&size, entry + 1, sizeof(size));
                return size + 1;
            }
        }

        return -1;
    }

    int nokv::Entry::from_stream(nokv::byte_t *stream, nokv::Entry *entry) {
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

    int nokv::kv_array_t::from_stream(nokv::byte_t *stream, nokv::kv_array_t *array) {
        array->begin_ = stream + 1;
        memcpy(&array->capacity_, array->begin_, 4);
        array->end_ = array->begin_ + array->capacity_;
        return 0;
    }

    int kv_array_t::create(kv_array_t &array) {
        array.capacity_ = 1024;
        array.begin_ = new byte_t[array.capacity_];
        array.begin_[0] = TYPE_ARRAY;
        array.end_ = array.begin_ + 4 /* 4u(len) */;
        return 0;
    }

    int kv_array_t::free(kv_array_t &array) {
        delete[] array.begin_;
        return 0;
    }

    int kv_array_t::put_string(const kv_string_t &str) {
        size_t str_len = strlen(str.str_) + 1;
        if (end_ + str_len + 1/* tag */> begin_ + capacity_) {
            resize();
        }
        end_[0] = TYPE_STRING;
        memcpy(++end_, str.str_, str_len);
        end_ += str_len;
        uint32_t size = end_ - begin_;
        memcpy(begin_, &size, 4);
        return 0;
    }

    int kv_array_t::put_null() {
        if (end_ + 1 > begin_ + capacity_) {
            resize();
        }
        end_[0] = TYPE_NULL;
        end_ = end_ + 1;
        uint32_t size = end_ - begin_;
        memcpy(begin_, &size, 4);
        return 0;
    }

    void kv_array_t::resize() {
        size_t new_size = capacity_ * 2;
        auto *buf = new byte_t[new_size];

        size_t size = end_ - begin_;
        memcpy(buf, begin_, size);

        delete[] begin_;

        begin_ = buf;
        end_ = begin_ + size;
        capacity_ = new_size;
    }

    int kv_array_t::put_string(const char *str) {
        kv_string_t string = {
                .str_ = str
        };
        return put_string(string);
    }

    int nokv::Map::put_array(const char *const key, const nokv::kv_array_t &array) {
        return put_value(key, TYPE_ARRAY, array.begin_, array.end_ - array.begin_);
    }

    int Map::put_string(const char *const key, const kv_string_t &str) {
        return put_value(key, TYPE_STRING, (byte_t *) str.str_, strlen(str.str_) + 1);
    }

    int Map::put_boolean(const char *const key, const kv_boolean_t &v) {
        return put_value(key, TYPE_BOOLEAN, (byte_t *) &v, 1);
    }

    int Map::put_null(const char *const key) {
        return put_value(key, TYPE_NULL, nullptr, 0);
    }

    int Map::put_int32(const char *const key, const kv_int32_t &v) {
        byte_t buf[4] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_INT32, buf, sizeof(buf));
    }

    int Map::put_int64(const char *const key, const kv_int64_t &v) {
        byte_t buf[8] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_INT64, buf, sizeof(buf));
    }

    int Map::put_float(const char *const key, const kv_float_t &v) {
        byte_t buf[4] = {0};
        memcpy(buf, &v, sizeof(buf));
        return put_value(key, TYPE_FLOAT, buf, sizeof(buf));
    }

    int Map::put_value(const char *const key, kv_type_t type, byte_t *value, size_t len) {
        byte_t *begin = this->begin();
        byte_t *end = this->end();

        byte_t *write_ptr = nullptr;
        get_value(key, &write_ptr);

        // 新值
        if (write_ptr == nullptr) {
            int code = put_value(end, key, type, value, len);
            if (code != 0) {
                return code;
            }
            header_.size_ = header_.size_ + len + 1 /* type */ + strlen(key) + 1;
            header_.crc_ = crc32(0, begin, header_.size_);
            memcpy(buf_, &header_, sizeof(Header));
            return 0;
        }

        // 旧值
        int prev_size = Entry::get_entry_size(write_ptr);
        if (prev_size < 0) {
            /* invalid state */
            return ERROR_INVALID_STATE;
        }

        if (prev_size == len + 1) {
            write_ptr[0] = type;
            memcpy(write_ptr + 1, value, len);
            header_.crc_ = crc32(0, begin, header_.size_);
            memcpy(buf_, &header_, sizeof(Header));
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
        header_.size_ = header_.size_ - prev_size + len + 1;
        header_.crc_ = crc32(0, begin, header_.size_);
        memcpy(buf_, &header_, sizeof(Header));
        return 0;
    }

    int Map::get_value(const char *const key, byte_t **ret) {
        int code = ERROR_NOT_FOUND;
        read_all(
                [&](const char *entry_key, size_t key_len, byte_t *body, size_t body_len) -> int {
                    if (strcmp(key, entry_key) != 0) {
                        return 0;
                    }

                    *ret = body;
                    code = 0;
                    return 1;
                }
        );

        /* not found */
        return code;
    }

    int Map::put_value(byte_t *where, const char *key, kv_type_t type, byte_t *value, size_t len) {
        size_t key_len = strlen(key);
        if (where + key_len + 1 + 1 + len >= begin_ + capacity_) {
            return ERROR_OVERFLOW;
        }

        memcpy(where, key, key_len + 1);
        where = where + key_len + 1;
        where[0] = type;
        memcpy(where + 1, value, len);
        return 0;
    }

    int Map::get_boolean(const char *const key, kv_boolean_t &rtn) {
        byte_t *ptr = nullptr;
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

        rtn = entry.data_.boolean_;
        return 0;
    }

    int Map::get_int32(const char *const key, kv_int32_t &rtn) {
        byte_t *ptr = nullptr;
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
            rtn = entry.data_.boolean_;
        } else {
            rtn = entry.data_.int32_;
        }
        return 0;
    }

    int Map::get_int64(const char *const key, kv_int64_t &rtn) {
        byte_t *ptr = nullptr;
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
            rtn = entry.data_.boolean_;
        } else if (entry.type() == TYPE_INT32) {
            rtn = entry.data_.int32_;
        } else {
            rtn = entry.data_.int64_;
        }
        return 0;
    }

    int Map::get_float(const char *const key, kv_float_t &rtn) {
        byte_t *ptr = nullptr;
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
            rtn = entry.data_.boolean_;
        } else if (entry.type() == TYPE_INT32) {
            rtn = (kv_float_t) entry.data_.int32_;
        } else {
            rtn = entry.data_.float_;
        }
        return 0;
    }

    int Map::get_string(const char *const key, kv_string_t &rtn) {
        byte_t *ptr = nullptr;
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

        rtn = entry.data_.string_;
        return 0;
    }

    int Map::get_array(const char *const key, kv_array_t &rtn) {
        byte_t *ptr = nullptr;
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

        rtn = entry.data_.array_;
        return 0;
    }

    bool Map::contains(const char *const key) {
        byte_t *ptr = nullptr;
        if (get_value(key, &ptr)) {
            return false;
        }
        return ptr != nullptr;
    }

    int Map::read_all(
            const std::function<void(const char *const, Entry *)> &fnc) {
        Entry entry;
        return read_all([&](const char *key, size_t key_len, byte_t *body, size_t body_len) -> int {
            if (Entry::from_stream(body, &entry)) {
                /* invalid state */
                return ERROR_INVALID_STATE;
            }

            fnc(key, &entry);
            return 0;
        });
    }

    int Map::remove(const char *const key) {
        return read_all(
                [&](const char *entry_key, size_t key_len, byte_t *body, size_t body_len) -> int {
                    if (strcmp(key, entry_key) != 0) {
                        return 0;
                    }

                    memcpy((void *) entry_key, body + body_len, key_len + body_len);
                    return 1;
                });
    }

    int Map::read_all(
            const std::function<int(const char *, size_t, byte_t *, size_t)> &fnc) {
        byte_t *begin = this->begin();
        byte_t *end = this->end();

        while (begin < end) {
            char *entry_key = reinterpret_cast<char *>(begin);
            size_t key_len = strlen(entry_key);
            byte_t *data = begin + key_len + 1;
            int entry_size = Entry::get_entry_size(data);
            if (entry_size < 0) {
                /* invalid state */
                return ERROR_INVALID_STATE;
            }

            int code = fnc((const char *) begin, key_len + 1, data, entry_size);
            if (code > 0) {
                return 0;
            } else if (code < 0) {
                return code;
            }

            begin = data + entry_size;
        }
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