/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Purple.
 *
 * Purple is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Purple is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Purple. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file cache.hpp
 * @brief Provides a generic, thread-safe, and extensible in-memory caching
 * system with LRU (Least Recently Used) eviction and configurable TTL
 * (time-to-live).
 *
 * The caching system is designed for efficient temporary data storage with
 * automatic eviction policies, expiration, and optional priority handling. It
 * includes:
 *
 * - Type-traits for estimating object sizes in memory.
 * - Configurable cache items with TTL, size, and priority.
 * - Statistics collection for monitoring cache efficiency.
 * - A generic `ICache` interface for extensibility.
 * - An `LruCache` implementation with periodic cleanup and eviction.
 * - A `CacheManager` for managing multiple named cache instances globally.
 *
 */
#ifndef PURPLE_SYS_CACHE_HPP
#define PURPLE_SYS_CACHE_HPP

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

#include <purple/sys/time.hpp>

namespace Purple::MemoryCache {

/**
 * @def CACHE_MAX_SIZE_BYTES
 * @brief Default maximum cache size in bytes (10 MB).
 */
#define CACHE_MAX_SIZE_BYTES 1024 * 1024 * 10

/**
 * @def CACHE_MAX_ITEMS
 * @brief Default maximum number of items allowed in a cache (1000).
 */
#define CACHE_MAX_ITEMS 1000

/**
 * @def CACHE_DEFAULT_PRIORITY
 * @brief Default priority value for cache items (0).
 *
 * Higher-priority items are less likely to be evicted when capacity limits are
 * reached.
 */
#define CACHE_DEFAULT_PRIORITY 0

/**
 * @def CACHE_DEFAULT_TTL_SECONDS
 * @brief Default time-to-live (TTL) for cache items in seconds (3600s = 1
 * hour).
 */
#define CACHE_DEFAULT_TTL_SECONDS 3600

/**
 * @def CACHE_CLEANUP_INTERVAL_MS
 * @brief Interval in milliseconds at which the background cleanup thread
 *        checks for expired items (default 5000 ms).
 */
#define CACHE_CLEANUP_INTERVAL_MS 5000

/**
 * @brief Type trait to determine if a type is a container (has size() and
 * value_type).
 * @tparam T The type to evaluate.
 */
template <typename T, typename = void> struct is_container : std::false_type {};

template <typename T>
struct is_container<
    T, std::void_t<decltype(std::declval<T>().size()), typename T::value_type>>
    : std::true_type {};

/**
 * @brief Estimates memory size of a generic type (non-container, non-string).
 * @tparam T The type of the value.
 * @param value Value to evaluate.
 * @return Estimated size in bytes.
 */
template <typename T,
          typename std::enable_if_t<!std::is_same_v<T, std::string> &&
                                        !is_container<T>::value &&
                                        !std::is_same_v<T, const char *>,
                                    int> = 0>
size_t get_estimated_size(const T &value) {
  return sizeof(T);
}

/**
 * @brief Estimates memory size for std::string.
 * @param value String value.
 * @return Length of the string plus null-terminator.
 */
template <typename std::enable_if_t<std::is_same_v<std::string, std::string>,
                                    int> = 0>
size_t get_estimated_size(const std::string &value) {
  return value.length() + 1;
}

/**
 * @brief Estimates memory size for C-style strings and arrays.
 * @param value Pointer or array.
 * @return Length of the string plus null-terminator.
 */
template <typename T, typename std::enable_if_t<
                          std::is_same_v<T, const char *> ||
                              std::is_array_v<std::remove_reference_t<T>>,
                          int> = 0>
size_t get_estimated_size(const T &value) {
  return strlen(value) + 1;
}

/**
 * @brief Estimates memory size for container types.
 * @tparam T Container type (e.g., vector, list).
 * @param value Container instance.
 * @return Approximate size of container data.
 */
template <typename T,
          typename std::enable_if_t<is_container<T>::value, int> = 0>
size_t get_estimated_size(const T &value) {
  return value.size() * sizeof(typename T::value_type) + sizeof(T);
}

/**
 * @brief Represents a single cache entry with metadata.
 *
 * Each cache item stores:
 * - The value.
 * - A timestamp of insertion.
 * - A TTL (time-to-live).
 * - Estimated memory size.
 * - An optional eviction priority.
 *
 * @tparam T Type of the stored value.
 */
template <typename T> struct CacheItem {
  T value;             ///< Stored value.
  long long timestamp; ///< Time of insertion in milliseconds.
  long long ttl;       ///< Time-to-live duration in milliseconds.

  size_t size_bytes; ///< Estimated memory size of the item.
  int priority;      ///< Priority level (higher = less likely to be evicted).

  /**
   * @brief Constructs a cache item.
   * @param val Value to store.
   * @param expiry_ms Time-to-live in milliseconds.
   * @param s_bytes Estimated size of the value in bytes.
   * @param p Priority (default = CACHE_DEFAULT_PRIORITY).
   */
  CacheItem(const T &val, long long expiry_ms, size_t s_bytes,
            int p = CACHE_DEFAULT_PRIORITY)
      : value(val), timestamp(Purple::Sys::timestamp_ms()), ttl(expiry_ms),
        size_bytes(s_bytes), priority(p) {}

  /**
   * @brief Checks if the item has expired.
   * @return True if expired, false otherwise.
   */
  bool is_expired() const {
    return (Purple::Sys::timestamp_ms() - timestamp) > ttl;
  }
};

/**
 * @brief Collects statistics for a cache instance.
 */
struct CacheStats {
  long long hits;               ///< Number of successful retrievals.
  long long misses;             ///< Number of failed retrievals.
  long long current_size_bytes; ///< Current total size of items in bytes.
  long long current_item_count; ///< Current number of items stored.
  long long evictions;          ///< Total number of evicted items.
  long long expired_evictions;  ///< Evictions due to expiration.
  long long capacity_evictions; ///< Evictions due to capacity limits.
  long long priority_evictions; ///< Evictions due to low priority.

