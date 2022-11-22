//
// Created by chan on 2022/11/20.
//
#include "lock.h"

namespace nkv {
    void Lock::lock() {
        thread_lock_.lock();
        // todo process lock
    }

    void Lock::unlock() {
        thread_lock_.unlock();
        // todo process lock
    }
}