//
// Created by chan on 2025/8/27.
//
#include "lock.h"
#include <sys/file.h>

void AbstractLock::lock(bool share) {
    onLock(share);
}

void AbstractLock::unlock(bool share) {
    onUnlock(share);
}

void AbstractLock::tryLock(bool share) {
    onTryLock(share);
}

void PosixLock::onLock(bool share) {
    pthread_mutex_lock(&_thread_lock_);
}

void PosixLock::onUnlock(bool share) {
    pthread_mutex_unlock(&_thread_lock_);
}

bool PosixLock::onTryLock(bool share) {
    return pthread_mutex_trylock(&_thread_lock_) == 0;
}

PosixLock::~PosixLock() {
    if (_owner) {
        pthread_mutex_destroy(&_thread_lock_);
    }
}

void FileLock::onLock(bool share) {
    flock(fd_, share ? LOCK_SH : LOCK_EX);
}

void FileLock::onUnlock(bool share) {
    flock(fd_, LOCK_UN);
}

bool FileLock::onTryLock(bool share) {
    return flock(fd_, share ? LOCK_SH | LOCK_NB : LOCK_EX | LOCK_NB) == 0;
}

FileLock::~FileLock() = default;