  /**
   * @brief Constructs and initializes counters.
   */
  CacheStats()
      : hits(0), misses(0), current_size_bytes(0), current_item_count(0),
        evictions(0), expired_evictions(0), capacity_evictions(0),
        priority_evictions(0) {}

  /**
   * @brief Resets all counters and usage statistics.
   */
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

/**
 * @brief Interface for generic cache implementations.
 * @tparam Key Type of the cache key.
 * @tparam Value Type of the stored value.
 */
template <typename Key, typename Value> class ICache {
public:
  virtual ~ICache() = default;

  /**
   * @brief Stores a value in the cache.
   * @param key The cache key.
   * @param value The value to store.
   * @param ttl_seconds Time-to-live in seconds (default =
   * CACHE_DEFAULT_TTL_SECONDS).
   * @param size_bytes Estimated size in bytes (auto-calculated if 0).
   * @param priority Priority of the item.
   */
  virtual void put(const Key &key, const Value &value,
                   long long ttl_seconds = CACHE_DEFAULT_TTL_SECONDS,
                   size_t size_bytes = 0,
                   int priority = CACHE_DEFAULT_PRIORITY) = 0;

  /**
   * @brief Retrieves a value from the cache.
   * @param key Cache key.
   * @param value Output reference to store retrieved value.
   * @return True if found and valid, false if missing or expired.
   */
  virtual bool get(const Key &key, Value &value) = 0;

  /**
   * @brief Removes an item from the cache by key.
   * @param key The cache key.
   */
  virtual void remove(const Key &key) = 0;

  /**
   * @brief Checks whether a valid item exists in the cache.
   * @param key The cache key.
   * @return True if present and not expired.
   */
  virtual bool contains(const Key &key) const = 0;

  /**
   * @brief Clears all entries from the cache.
   */
  virtual void clear() = 0;

  /**
   * @brief Retrieves statistics of the cache.
   * @return CacheStats with current metrics.
   */
  virtual CacheStats get_stats() const = 0;

  /**
   * @brief Starts the cleanup background thread.
   *
   * The thread periodically scans and removes expired items.
   */
  virtual void start_thread_cleanup() = 0;

  /**
   * @brief Stops the cleanup background thread.
   */
  virtual void stop_thread_cleanup() = 0;
};

/**
 * @brief LRU (Least Recently Used) cache implementation with TTL and
 * priority-based eviction.
 *
 * This cache:
 * - Automatically removes expired items.
 * - Evicts least recently used and lowest priority items when capacity is
 * exceeded.
 * - Provides a background cleanup thread for continuous maintenance.
 *
 * @tparam Key Cache key type.
 * @tparam Value Cache value type.
 */
template <typename Key, typename Value>
class LruCache : public ICache<Key, Value> {
private:
  /**
   * @brief Map of keys to iterators pointing into the LRU list for O(1) access.
   */
  std::map<Key, typename std::list<std::pair<Key, CacheItem<Value>>>::iterator>
      cache_map;

