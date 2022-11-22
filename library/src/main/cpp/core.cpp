//
// Created by chan on 2022/11/20.
//

#include "lock.h"
#include "core.h"
#include <functional>
#include <string.h>

namespace nkv {

    int read_map(MapHeader *map, const char * const key, byte **mem) {
        if (map == nullptr) {
            return -1;
        }

        byte *begin = &map->mem_;
        byte *end = begin + map->header_.size_;
        EntryHeader entryHeader;
        while (begin < end) {
            memcpy(&entryHeader, begin, sizeof(EntryHeader));
            byte *entry_buf = begin + sizeof(EntryHeader);
            if (fnc(entryHeader.key_, entry_buf, entryHeader.value_size_, entryHeader.type_)) {
                return entry_buf;
            }
            begin = entry_buf + entryHeader.value_size_;
        }

        return -2;
    }

    int write_map(MapHeader *map, char *key, byte type, byte *value, int size) {
        if (map == nullptr) {
            return -1;
        }

        return 0;
    }

    int KV::read(char *key, byte *buf, int &buf_size, byte &type) {
        ScopedLock lock(lock_);
        return read_map(&header_, [&](char *entry_key, byte *entry_buf, int entry_buf_size,
                                   byte entry_type) -> bool {
            if (strcmp(key, entry_key) != 0) {
                return true;
            }

            memcpy(buf, entry_buf, entry_buf_size);
            buf_size = entry_buf_size;
            type = entry_type;

            return false;
        }) > 0;
    }

    int KV::write(char *key, byte type, byte *value, int size) {
        ScopedLock lock(lock_);
        return write_map(&header_, key, type, value, size);
    }
}