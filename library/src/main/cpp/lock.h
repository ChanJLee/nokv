//
// Created by chan on 2022/11/20.
//

#ifndef NKV_LOCK_H
#define NKV_LOCK_H

#include <shared_mutex>

namespace nokv {

    class ThreadLock {
        std::shared_mutex thread_lock_;
    public:
        ThreadLock() {};

        void lock(bool share);

        void unlock(bool share);
    };

    class ProcessLock {
        int fd_;
    public:
        ProcessLock(int fd) : fd_(fd) {};

        void lock(bool share);

        void unlock(bool share);
    };

    class Lock {
        ThreadLock thread_lock_;
        ProcessLock process_lock_;
    public:
        Lock(int fd) : process_lock_(fd), thread_lock_() {}

        void lock(bool share);

        void unlock(bool share);
    };

    template<typename T, bool share>
    class ScopedLock {
        T &lock_;
    public:
        ScopedLock(T &lock) : lock_(lock) {
            lock_.lock(share);
        }

        ~ScopedLock() {
            lock_.unlock(share);
        }
    };
}

#endif //NKV_LOCK_H
