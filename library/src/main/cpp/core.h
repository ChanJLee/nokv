//
// Created by chan on 2022/11/20.
//

#ifndef NKV_CORE_H
#define NKV_CORE_H

#include "lock.h"
#include "kv.h"

namespace nkv {
    class KV {
        Lock lock_;
        int fd_;
        Map map_;
        int capacity_;

        KV(int fd, int capacity) : lock_(fd), fd_(fd), map_(), capacity_(capacity) {}

    public:
        void flush();

        void close();

        int write_int32(char *key, int32_t v);

        int write_float(char *key, float v);

        int write_int64(char *key, int64_t v);

        int write_boolean(char *key, bool v);

        int write_string(char *key, const char *const v);

        int read_int32(char *key, int32_t &v);

        int read_float(char *key, float &v);

        int read_int64(char *key, int64_t &v);

        int read_boolean(char *key, bool &v);

        int read_string(char *key, char **v);

    private:
        int write(char *key, byte *value, byte type, size_t size);

        int read(char *key, byte **value);

    public:
        KV *create(const char *file);

        void destroy(KV *kv);
    };

    int init(const char *meta_file);
}

#endif //NKV_CORE_H
