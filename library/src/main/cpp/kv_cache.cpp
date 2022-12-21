//
// Created by chan on 2022/12/21.
//

#include "kv_cache.h"
#include "kv_map.h"

int nokv::MemCache::read_all(const std::function<void(const kv_string_t &, Entry *)> &fnc) {
    Entry entry = {};
    for (auto it: fast_cache_) {
        int code = Entry::from_stream(it.second + it.first.byte_size(), &entry);
        if (code) return code;
        fnc(it.first, &entry);
    }
    return 0;
}

void nokv::MemCache::move_cache(const nokv::byte_t *const begin, const nokv::byte_t *const end,
                                const int64_t offset) {
    kv_cache_t tmp;
    while (!fast_cache_.empty()) {
        auto node = fast_cache_.extract(fast_cache_.begin());
        kv_string_t &key = node.key();
        byte_t *&value = node.mapped();
        bool val_dirty = value >= begin && value < end;
        if (val_dirty) {
            key.str_ = key.str_ + offset;
            value += offset;
        }
        tmp.insert(std::move(node));
    }
    fast_cache_ = std::move(tmp);
}

