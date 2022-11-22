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
        MapHeader header_;

    public:
        KV(int fd): lock_(fd), fd_(fd), header_() {}
        int write(char* key, byte type, byte* value);
        int read(char* key, byte* buf, size_t buf_size);
    };
}

#endif //NKV_CORE_H
