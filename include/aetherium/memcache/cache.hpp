/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Aetherium.
 * 
 * Aetherium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * Aetherium is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Aetherium. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AETHERIUM_SYS_CACHE_HPP
#define AETHERIUM_SYS_CACHE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <aetherium/sys/time.hpp>

namespace Aetherium::MemoryCache {

#define CACHE_MAX_SIZE_BYTES 1024 * 1024 * 10
#define CACHE_MAX_ITEMS 1000

#define CACHE_DEFAULT_PRIORITY 0
#define CACHE_DEFAULT_TTL_SECONDS 3600
#define CACHE_CLEANUP_INTERVAL_MS 5000

template <typename T, typename = void>
struct is_container : std::false_type { };

template <typename T>
struct is_container<T, std::void_t<
    decltype(std::declval<T>().size()),
    typename T::value_type
>> : std::true_type {};

template <
    typename T,
    typename std::enable_if_t<
        !std::is_same_v<T, std::string> &&
        !is_container<T>::value &&
        !std::is_same_v<T, const char*>,
        int
    > = 0
>
size_t get_estimated_size(const T& value) {
    return sizeof(T);
}

template <
    typename std::enable_if_t<
        std::is_same_v<
            std::string,
            std::string
        >,
        int
    > = 0
>
size_t get_estimated_size(
    const std::string& value
) {
    return value.length() + 1;
}

template <
    typename T,
    typename std::enable_if_t<
        std::is_same_v<T, const char*> ||
        std::is_array_v<std::remove_reference_t<T>>,
        int
    > = 0
>
size_t get_estimated_size(const T& value) {
    return strlen(value) + 1;
}

template <
    typename T,
    typename std::enable_if_t<
        is_container<T>::value,
        int
    > = 0
>
size_t get_estimated_size(const T& value) {
    return value.size() *
        sizeof(typename T::value_type) +
        sizeof(T);
}

template <typename T>
struct CacheItem {
    T value;
    long long timestamp;
    long long ttl;

    size_t size_bytes;
    int priority;

    CacheItem(
        const T& val,
        long long expiry_ms,
        size_t s_bytes,
        int p = CACHE_DEFAULT_PRIORITY
    ) : value(val),
        timestamp(Aetherium::Sys::timestamp_ms()),
        ttl(expiry_ms),
        size_bytes(s_bytes),
        priority(p) { }

    bool is_expired() const {
        return (Aetherium::Sys::timestamp_ms() - timestamp) > ttl;
    }
};

struct CacheStats {
    long long hits;
    long long misses;
    long long current_size_bytes;
    long long current_item_count;
    long long evictions;
    long long expired_evictions;
    long long capacity_evictions;
    long long priority_evictions;

    CacheStats() :
        hits(0),
        misses(0),
        current_size_bytes(0),
        current_item_count(0),
        evictions(0),
        expired_evictions(0),
        capacity_evictions(0),
        priority_evictions(0) { }

    void reset() {
        hits = 0;
        misses = 0;

        current_size_bytes = 0;
        current_item_count = 0;

        expired_evictions = 0;
        capacity_evictions = 0;

        evictions = 0;
        priority_evictions = 0;
    }
};

template <typename Key, typename Value>
class ICache {
public:
    virtual ~ICache() = default;

    virtual void put(
        const Key& key,
        const Value& value,
        long long ttl_seconds = CACHE_DEFAULT_TTL_SECONDS,
        size_t size_bytes = 0,
        int priority = CACHE_DEFAULT_PRIORITY
    ) = 0;

    virtual bool get(const Key& key, Value& value) = 0;

    virtual void remove(const Key& key) = 0;
    virtual bool contains(const Key& key) const = 0;

    virtual void clear() = 0;

    virtual CacheStats get_stats() const = 0;

    virtual void start_thread_cleanup() = 0;
    virtual void stop_thread_cleanup() = 0;
};

template <typename Key, typename Value>
class LruCache : public ICache<Key, Value> {
private:
    std::map<
        Key,
        typename std::list<
            std::pair<Key, CacheItem<Value>>
        >::iterator
    > cache_map;

    std::list<std::pair<
        Key,
        CacheItem<Value>
    >> lru_list;

    CacheStats stats;
    mutable std::mutex cache_mutex;

    size_t max_size_bytes;
    size_t max_items;

    std::atomic<bool> stop_cleanup_thread_flag;
    std::thread cleanup_thread;

    void evict_one() {
        if(lru_list.empty())
            return;

        auto victim_it = lru_list.end();
        int min_priority = std::numeric_limits<int>::max();

        for(auto it = lru_list.rbegin(); it != lru_list.rend(); ++it)
            if(it->second.priority < min_priority) {
                min_priority = it->second.priority;
                victim_it = std::next(it).base();
            }
            else if(it->second.priority == min_priority)
                victim_it = std::next(it).base();

        if(victim_it != lru_list.end()) {
            stats.current_size_bytes -= victim_it->second.size_bytes;
            stats.current_item_count--;

            stats.evictions++;
            stats.capacity_evictions++;
            stats.priority_evictions++;

            cache_map.erase(victim_it->first);
            lru_list.erase(victim_it);
        }
    }

    void cleanup_task() {
        while(!stop_cleanup_thread_flag.load()) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(CACHE_CLEANUP_INTERVAL_MS)
            );

            if(stop_cleanup_thread_flag.load())
                break;
            std::unique_lock<std::mutex> lock(cache_mutex);

            auto it = lru_list.begin();
            while(it != lru_list.end()) {
                if(it->second.is_expired()) {
                    stats.current_size_bytes -= it->second.size_bytes;

                    stats.current_item_count--;
                    stats.evictions++;
                    stats.expired_evictions++;

                    cache_map.erase(it->first);
                    it = lru_list.erase(it);
                }
                else ++it;
            }

