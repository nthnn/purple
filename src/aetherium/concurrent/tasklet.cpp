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

#include <aetherium/concurrent/tasklet.hpp>

#include <iostream>

namespace Aetherium::Concurrent {

TaskletPanicException::TaskletPanicException(
    const std::string& message
) : std::runtime_error(
    "Tasklet Panic: " + message
) { }

void tasklet_panic(
    const std::string& message
) {
    throw TaskletPanicException(message);
}

TaskletManager::TaskletManager(size_t num_threads) :
        queue_mutex(),
        condition(),
        workers(),
        tasks(),
        active_tasks_count(0),
        tasks_completion_cv(),
        stop_threads(false)
{
    if(num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();

        if(num_threads == 0)
            num_threads = 4;
    }

    for(size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while(true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(
                        this->queue_mutex
                    );

                    this->condition.wait(lock, [this] {
                        return !tasks.empty() || this->stop_threads;
                    });

                    if(this->stop_threads && this->tasks.empty())
                        return;

                    task = this->tasks.front();
                    this->tasks.pop();
                }

                try {
                    task();
                }
                catch(const TaskletPanicException& e) {
                    std::cerr << (
                        "Tasklet panicked: " +
                            std::string(e.what())
                    ) << std::endl;
                }
                catch(const std::exception& e) {
                    std::cerr << (
                        "Tasklet unexpected exception: " +
                            std::string(e.what())
                    ) << std::endl;
                }

                this->active_tasks_count.fetch_sub(
                    1,
                    std::memory_order_release
                );
                this->tasks_completion_cv.notify_all();
            }
        });
    }
}

TaskletManager::~TaskletManager() {
    {
        std::unique_lock<std::mutex> lock(
            this->queue_mutex
        );
        this->stop_threads = true;
    }

    this->condition.notify_all();
    for(std::thread& worker : this->workers)
        if(worker.joinable())
            worker.join();
}

void TaskletManager::go(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(
            this->queue_mutex
        );

        this->tasks.push(std::move(task));
        this->active_tasks_count.fetch_add(
            1,
            std::memory_order_release
        );
    }

    this->condition.notify_one();
}

void TaskletManager::wait_for_completion() {
    std::unique_lock<std::mutex> lock(
        this->queue_mutex
    );

    this->tasks_completion_cv.wait(lock, [this] {
        return this->active_tasks_count == 0;
    });
}

}
