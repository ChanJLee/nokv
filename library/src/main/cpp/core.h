//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"
#include <cstring>
#include <functional>

namespace nokv {
    class KV {
        Lock lock_;
        int fd_;
        Map *map_;
        int capacity_;

        KV(int fd, int capacity, void *mem) : lock_(fd), fd_(fd),
                                              map_(reinterpret_cast<Map *>(mem)),
                                              capacity_(capacity) {
        }

        int read(const char *const key, byte **value);

        int write(const char *const key, byte *value, byte type, size_t size);

        static int check_kv(KV *kv);

        template<class T, class O>
        int cast_stream(byte *ptr, O &ret) {
            /* type compat */
            if (!Entry<T>::compat(ptr[0])) {
                return -1;
            }

            T tmp;
            memcpy(&tmp, ptr + 1, Entry<T>::size());
            ret = (T) tmp;
            return 0;
        }

    public:
        template<class T>
        int read(const char *const key, T &ret) {
            static_assert(Entry<T>::kv_type,
                          "only support kv_boolean_t/kv_string_t/kv_int32_t/kv_int64_t/kv_float_t");
            byte *ptr = nullptr;
            if (read(key, &ptr)) {
                return -1;
            }

            switch (ptr[0]) {
                case TYPE_INT32:
                    return cast_stream<kv_int32_t, T>(ptr, ret);
                case TYPE_FLOAT:
                    return cast_stream<kv_float_t, T>(ptr, ret);
                case TYPE_BOOLEAN:
                    return cast_stream<kv_boolean_t, T>(ptr, ret);
                case TYPE_INT64:
                    return cast_stream<kv_int64_t, T>(ptr, ret);
            }

            return -1;
        }

        template<class T = kv_string_t>
        int read(const char *const key, kv_string_t &ret) {
            byte *ptr = nullptr;
            if (read(key, &ptr)) {
                return -1;
            }

            if (ptr[0] == TYPE_STRING) {
                ret = (char *) ptr + 1;
                return 0;
            }

            return -1;
        }

        template<class T>
        int write(const char *const key, T v) {
            static_assert(Entry<T>::kv_type,
                          "only support kv_boolean_t/kv_string_t/kv_int32_t/kv_int64_t/kv_float_t");
            Entry<T> entry(v);
            return write(key,
                         entry.value(),
                         Entry<T>::kv_type,
                         entry.size()
            );
        }

        void lock() { lock_.lock(); }

        void unlock() { lock_.unlock(); }

        void flush();

        void close();

        size_t size() const { return map_->size_; }

        int read_all(
                const std::function<void(const char *const, const byte *, byte, size_t size)> &fnc);

        bool contains(const char *const key);

        static KV *create(const char *file);

        static void destroy(KV *kv);

        int remove_all();

        int remove(const char *const key);
    };

    int init(const char *meta_file);
}

#endif //NKV_CORE_H
