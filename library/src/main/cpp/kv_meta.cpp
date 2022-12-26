//
// Created by chan on 2022/11/30.
//

#include "kv_meta.h"
#include <string>
#include <unistd.h>
#include "kv_map.h"
#include "kv_log.h"

namespace nokv {
    bool KVMeta::operator==(const KVMeta &rhs) const {
        return fd_ == rhs.fd_ &&
               memcmp(&seq_, &rhs.seq_, sizeof(seq_)) == 0 &&
               size_ == rhs.size_;
    }

    bool KVMeta::operator!=(const KVMeta &rhs) const {
        return !(rhs == *this);
    }

    KVMeta KVMeta::next_seq(int fd) {
        struct timespec ts = {};
        clock_gettime(CLOCK_REALTIME, &ts);
        struct timespec times[2] = {0};
        times[0] = ts;
        times[1] = ts;
        if (futimens(fd, times)) {
#ifdef NKV_UNIT_TEST
            exit(1);
#endif
        }
        return get_seq(fd);
    }

    KVMeta KVMeta::get_seq(int fd) {
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