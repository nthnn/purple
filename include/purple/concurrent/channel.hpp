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
 * @file channel.hpp
 * @author Nathanne Isip <nathanneisip@gmail.com
 * @brief Provides a concurrent channel implementation for inter-thread
 * communication.
 *
 * This header defines the `Purple::Concurrent::Channel` class template, which
 * implements a thread-safe, blocking and non-blocking channel for communication
 * between threads. It supports both bounded and unbounded (synchronous)
 * channels, offering synchronization via condition variables and atomic
 * counters.
 */
#ifndef PURPLE_CONCURRENT_CHANNEL_HPP
#define PURPLE_CONCURRENT_CHANNEL_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

namespace Purple::Concurrent {

/**
 * @class Channel
 * @brief A thread-safe communication channel between concurrent tasks.
 *
 * The `Channel<T>` class provides a Go-style channel abstraction for sending
 * and receiving messages between threads. It supports both bounded and
 * unbounded capacities:
 *
 * - **Bounded mode**: When `capacity > 0`, the channel acts as a buffered
 * channel with a maximum capacity. Sending blocks if the buffer is full until
 * space is available.
 * - **Unbounded (synchronous) mode**: When `capacity == 0`, each send operation
 * must synchronize with a waiting receiver, effectively creating a rendezvous
 * channel.
 *
 * Once closed, no further messages may be sent, but pending messages can still
 * be received.
 *
 * @tparam T Type of elements that can be transmitted through the channel.
 */
template <typename T> class Channel {
private:
  bool closed;     ///< Indicates whether the channel is closed.
  size_t capacity; ///< Maximum number of buffered items (0 = unbounded
                   ///< synchronous channel).

  std::mutex mtx;     ///< Mutex protecting channel state.
  std::deque<T> data; ///< Queue holding buffered messages.
  std::condition_variable
      send_cond_var; ///< Condition variable for send-side synchronization.
  std::condition_variable receive_cond_var; ///< Condition variable for
                                            ///< receive-side synchronization.
  std::condition_variable
      send_ack; ///< Condition variable to acknowledge synchronous sends.
  std::atomic<int> get_wait_count{
      0}; ///< Number of receivers currently waiting.

public:
  /**
   * @brief Constructs a new channel with an optional buffer capacity.
   * @param cap Buffer capacity. If `0`, the channel acts as synchronous
   * (unbuffered).
   */
  explicit Channel(size_t cap = 0)
      : closed(false), capacity(cap), mtx(), data(), send_cond_var(),
        receive_cond_var(), send_ack(), get_wait_count{0} {}

  /**
   * @brief Sends a value into the channel, blocking if necessary.
   *
   * - If the channel is bounded and full, the sender blocks until space is
   * available.
   * - If the channel is synchronous (capacity == 0), the sender blocks until a
   * receiver is waiting.
   *
   * @param value The value to send into the channel.
   * @throws std::runtime_error If the channel is closed.
   */
  void send(const T &value) {
    std::unique_lock<std::mutex> lock(mtx);
    if (closed)
      throw std::runtime_error("send on closed channel");

    send_cond_var.wait(lock, [this] {
      return closed || data.size() < capacity ||
             (capacity == 0 && get_wait_count > 0);
    });

    if (closed)
      throw std::runtime_error("send on closed channel");

    data.push_back(value);
    if (capacity == 0) {
      receive_cond_var.notify_one();
      send_ack.wait(lock, [this] { return closed || data.empty(); });

      if (closed && !data.empty())
        throw std::runtime_error(
            "send on closed channel (ack failed due to close)");
    } else
      receive_cond_var.notify_one();
  }

  /**
   * @brief Receives a value from the channel, blocking if necessary.
   *
   * If the channel is closed and no more messages remain, returns `{T{},
   * false}`.
   *
   * @return A pair consisting of the received value and a boolean indicating
   * success.
   *         - If `true`, the value is valid.
   *         - If `false`, the channel has been closed and no more messages are
   * available.
   */
  std::pair<T, bool> receive() {
    std::unique_lock<std::mutex> lock(mtx);
    if (capacity == 0) {
      get_wait_count++;
      send_cond_var.notify_one();
    }

    receive_cond_var.wait(lock, [this] { return closed || !data.empty(); });

    if (!data.empty()) {
      T value = data.front();
      data.pop_front();

      if (capacity == 0) {
        get_wait_count--;
        send_ack.notify_one();
      }

      send_cond_var.notify_one();
      return {value, true};
    }

    if (capacity == 0)
      get_wait_count--;
    return {T{}, false};
  }

  /**
   * @brief Attempts to send a value without blocking.
   *
   * - If space is available (bounded mode) or a receiver is waiting
   * (synchronous mode), the value is sent immediately.
   * - Otherwise, the function returns `false` without sending.
   *
   * @param value The value to send.
   * @return `true` if the value was successfully sent, `false` otherwise.
   */
  bool try_send(const T &value) {
    std::unique_lock<std::mutex> lock(mtx);
    if (closed)
      return false;

    if (capacity == 0) {
      if (get_wait_count > 0) {
        data.push_back(value);
        receive_cond_var.notify_one();
        send_ack.wait(lock, [this] { return closed || data.empty(); });

        return !(closed && !data.empty());
      }

      return false;
    }

    if (data.size() < capacity) {
      data.push_back(value);
      receive_cond_var.notify_one();

      return true;
    }

    return false;
  }

  /**
   * @brief Attempts to receive a value without blocking.
   *
   * - If a value is available, it is returned and the function returns `true`.
   * - If the channel is empty or closed with no remaining values, returns
   * `false`.
   *
   * @param value Reference to store the received value (if available).
   * @return `true` if a value was successfully received, `false` otherwise.
   */
  bool try_receive(T &value) {
    std::unique_lock<std::mutex> lock(mtx);

    if (!data.empty()) {
      value = data.front();
      data.pop_front();

      if (capacity == 0)
        send_ack.notify_one();
      send_cond_var.notify_one();

      return true;
    } else if (closed)
      return false;

    return false;
  }

  /**
   * @brief Closes the channel.
   *
   * Once closed:
   * - Further send attempts will fail with an exception (or return `false` for
   * try_send).
   * - Receivers may still drain any buffered values.
   * - All waiting senders and receivers are notified and unblocked.
   */
  void close() {
    std::unique_lock<std::mutex> lock(mtx);

    if (closed)
      return;
    closed = true;

    send_cond_var.notify_all();
    receive_cond_var.notify_all();
    send_ack.notify_all();
  }
};

} // namespace Purple::Concurrent

#endif
