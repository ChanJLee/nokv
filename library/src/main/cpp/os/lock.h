//
// Created by chan on 2025/8/27.
//

#ifndef NKV_LOCK_H
#define NKV_LOCK_H

#include <pthread.h>

class AbstractLock {
public:
    virtual ~AbstractLock() = default;

    // 禁止重载
    void lock(bool share);

    void unlock(bool share);

    void tryLock(bool share);

protected:
    virtual void onLock(bool share) = 0;

    virtual void onUnlock(bool share) = 0;

    virtual bool onTryLock(bool share) = 0;
};

class PosixLock : public AbstractLock {
    pthread_mutex_t &_thread_lock_;
    bool _owner;
public:
    PosixLock(pthread_mutex_t &thread_lock, bool owner = false) : _thread_lock_(thread_lock),
                                                                  _owner(owner) {};

    ~PosixLock() override;

protected:
    void onLock(bool share) final;

    void onUnlock(bool share) final;

    bool onTryLock(bool share) final;
};

class FileLock : public AbstractLock {
    int fd_;
public:
    FileLock(int fd) : fd_(fd) {};

    ~FileLock() override;

protected:
    void onLock(bool share) final;

    void onUnlock(bool share) final;

    bool onTryLock(bool share) final;
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

#endif //NKV_LOCK_H
