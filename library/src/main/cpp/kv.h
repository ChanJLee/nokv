//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <inttypes.h>

namespace nkv {
    typedef unsigned char byte;

    struct Map {
        char magic_[4];
        uint16_t order;
        uint16_t version_;
        uint32_t crc_;
        uint32_t size_;
    } __attribute__ ((aligned (4)));
}

#endif //NKV_IO_H
