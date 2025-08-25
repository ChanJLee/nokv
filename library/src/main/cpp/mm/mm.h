#ifndef __MM_H__
#define __MM_H__

#include <string>
#include <pthread.h>

namespace mm {

    class Memory {
        struct ShmMutex {
            pthread_mutex_t mutex;
            int counter;
            int initialized; // 0 = 未初始化, 1 = 初始化完成
        };
        ShmMutex* _mutex;
        size_t _size;

        Memory(ShmMutex* mutex, size_t size): _mutex(mutex), _size(size) {}
    public:
        static Memory *create(const std::string &file);
    };
}

#endif