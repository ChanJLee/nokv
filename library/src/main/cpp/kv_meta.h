//
// Created by chan on 2022/11/30.
//

#ifndef NKV_KV_META_H
#define NKV_KV_META_H

#include <inttypes.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/stat.h>

namespace nokv {

    struct KVMeta {
        typedef timespec meta_t;

        int fd_;
        meta_t seq_;
        size_t size_;

        static KVMeta get_seq(int fd);

        static KVMeta next_seq(int fd);

        void update(int fd, const struct stat &st);

        bool operator==(const KVMeta &rhs) const;

        bool operator!=(const KVMeta &rhs) const;
    };
}

#endif //NKV_KV_META_H
