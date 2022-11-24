//
// Created by chan on 2022/11/20.
//
#include "lock.h"
#include <fcntl.h>
#include "log.h"

namespace nkv {
    void Lock::lock() {
        thread_lock_.lock();
        int code = 0;
        for (int i = 0; i < 3 && (code = flock(fd_, LOCK_EX)) != 0; ++i) {
            LOGD("lock %d failed, times: %d", fd_, i);
        }
    }

    void Lock::unlock() {
        thread_lock_.unlock();
        int code = 0;
        for (int i = 0; i < 3 && (code = flock(fd_, LOCK_UN)) != 0; ++i) {
            LOGD("unlock %d failed, times: %d", fd_, i);
        }
    }
}