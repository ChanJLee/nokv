//
// Created by chan on 2022/11/20.
//
#include "lock.h"
#include <sys/file.h>
#include "log.h"

namespace nokv {
    void Lock::lock(bool share) {
        thread_lock_.lock(share);
        process_lock_.lock(share);
    }

    void Lock::unlock(bool share) {
        process_lock_.unlock(share);
        thread_lock_.unlock(share);
    }

    void ThreadLock::lock(bool share) {
        if (share) {
            thread_lock_.lock_shared();
        } else {
            thread_lock_.lock();
        }
    }

    void ThreadLock::unlock(bool share) {
        if (share) {
            thread_lock_.unlock_shared();
        } else {
            thread_lock_.unlock();
        }
    }

    void ProcessLock::lock(bool share) {
        for (int i = 0; i < 3 && flock(fd_, share ? LOCK_SH : LOCK_EX) != 0; ++i) {
            LOGD("lock %d failed, times: %d", fd_, i);
        }
    }

    void ProcessLock::unlock(bool share) {
        for (int i = 0; i < 3 && flock(fd_, LOCK_UN) != 0; ++i) {
            LOGD("unlock %d failed, times: %d", fd_, i);
        }
    }
}