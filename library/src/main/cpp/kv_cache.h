//
// Created by chan on 2022/12/20.
//

#ifndef NKV_KV_CACHE_H
#define NKV_KV_CACHE_H

#include "kv_types.h"
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cstring>

namespace nokv {
    class MemCache {
        template<class _Tp>
        struct predicate : public std::binary_function<_Tp, _Tp, bool> {
            bool operator()(const _Tp &__x, const _Tp &__y) const {
                return __x.size_ == __y.size_ &&
                       (__x.str_ == __y.str_ || strncmp(__x.str_, __y.str_, __x.size_) == 0);
            }
        };

        struct hash {
            size_t operator()(const kv_string_t &key) const {
                int seed = 31;
                size_t hash = 0;
                for (kv_string_t::kv_string_size_t i = 0; i < key.size_; ++i) {
                    hash = (hash * seed) + key.str_[i];
                }
                return hash;
            }
        };

        typedef std::unordered_map<kv_string_t, byte_t *, hash, predicate<kv_string_t>> kv_cache_t;
        typedef kv_cache_t::value_type kv_cache_value_t;
        kv_cache_t fast_cache_;

    public:
        void clear() { fast_cache_.clear(); }

        size_t size() const { return fast_cache_.size(); }

        void put(byte_t *entry) {
            kv_string_t key = {};
            kv_string_t::from_stream(entry, key);
            put(key, entry);
        }

        void put(const kv_string_t &key, byte_t *entry) {
            fast_cache_[key] = entry;
        }

        int get(const kv_string_t &key, byte_t *&entry) const {
            auto it = fast_cache_.find(key);
            if (it == fast_cache_.end()) {
                return ERROR_NOT_FOUND;
            }

            entry = it->second;
            return 0;
        }

        void remove(const kv_string_t &key) {
            fast_cache_.erase(key);
        }

        void move_cache(const byte_t *const begin, const byte_t *const end, const int64_t offset);

        int read_all(const std::function<void(const kv_string_t &, Entry *)> &fnc);
    };
}

#endif //NKV_KV_CACHE_H