            while(static_cast<size_t>(stats.current_size_bytes) >
                    max_size_bytes ||
                static_cast<size_t>(stats.current_item_count) >
                    max_items) {
                if(lru_list.empty())
                    break;
                evict_one();
            }
        }
    }

public:
    LruCache(
        size_t max_s_bytes,
        size_t max_i
    ) : cache_map({}),
        lru_list({}),
        stats(),
        cache_mutex(),
        max_size_bytes(max_s_bytes),
        max_items(max_i),
        stop_cleanup_thread_flag(false),
        cleanup_thread()
    {
        max_s_bytes = CACHE_MAX_SIZE_BYTES;
        max_i = CACHE_MAX_ITEMS;

        this->start_thread_cleanup();
    }

    ~LruCache() override {
        this->stop_thread_cleanup();
    }

    void put(
        const Key& key,
        const Value& value,
        long long ttl_seconds = CACHE_DEFAULT_TTL_SECONDS,
        size_t initial_size_bytes = 0,
        int priority = CACHE_DEFAULT_PRIORITY
    ) override {
        std::unique_lock<std::mutex> lock(cache_mutex);
        long long ttl_ms = ttl_seconds * 1000;

        size_t actual_size_bytes = initial_size_bytes;
        if(actual_size_bytes == 0)
            actual_size_bytes = get_estimated_size(value);

        auto it = cache_map.find(key);
        if(it != cache_map.end()) {
            lru_list.erase(it->second);
            stats.current_size_bytes -=
                it->second->second.size_bytes;
        }

        lru_list.emplace_front(
            key,
            CacheItem<Value>(
                value,
                ttl_ms,
                actual_size_bytes,
                priority
            )
        );

        cache_map[key] = lru_list.begin();
        stats.current_size_bytes += actual_size_bytes;
        stats.current_item_count++;

        while(static_cast<size_t>(stats.current_size_bytes) >
                max_size_bytes ||
            static_cast<size_t>(stats.current_item_count) >
                max_items
        ) {
            if(lru_list.empty())
                break;

            this->evict_one();
        }
    }

    bool get(const Key& key, Value& value) override {
        std::unique_lock<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);

        if(it != cache_map.end()) {
            if(!it->second->second.is_expired()) {
                lru_list.splice(
                    lru_list.begin(),
                    lru_list,
                    it->second
                );

                value = it->second->second.value;
                it->second->second.timestamp =
                    Aetherium::Sys::timestamp_ms();

                stats.hits++;
                return true;
            }
            else {
                stats.current_size_bytes -=
                    it->second->second.size_bytes;

                stats.current_item_count--;
                stats.evictions++;
                stats.expired_evictions++;

                lru_list.erase(it->second);
                cache_map.erase(it);

                stats.misses++;
                return false;
            }
        }

        stats.misses++;
        return false;
    }

    void remove(const Key& key) override {
        std::unique_lock<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);

        if(it != cache_map.end()) {
            stats.current_size_bytes -=
                it->second->second.size_bytes;
            stats.current_item_count--;

            lru_list.erase(it->second);
            cache_map.erase(it);
        }
    }

    bool contains(const Key& key) const override {
        std::unique_lock<std::mutex> lock(cache_mutex);
        auto it = cache_map.find(key);

        return it != cache_map.end() ?
            !it->second->second.is_expired() : false;
    }

    void clear() override {
        std::unique_lock<std::mutex> lock(cache_mutex);

        cache_map.clear();
        lru_list.clear();
        stats.reset();
    }

    CacheStats get_stats() const override {
        std::unique_lock<std::mutex> lock(cache_mutex);
        return stats;
    }

    void start_thread_cleanup() override {
        if(!cleanup_thread.joinable()) {
            this->stop_cleanup_thread_flag.store(false);
            this->cleanup_thread = std::thread(
                &LruCache<Key, Value>::cleanup_task,
                this
            );
        }
    }

    void stop_thread_cleanup() override {
        if(cleanup_thread.joinable()) {
            stop_cleanup_thread_flag.store(true);
            cleanup_thread.join();
        }
    }
};

template <typename Key, typename Value>
class CacheManager {
private:
    static std::map<
        std::string,
        std::shared_ptr<ICache<Key, Value>>
    > caches;
    static std::mutex manager_mutex;

    CacheManager() = default;

public:
    static std::shared_ptr<ICache<Key, Value>> get_cache(
        const std::string& name = "default_cache",
        size_t max_s_bytes = CACHE_MAX_SIZE_BYTES,
        size_t max_i = CACHE_MAX_ITEMS
    ) {
        std::unique_lock<std::mutex> lock(manager_mutex);

        if(caches.find(name) == caches.end())
            caches[name] = std::make_shared<
                LruCache<Key, Value>
            >(max_s_bytes, max_i);

        return caches[name];
    }

    static void remove_cache(const std::string& name) {
        std::unique_lock<std::mutex> lock(manager_mutex);

        auto it = caches.find(name);
        if(it != caches.end()) {
            it->second->stop_thread_cleanup();
            caches.erase(it);
        }
    }

    static void clear_all_caches() {
        std::unique_lock<std::mutex> lock(manager_mutex);

        for(auto const& [name, cache] : caches)
            cache->stop_thread_cleanup();
        caches.clear();
    }
};

template <typename Key, typename Value>
std::map<
    std::string,
    std::shared_ptr<ICache<Key, Value>>
> CacheManager<Key, Value>::caches;

template <typename Key, typename Value>
std::mutex CacheManager<Key, Value>::manager_mutex;

}

#endif
