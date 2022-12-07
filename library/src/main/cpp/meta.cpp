//
// Created by chan on 2022/11/30.
//

#include "meta.h"
#include <string>
#include "kv.h"
#include <ctime>

namespace nokv {
    bool KVMeta::operator==(const KVMeta &rhs) const {
        return fd_ == rhs.fd_ &&
               seq_.tv_sec == rhs.seq_.tv_sec &&
               seq_.tv_nsec == rhs.seq_.tv_nsec &&
               size_ == rhs.size_;
    }

    bool KVMeta::operator!=(const KVMeta &rhs) const {
        return !(rhs == *this);
    }

    KVMeta KVMeta::seq(int fd) {
        KVMeta meta = {
                .fd_ = fd
        };
        struct stat st = {};
        fstat(fd, &st);
        meta.update(fd, st);
        return meta;
    }

    void KVMeta::next(size_t size) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        if (seq_.tv_sec == ts.tv_sec) {
            seq_.tv_sec++;
            return;
        }
        seq_ = ts;
        futimens(fd_, &ts);
        size_ = size;
    }

    void KVMeta::update(int fd, const struct stat &st) {
        seq_ = st.st_mtim;
        size_ = st.st_size;
        fd_ = fd;
    }
}