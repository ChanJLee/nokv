//
// Created by chan on 2022/11/20.
//

#ifndef NKV_LOCK_H
#define NKV_LOCK_H

#include <mutex>

namespace nkv {
    class Lock {
        int fd_;
        std::recursive_mutex thread_lock_;
    public: Lock(int fd): fd_(fd), thread_lock_() {}

        void lock();

        void unlock();
    };

    class ScopedLock {
        Lock &lock_;
    public:
        ScopedLock(Lock &lock): lock_(lock) {
            lock.lock();
        }

        ~ScopedLock() {
            lock_.unlock();
        }
    };
}

#endif //NKV_LOCK_H
