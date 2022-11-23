//
// Created by chan on 2022/11/20.
//

#ifndef NKV_IO_H
#define NKV_IO_H

#include "inttypes.h"

namespace nkv {
    typedef unsigned char byte;

    struct EntryHeader {
        byte *key_;
        byte type_;
    };

    struct MapHeader {
        char magic_[4] = {'n', 'k', 'v', '~'};
        int version_ = 0x12340100;
        uint16_t crc_;
        int size_;
    };
}

#endif //NKV_IO_H
