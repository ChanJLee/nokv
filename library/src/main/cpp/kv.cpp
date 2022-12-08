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
            case nokv::TYPE_STRING: {
                int32_t size = 0;
                memcpy(&size, entry + 1, sizeof(size));
                return 4 /* str len */ + size + 1 /* \0 */ + 1 /* tag */;
            }
            case nokv::TYPE_NULL:
                return 1;
            case nokv::TYPE_ARRAY: {
                int32_t size = 0;
                memcpy(&size, entry + 1, sizeof(size));
                return size + 1 + 4;
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
            return kv_string_t::from_stream(stream + 1, &entry->data_.string_);
        }

        if (entry->type_ == TYPE_ARRAY) {
            return nokv::kv_array_t::from_stream(stream + 1, &entry->data_.array_);
        }

        int size = get_entry_size(stream);
        if (size < 0) {
            return ERROR_INVALID_STATE;
        }

        memcpy(&entry->data_, stream + 1, size);
        return 0;
    }

    int nokv::kv_array_t::from_stream(nokv::byte_t *stream, nokv::kv_array_t *array) {
        memcpy(&array->capacity_, stream, 4);
        array->begin_ = stream + 4;
        array->end_ = array->begin_ + array->capacity_;
        return 0;
    }

    int kv_array_t::to_stream(byte_t *stream) const {
        uint32_t size = end_ - begin_;
        memcpy(stream, &size, sizeof(size));
        memcpy(stream + sizeof(size), begin_, size);
        return 0;
    }

    int kv_array_t::create(kv_array_t &array) {
        array.capacity_ = 1024;
        array.begin_ = new byte_t[array.capacity_];
        array.end_ = array.begin_;
        return 0;
    }

    int kv_array_t::free(kv_array_t &array) {
        delete[] array.begin_;
        return 0;
    }

    int kv_array_t::put_string(const kv_string_t &str) {
        size_t byte_len = str.byte_size() + 1;
        if (end_ + byte_len > begin_ + capacity_) {
            resize();
        }

        end_[0] = TYPE_STRING;
        str.to_stream(end_ + 1);
        end_ = end_ + byte_len;
        return 0;
    }

    int kv_array_t::put_null() {
        if (end_ + 1 > begin_ + capacity_) {
            resize();
        }
        end_[0] = TYPE_NULL;
        end_ = end_ + 1;
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
                .size_ = (uint32_t) strlen(str),
                .str_ = str
        };
        return put_string(string);
    }

    int nokv::Map::put_array(const kv_string_t &key, const nokv::kv_array_t &array) {
        return put_value(key, TYPE_ARRAY, [=](byte_t *buf) -> void {
            array.to_stream(buf);
        }, array.byte_count());
    }

    int Map::put_string(const kv_string_t &key, const kv_string_t &str) {
        return put_value(key, TYPE_STRING, [=](byte_t *buf) -> void {
            str.to_stream(buf);
        }, str.byte_size());
    }

    int Map::put_boolean(const kv_string_t &key, const kv_boolean_t &v) {
        return put_value(key, TYPE_BOOLEAN, [=](byte_t *buf) -> void {
            buf[0] = v;
        }, 1);
    }

    int Map::put_null(const kv_string_t &key) {
        return put_value(key, TYPE_NULL, [=](byte_t *buf) -> void {
        }, 0);
    }

    int Map::put_int32(const kv_string_t &key, const kv_int32_t &v) {
        return put_value(key, TYPE_INT32, [=](byte_t *buf) -> void {
            memcpy(buf, &v, 4);
        }, 4);
    }

    int Map::put_int64(const kv_string_t &key, const kv_int64_t &v) {
        return put_value(key, TYPE_INT64, [=](byte_t *buf) -> void {
            memcpy(buf, &v, 8);
        }, 8);
    }

    int Map::put_float(const kv_string_t &key, const kv_float_t &v) {
        return put_value(key, TYPE_FLOAT, [=](byte_t *buf) -> void {
            memcpy(buf, &v, 4);
        }, 4);
    }

    int
    Map::put_value(const kv_string_t &key, kv_type_t type, const std::function<void(byte_t *)> &is,
                   size_t len) {
        size_t precompute_size = key.byte_size() /* key */ + 1 /* tag*/ + len /* value */;
        if (header_.size_ + precompute_size >= capacity_) {
            return ERROR_OVERFLOW;
        }


        byte_t *begin = this->begin();
        byte_t *end = this->end();

        byte_t *write_ptr = nullptr;
        get_value(key, &write_ptr);

        // 1. 先保证数据结构完整
        // 不更新crc，方便后面可以恢复
        auto prev_total_size = header_.size_;
        header_.size_ = write_ptr == nullptr ? prev_total_size : write_ptr - begin;
        memcpy(buf_, &header_, sizeof(Header));

        size_t new_size = 0;

        // 2. 开始计算写入位置
        if (write_ptr == nullptr) {
            write_ptr = end;
            new_size = prev_total_size + key.byte_size() + len + 1;
            goto do_write;
        }

        {
            int prev_size = Entry::get_entry_size(write_ptr);
            if (prev_size < 0) {
                /* invalid state */
                return ERROR_INVALID_STATE;
            }

            if (prev_size == len + 1) {
                write_ptr -= key.byte_size();
                new_size = prev_total_size;
                goto do_write;
            }

            // 长度发生了变化就要重排
            size_t offset_size = end - write_ptr - prev_size;
            byte_t *adjust_ptr = write_ptr - key.byte_size();
            memcpy(adjust_ptr, write_ptr + prev_size, offset_size);
            write_ptr = adjust_ptr + offset_size;
            build_lru_cache(adjust_ptr, write_ptr);
            new_size = prev_total_size + (len + 1 - prev_size);
            goto do_write;
        }

        do_write:
        byte_t *save = write_ptr;
        key.to_stream(write_ptr);
        write_ptr += key.byte_size();
        write_ptr[0] = type;
        is(write_ptr + 1);
        header_.size_ = new_size;
        memcpy(buf_, &header_, sizeof(Header));
        lru_[key.str_] = save;
        return 0;
    }

    int Map::get_value(const kv_string_t &key, byte_t **ret) {
        int code = ERROR_NOT_FOUND;

        const auto &it = lru_.find(key.str_);
        if (it != lru_.end()) {
            // cache hit
            byte_t *cache = it->second;
            byte_t *end = this->end();
            if (cache >= begin() && cache < end) {
                kv_string_t temp = {};
                if (kv_string_t::from_stream_safe(cache, &temp, end) == 0 &&
                    key.size_ == temp.size_ &&
                    strncmp(key.str_, temp.str_, key.size_) == 0) {
                    *ret = cache + key.byte_size();
                    return 0;
                }
            }
        }

        read_all(
                [&](const kv_string_t &entry_key, byte_t *body,
                    size_t body_len) -> int {
                    if (key.size_ != entry_key.size_ ||
                        strncmp(key.str_, entry_key.str_, key.size_) != 0) {
                        return 0;
                    }

                    lru_[key.str_] = body - key.byte_size();
                    *ret = body;
                    code = 0;
                    return 1;
                }
        );

        /* not found */
        return code;
    }

    int Map::get_boolean(const kv_string_t &key, kv_boolean_t &rtn) {
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

    int Map::get_int32(const kv_string_t &key, kv_int32_t &rtn) {
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

    int Map::get_int64(const kv_string_t &key, kv_int64_t &rtn) {
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

    int Map::get_float(const kv_string_t &key, kv_float_t &rtn) {
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

    int Map::get_string(const kv_string_t &key, kv_string_t &rtn) {
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

    int Map::get_array(const kv_string_t &key, kv_array_t &rtn) {
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

    bool Map::contains(const kv_string_t &key) {
        byte_t *ptr = nullptr;
        if (get_value(key, &ptr)) {
            return false;
        }
        return ptr != nullptr;
    }

    int Map::read_all(
            const std::function<void(const kv_string_t &, Entry *)> &fnc) {
        Entry entry;
        return read_all(
                [&](const kv_string_t &key, byte_t *body, size_t body_len) -> int {
                    if (Entry::from_stream(body, &entry)) {
                        /* invalid state */
                        return ERROR_INVALID_STATE;
                    }

                    fnc(key, &entry);
                    return 0;
                });
    }

    int Map::remove(const kv_string_t &key) {
        if (header_.size_ == 0) {
            return 0;
        }

        return read_all(
                [&](const kv_string_t &entry_key, byte_t *body,
                    size_t body_len) -> int {
                    if (key.size_ != entry_key.size_ ||
                        strncmp(key.str_, entry_key.str_, key.size_) != 0) {
                        return 0;
                    }

                    size_t count = entry_key.byte_size() + body_len;
                    memcpy(body - entry_key.byte_size(), body + body_len, count);
                    header_.size_ -= count;
                    header_.crc_ = crc32(0, begin(), header_.size_);
                    return 1;
                });
    }

    int Map::read_all(
            const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc) {
        return read_all(begin(), end(), fnc);
    }

    int Map::read_all(
            byte_t *begin, byte_t *end,
            const std::function<int(const kv_string_t &, byte_t *, size_t)> &fnc) {
        kv_string_t key = {};

        while (begin < end) {
            kv_string_t::from_stream(begin, &key);
            byte_t *data = begin + key.byte_size();
            int entry_size = Entry::get_entry_size(data);
            if (entry_size < 0) {
                /* invalid state */
                LOGD("invalid state");
                return ERROR_INVALID_STATE;
            }

            int code = fnc(key, data, entry_size);
            if (code > 0) {
                return 0;
            } else if (code < 0) {
                return code;
            }

            begin = data + entry_size;
        }
        return 0;
    }

    int Map::remove_all() {
        // todo test only clear all
        header_.crc_ = 0;
        header_.size_ = 0;
        return 0;
    }

    void Map::sync() {
        header_.crc_ = crc32(0, begin(), header_.size_);
        memcpy(buf_, &header_, sizeof(Header));
    }

    void Map::build_lru_cache(byte_t *begin, byte_t *end) {
        read_all(begin, end, [=](const kv_string_t &key, byte_t *body, size_t) -> int {
            lru_[key.str_] = body - key.byte_size();
            return 0;
        });
    }

    bool kv_array_t::iterator::next(Entry *entry) {
        if (begin_ >= end_) {
            return false;
        }

        int code = Entry::from_stream(begin_, entry);
        if (code != 0) {
            return false;
        }

        size_t offset = Entry::get_entry_size(begin_);
        begin_ += offset;
        return true;
    }

    int kv_string_t::to_stream(byte_t *stream) const {
        memcpy(stream, (void *) &size_, sizeof(size_));
        stream += sizeof(size_);
        memcpy(stream, str_, size_ + 1);
        return 0;
    }

    int kv_string_t::from_stream(byte_t *stream, kv_string_t *str) {
        memcpy(&str->size_, stream, 4);
        str->str_ = (const char *) (stream + sizeof(str->size_));
        return 0;
    }

    int kv_string_t::from_stream_safe(byte_t *stream, kv_string_t *str, byte_t *end) {
        memcpy(&str->size_, stream, 4);
        byte_t *str_end = stream + str->size_ + 4;
        if (str_end >= end && *str_end != '\0') {
            return ERROR_INVALID_STATE;
        }

        str->str_ = (const char *) (stream + sizeof(str->size_));
        return 0;
    }
}