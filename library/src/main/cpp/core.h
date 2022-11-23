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
    public:
        KV(int fd) : lock_(fd), fd_(fd), map_() {}

        void flush();

        void close();

        int write_int32(char *key, int32_t v);

        int write_float(char *key, float v);

        int write_int64(char *key, int64_t v);

        int write_boolean(char *key, bool v);

        int write_string(char *key, const char* const v);

        int read_int32(char *key, int32_t &v);

        int read_float(char *key, float &v);

        int read_int64(char *key, int64_t &v);

        int read_boolean(char *key, bool &v);

        int read_string(char *key, char **v);
    private:
        int write(char *key, byte *value, size_t size);

        int read(char *key, byte **value);
    };
}

#endif //NKV_CORE_H
