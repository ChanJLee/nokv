//
// Created by chan on 2022/12/20.
//

#ifndef NKV_KV_TYPES_H
#define NKV_KV_TYPES_H

#include <inttypes.h>
#include <stddef.h>

namespace nokv {
    typedef unsigned char byte_t;
    typedef byte_t kv_type_t;

    /* 类型前缀 */
    const int TYPE_INT32 = 'I';
    const int TYPE_FLOAT = 'F';
    const int TYPE_INT64 = 'L';
    const int TYPE_BOOLEAN = 'B';
    const int TYPE_STRING = 'S';
    const int TYPE_ARRAY = 'A';
    const int TYPE_NULL = 'N';

    /* 必须是负值 */
    const int ERROR_OVERFLOW = -1; /* 写buffer满了 */
    const int ERROR_NOT_FOUND = -2; /* 没有找到 */
    const int ERROR_TYPE_ERROR = -3; /* 类型不兼容 */
    const int ERROR_INVALID_STATE = -4; /* 错误状态 */
    const int ERROR_MAP_FAILED = -5; /* mmap失败 */
    const int ERROR_INVALID_ARGUMENTS = -6; /* 无效参数 */
    const int ERROR_CACHE_INVALID = -7; /* 缓存失效 */

    const int CODE_OK = 0; /* ok */
    const int VALUE_NULL = 1; /* 空值 */

    /* 基本类型 */
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
}
#endif //NKV_KV_TYPES_H