  /**
   * @brief Doubly linked list of cached items ordered by recency.
   *
   * - Front = most recently used.
   * - Back = least recently used.
   */
  std::list<std::pair<Key, CacheItem<Value>>> lru_list;

  /**
   * @brief Cache performance and usage statistics.
   */
  CacheStats stats;

  /**
   * @brief Mutex protecting all cache operations.
   */
  mutable std::mutex cache_mutex;

  /**
   * @brief Maximum total size of items allowed in the cache (bytes).
   */
  size_t max_size_bytes;

  /**
   * @brief Maximum number of items allowed in the cache.
   */
  size_t max_items;

  /**
   * @brief Flag controlling the background cleanup thread.
   */
  std::atomic<bool> stop_cleanup_thread_flag;

  /**
   * @brief Background thread that performs periodic cleanup of expired items.
   */
  std::thread cleanup_thread;

  /**
   * @brief Evicts one item from the cache based on priority and recency.
   *
   * - Lowest-priority items are preferred victims.
   * - Among items of equal priority, the least recently used is evicted.
   */
  void evict_one() {
    if (lru_list.empty())
      return;

    auto victim_it = lru_list.end();
    int min_priority = std::numeric_limits<int>::max();

    for (auto it = lru_list.rbegin(); it != lru_list.rend(); ++it)
      if (it->second.priority < min_priority) {
        min_priority = it->second.priority;
        victim_it = std::next(it).base();
      } else if (it->second.priority == min_priority)
        victim_it = std::next(it).base();

    if (victim_it != lru_list.end()) {
      stats.current_size_bytes -= victim_it->second.size_bytes;
      stats.current_item_count--;

      stats.evictions++;
      stats.capacity_evictions++;
      stats.priority_evictions++;

      cache_map.erase(victim_it->first);
      lru_list.erase(victim_it);
    }
  }

  /**
   * @brief Background task executed in cleanup thread.
   *
   * This task:
   * - Sleeps for @ref CACHE_CLEANUP_INTERVAL_MS between cycles.
   * - Removes expired items from the cache.
   * - Enforces capacity limits by evicting items if necessary.
   */
  void cleanup_task() {
    while (!stop_cleanup_thread_flag.load()) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(CACHE_CLEANUP_INTERVAL_MS));

      if (stop_cleanup_thread_flag.load())
        break;
      std::unique_lock<std::mutex> lock(cache_mutex);

      auto it = lru_list.begin();
      while (it != lru_list.end()) {
        if (it->second.is_expired()) {
          stats.current_size_bytes -= it->second.size_bytes;

          stats.current_item_count--;
          stats.evictions++;
          stats.expired_evictions++;

          cache_map.erase(it->first);
          it = lru_list.erase(it);
        } else
          ++it;
      }

