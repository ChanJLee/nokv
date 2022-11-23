//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include <inttypes.h>

namespace nkv {
    typedef unsigned char byte;

    struct Map {
        char magic_[4] = {'n', 'k', 'v', '~'};
        uint16_t order = 0x1234;
        uint16_t version_ = 0x0101;
        uint32_t crc_;
        size_t size_;
    } __attribute__ ((aligned (8)));
}

#endif //NKV_IO_H
