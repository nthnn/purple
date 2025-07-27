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

#ifndef AETHERIUM_CONCURRENT_CHANNEL_HPP
#define AETHERIUM_CONCURRENT_CHANNEL_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>

namespace Aetherium::Concurrent {

template<typename T>
class Channel {
private:
    bool closed;
    size_t capacity;

    std::mutex mtx;
    std::deque<T> data;
    std::condition_variable send_cond_var;
    std::condition_variable receive_cond_var;
    std::condition_variable send_ack;
    std::atomic<int> get_wait_count{0};

public:
    explicit Channel(size_t cap = 0) :
        closed(false),
        capacity(cap),
        mtx(),
        data(),
        send_cond_var(),
        receive_cond_var(),
        send_ack(),
        get_wait_count{0} { }

    void send(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        if(closed)
            throw std::runtime_error("send on closed channel");

        send_cond_var.wait(lock, [this] {
            return closed ||
                data.size() < capacity ||
                (capacity == 0 &&
                    get_wait_count > 0);
        });

        if(closed)
            throw std::runtime_error("send on closed channel");

        data.push_back(value);
        if(capacity == 0) {
            receive_cond_var.notify_one();
            send_ack.wait(lock, [this] {
                return closed || data.empty();
            });

            if(closed && !data.empty())
                throw std::runtime_error(
                    "send on closed channel (ack failed due to close)"
                );
        }
        else receive_cond_var.notify_one();
    }

    std::pair<T, bool> receive() {
        std::unique_lock<std::mutex> lock(mtx);
        if(capacity == 0) {
            get_wait_count++;
            send_cond_var.notify_one();
        }

        receive_cond_var.wait(lock, [this] {
            return closed || !data.empty();
        });

        if(!data.empty()) {
            T value = data.front();
            data.pop_front();

            if(capacity == 0) {
                get_wait_count--;
                send_ack.notify_one();
            }

            send_cond_var.notify_one();
            return {value, true};
        }

        if(capacity == 0)
            get_wait_count--;
        return {T{}, false};
    }

    bool try_send(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        if(closed)
            return false;

        if(capacity == 0) {
            if(get_wait_count > 0) {
                data.push_back(value);
                receive_cond_var.notify_one();
                send_ack.wait(lock, [this] {
                    return closed || data.empty();
                });

                return !(closed && !data.empty());
            }

            return false;
        }

        if(data.size() < capacity) {
            data.push_back(value);
            receive_cond_var.notify_one();

            return true;
        }

        return false;
    }

    bool try_receive(T& value) {
        std::unique_lock<std::mutex> lock(mtx);

        if(!data.empty()) {
            value = data.front();
            data.pop_front();

            if(capacity == 0)
                send_ack.notify_one();
            send_cond_var.notify_one();

            return true;
        }
        else if(closed)
            return false;

        return false;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mtx);

        if(closed)
            return;
        closed = true;

        send_cond_var.notify_all();
        receive_cond_var.notify_all();
        send_ack.notify_all();
    }
};

}

#endif
