//
// Created by chan on 2022/11/30.
//

#ifndef NKV_META_H
#define NKV_META_H

#include <inttypes.h>
#include <stddef.h>

namespace nokv {

    class KVMeta {
        void *buf_;
    public:
        KVMeta(void *buf) : buf_(buf) {}

        uint32_t seq();

        uint32_t next();

        static KVMeta* get(const char* name);

        static void init(void* buf, size_t size);
    };
}

#endif //NKV_META_H
