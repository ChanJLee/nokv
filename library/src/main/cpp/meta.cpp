//
// Created by chan on 2022/11/30.
//

#include "meta.h"
#include <sys/mman.h>
#include <string>
#include "kv.h"
#include <map>

namespace nokv {
    byte_t *gMetaBuf;
    size_t gMetaBufCapacity;
    std::map<const char *, KVMeta *> gMetaTable;

    KVMeta *KVMeta::get(const char *name) {
        const auto &it = gMetaTable.find(name);
        if (it != gMetaTable.end()) {
            return it->second;
        }

        byte_t *begin = gMetaBuf;
        byte_t *end = begin + gMetaBufCapacity;
        while (begin < end) {
            const char *key = reinterpret_cast<const char *>(begin);
            size_t key_len = strlen(key);
            if (key_len == 0) {
                size_t offset = strlen(name) + 1;
                memcpy(begin, name, offset);
                begin += offset;
                break;
            }

            if (strcmp(key, name) == 0) {
                begin = begin + key_len + 1;
                break;
            }

            begin = begin + key_len + 1 /* \0 */ + 4 /* seq size */;
        }

        if (begin >= end) {
            return nullptr;
        }

        auto *meta = new KVMeta(begin);
        gMetaTable[name] = meta;
        return meta;
    }

    uint32_t KVMeta::seq() {
        uint32_t id = 0;
        memcpy(&id, buf_, sizeof(id));
        return id;
    }

    uint32_t KVMeta::next() {
        uint32_t id = seq() + 1;
        memcpy(buf_, &id, sizeof(id));
        ::msync(buf_, sizeof(id), MS_SYNC);
        return id;
    }

    void KVMeta::init(void *buf, size_t size) {
        gMetaBuf = static_cast<byte_t *>(buf);
        gMetaBufCapacity = size;
    }
}