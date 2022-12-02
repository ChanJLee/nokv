//
// Created by chan on 2022/11/20.
//

#ifndef NKV_LOCK_H
#define NKV_LOCK_H

#include <mutex>

namespace nokv {
    class Lock {
        int fd_;
        std::recursive_mutex thread_lock_;
    public: Lock(int fd): fd_(fd), thread_lock_() {}

        void lock();

        void unlock();
    };

    template <typename T>
    class ScopedLock {
        T &lock_;
    public:
        ScopedLock(T &lock) : lock_(lock) {
            lock_.lock();
        }

        ~ScopedLock() {
            lock_.unlock();
        }
    };
}

#endif //NKV_LOCK_H