      while (static_cast<size_t>(stats.current_size_bytes) > max_size_bytes ||
             static_cast<size_t>(stats.current_item_count) > max_items) {
        if (lru_list.empty())
          break;
        evict_one();
      }
    }
  }

public:
  /**
   * @brief Constructs an LRU cache with specified capacity limits.
   * @param max_s_bytes Maximum size in bytes.
   * @param max_i Maximum number of items.
   *
   * A background cleanup thread is automatically started upon construction.
   */
  LruCache(size_t max_s_bytes, size_t max_i)
      : cache_map({}), lru_list({}), stats(), cache_mutex(),
        max_size_bytes(max_s_bytes), max_items(max_i),
        stop_cleanup_thread_flag(false), cleanup_thread() {
    max_s_bytes = CACHE_MAX_SIZE_BYTES;
    max_i = CACHE_MAX_ITEMS;

    this->start_thread_cleanup();
  }

  /**
   * @brief Destructor. Ensures cleanup thread is stopped before destruction.
   */
  ~LruCache() override { this->stop_thread_cleanup(); }

  /**
   * @copydoc ICache::put()
   *
   * If the key already exists, the old item is replaced.
   * Items may trigger eviction if capacity is exceeded after insertion.
   */
  void put(const Key &key, const Value &value,
           long long ttl_seconds = CACHE_DEFAULT_TTL_SECONDS,
           size_t initial_size_bytes = 0,
           int priority = CACHE_DEFAULT_PRIORITY) override {
    std::unique_lock<std::mutex> lock(cache_mutex);
    long long ttl_ms = ttl_seconds * 1000;

    size_t actual_size_bytes = initial_size_bytes;
    if (actual_size_bytes == 0)
      actual_size_bytes = get_estimated_size(value);

    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
      lru_list.erase(it->second);
      stats.current_size_bytes -= it->second->second.size_bytes;
    }

    lru_list.emplace_front(
        key, CacheItem<Value>(value, ttl_ms, actual_size_bytes, priority));

    cache_map[key] = lru_list.begin();
    stats.current_size_bytes += actual_size_bytes;
    stats.current_item_count++;

