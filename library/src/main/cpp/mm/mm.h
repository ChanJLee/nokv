#ifndef __MM_H__
#define __MM_H__

#include <string>
#include <pthread.h>
#include "../os/boot.h"

namespace mm {

    class Memory {
        struct Nokv {
            char magic[4] = {'N', 'N', 'K', 'V'};
            char boot_id[BOOT_ID_SIZE] = {0};
            pthread_mutex_t mutex{};
        };

        Nokv *_kv;
        size_t _size;

        Memory(Nokv *kv, size_t size) : _kv(kv), _size(size) {}

    public:
        void *buffer() const { return _kv + sizeof(Nokv); }

        static Memory *create(const std::string &file, size_t size);
    };
}

#endif