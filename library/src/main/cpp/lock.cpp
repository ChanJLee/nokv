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

    void AbstractLock::lock(bool share) {
        if (bad_ < 2 && doTryLock(share)) {
            ++good_;
            if (good_ > 4) {
                bad_ = 0;
            }
            return;
        }

        doLock(share);
        ++bad_;
        good_ = 0;
    }

    void AbstractLock::unlock(bool share) {
        doUnlock(share);
    }

    void ThreadLock::doLock(bool share) {
        if (share) {
            thread_lock_.lock_shared();
            return;
        }

        thread_lock_.lock();
    }

    void ThreadLock::doUnlock(bool share) {
        if (share) {
            thread_lock_.unlock_shared();
            return;
        }

        thread_lock_.unlock();
    }

    bool ThreadLock::doTryLock(bool share) {
        return share ? thread_lock_.try_lock_shared() : thread_lock_.try_lock();
    }

    void ProcessLock::doLock(bool share) {
        for (int i = 0; i < 3 && flock(fd_, share ? LOCK_SH : LOCK_EX) != 0; ++i) {
            LOGD("lock %d failed, times: %d", fd_, i);
        }
    }

    void ProcessLock::doUnlock(bool share) {
        for (int i = 0; i < 3 && flock(fd_, LOCK_UN) != 0; ++i) {
            LOGD("unlock %d failed, times: %d", fd_, i);
        }
    }

    bool ProcessLock::doTryLock(bool share) {
        int flag = share ? LOCK_SH : LOCK_EX;
        return flock(fd_, flag | LOCK_NB) == 0;
    }
}