    while (static_cast<size_t>(stats.current_size_bytes) > max_size_bytes ||
           static_cast<size_t>(stats.current_item_count) > max_items) {
      if (lru_list.empty())
        break;

      this->evict_one();
    }
  }

  /**
   * @copydoc ICache::get()
   *
   * On hit, the accessed item is moved to the front of the LRU list
   * (most recently used).
   */
  bool get(const Key &key, Value &value) override {
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(key);

    if (it != cache_map.end()) {
      if (!it->second->second.is_expired()) {
        lru_list.splice(lru_list.begin(), lru_list, it->second);

        value = it->second->second.value;
        it->second->second.timestamp = Purple::Sys::timestamp_ms();

        stats.hits++;
        return true;
      } else {
        stats.current_size_bytes -= it->second->second.size_bytes;

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

  /**
   * @copydoc ICache::remove()
   *
   * If the key exists, removes it and updates statistics.
   */
  void remove(const Key &key) override {
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(key);

    if (it != cache_map.end()) {
      stats.current_size_bytes -= it->second->second.size_bytes;
      stats.current_item_count--;

      lru_list.erase(it->second);
      cache_map.erase(it);
    }
  }

  /**
   * @copydoc ICache::contains()
   */
  bool contains(const Key &key) const override {
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(key);

    return it != cache_map.end() ? !it->second->second.is_expired() : false;
  }

  /**
   * @copydoc ICache::clear()
   *
   * Clears all items and resets statistics.
   */
  void clear() override {
    std::unique_lock<std::mutex> lock(cache_mutex);

    cache_map.clear();
    lru_list.clear();
    stats.reset();
  }

  /**
   * @copydoc ICache::get_stats()
   */
  CacheStats get_stats() const override {
    std::unique_lock<std::mutex> lock(cache_mutex);
    return stats;
  }

  /**
   * @copydoc ICache::start_thread_cleanup()
   *
   * If already running, this call has no effect.
   */
  void start_thread_cleanup() override {
    if (!cleanup_thread.joinable()) {
      this->stop_cleanup_thread_flag.store(false);
      this->cleanup_thread =
          std::thread(&LruCache<Key, Value>::cleanup_task, this);
    }
  }

  /**
   * @copydoc ICache::stop_thread_cleanup()
   *
   * Joins the cleanup thread safely before returning.
   */
  void stop_thread_cleanup() override {
    if (cleanup_thread.joinable()) {
      stop_cleanup_thread_flag.store(true);
      cleanup_thread.join();
    }
  }
};

/**
 * @brief Global manager for handling multiple named cache instances.
 *
 * Provides access to reusable shared caches, identified by string names.
 * Automatically creates caches on-demand and manages cleanup threads.
 *
 * @tparam Key Cache key type.
 * @tparam Value Cache value type.
 */
template <typename Key, typename Value> class CacheManager {
private:
  /**
   * @brief Registry of named caches.
   */
  static std::map<std::string, std::shared_ptr<ICache<Key, Value>>> caches;

  /**
   * @brief Mutex protecting the global cache registry.
   */
  static std::mutex manager_mutex;

  /// @brief Disabled constructor (static-only class).
  CacheManager() = default;

public:
  /**
   * @brief Retrieves or creates a named cache.
   * @param name Name of the cache (default = "default_cache").
   * @param max_s_bytes Maximum size in bytes (used only if creating new cache).
   * @param max_i Maximum number of items (used only if creating new cache).
   * @return Shared pointer to the cache instance.
   */
  static std::shared_ptr<ICache<Key, Value>>
  get_cache(const std::string &name = "default_cache",
            size_t max_s_bytes = CACHE_MAX_SIZE_BYTES,
            size_t max_i = CACHE_MAX_ITEMS) {
    std::unique_lock<std::mutex> lock(manager_mutex);

    if (caches.find(name) == caches.end())
      caches[name] = std::make_shared<LruCache<Key, Value>>(max_s_bytes, max_i);

    return caches[name];
  }

  /**
   * @brief Removes a named cache and stops its cleanup thread.
   * @param name Name of the cache to remove.
   */
  static void remove_cache(const std::string &name) {
    std::unique_lock<std::mutex> lock(manager_mutex);

    auto it = caches.find(name);
    if (it != caches.end()) {
      it->second->stop_thread_cleanup();
      caches.erase(it);
    }
  }

  /**
   * @brief Clears and removes all caches.
   *
   * Each cache’s cleanup thread is stopped before removal.
   */
  static void clear_all_caches() {
    std::unique_lock<std::mutex> lock(manager_mutex);

    for (auto const &[name, cache] : caches)
      cache->stop_thread_cleanup();
    caches.clear();
  }
};

/**
 * @brief Static container holding all named cache instances managed by
 *        CacheManager.
 *
 * This map associates a string identifier (cache name) with a shared pointer
 * to an `ICache` instance (usually an `LruCache`). It is used internally by
 * CacheManager to provide access to reusable cache objects across different
 * parts of the application.
 *
 * @tparam Key   Cache key type.
 * @tparam Value Cache value type.
 */
template <typename Key, typename Value>
std::map<std::string, std::shared_ptr<ICache<Key, Value>>>
    CacheManager<Key, Value>::caches;

/**
 * @brief Global mutex protecting CacheManager::caches from concurrent access.
 *
 * This ensures thread-safe creation, retrieval, and removal of shared cache
 * instances. All CacheManager operations acquire a lock on this mutex before
 * modifying or accessing the internal `caches` map.
 *
 * @tparam Key   Cache key type.
 * @tparam Value Cache value type.
 */
template <typename Key, typename Value>
std::mutex CacheManager<Key, Value>::manager_mutex;

} // namespace Purple::MemoryCache

#endif
