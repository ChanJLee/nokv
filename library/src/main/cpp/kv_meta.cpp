//
// Created by chan on 2022/11/30.
//

#include "kv_meta.h"
#include <string>
#include "kv_map.h"

namespace nokv {
    bool KVMeta::operator==(const KVMeta &rhs) const {
        return fd_ == rhs.fd_ &&
               memcmp(&seq_, &rhs.seq_, sizeof(seq_)) == 0 &&
               size_ == rhs.size_;
    }

    bool KVMeta::operator!=(const KVMeta &rhs) const {
        return !(rhs == *this);
    }

    void next_seq(int fd) {
        struct timespec ts = {};
        clock_gettime(CLOCK_REALTIME, &ts);
        futimens(fd, &ts);
    }

    KVMeta KVMeta::seq(int fd) {
        next_seq(fd);
        KVMeta meta = {
                .fd_ = fd
        };
        struct stat st = {};
        fstat(fd, &st);
        meta.update(fd, st);
        return meta;
    }

    void KVMeta::update(int fd, const struct stat &st) {
        seq_ = st.st_mtim;
        size_ = st.st_size;
        fd_ = fd;
    }
}