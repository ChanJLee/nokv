//
// Created by chan on 2022/11/20.
//

#ifndef NKV_KV_LOCK_H
#define NKV_KV_LOCK_H

#include <shared_mutex>

namespace nokv {

    class AbstractLock {
        int bad_;
        int good_;
    public:
        AbstractLock() : bad_(0), good_(0) {}

        virtual ~AbstractLock() {}

        void lock(bool share);

        void unlock(bool share);

    protected:
        virtual void doLock(bool share) = 0;

        virtual void doUnlock(bool share) = 0;

        virtual bool doTryLock(bool share) = 0;
    };

    class ThreadLock : public AbstractLock {
        std::shared_mutex thread_lock_;
    public:
        ThreadLock() : thread_lock_() {};

    protected:
        void doLock(bool share) override final;

        void doUnlock(bool share) override final;

        bool doTryLock(bool share) override final;
    };

    class ProcessLock : public AbstractLock {
        int fd_;
    public:
        ProcessLock(int fd) : fd_(fd) {};

    protected:
        void doLock(bool share) override final;

        void doUnlock(bool share) override final;

        bool doTryLock(bool share) override final;
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

#endif //NKV_KV_LOCK_H
