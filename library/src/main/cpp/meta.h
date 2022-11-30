//
// Created by chan on 2022/11/30.
//

#ifndef NKV_META_H
#define NKV_META_H

#include <inttypes.h>

namespace nokv {

    class KVMeta {
    public:
        static KVMeta *create(const char *file);

        uint32_t seq();
    };
}

#endif //NKV_META_H
