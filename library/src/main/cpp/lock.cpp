//
// Created by chan on 2022/11/20.
//
#include "lock.h"
#include <sys/file.h>
#include "log.h"

namespace nokv {
    void Lock::lock() {
        thread_lock_.lock();
        for (int i = 0; i < 3 && flock(fd_, LOCK_EX) != 0; ++i) {
            LOGD("lock %d failed, times: %d", fd_, i);
        }
    }

    void Lock::unlock() {
        thread_lock_.unlock();
        for (int i = 0; i < 3 && flock(fd_, LOCK_UN) != 0; ++i) {
            LOGD("unlock %d failed, times: %d", fd_, i);
        }
    }
}