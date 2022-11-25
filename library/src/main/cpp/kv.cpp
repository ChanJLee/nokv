//
// Created by chan on 2022/11/25.
//

#include "kv.h"
#include <cstring>

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

int nokv::kv_string_t::to_stream(nokv::byte *stream, nokv::kv_string_t *str) {
    memcpy(stream, str->str_, str->size_ + 1);
    return 0;
}
