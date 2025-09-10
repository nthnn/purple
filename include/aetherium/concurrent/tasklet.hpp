/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Netlet.
 *
 * Netlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Netlet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Netlet. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NETLET_CONCURRENT_TASKLET_HPP
#define NETLET_CONCURRENT_TASKLET_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>

namespace Netlet::Concurrent {

class TaskletPanicException : public std::runtime_error {
public:
  explicit TaskletPanicException(const std::string &message);
};

void tasklet_panic(const std::string &message);

class TaskletManager {
private:
  std::mutex queue_mutex;
  std::condition_variable condition;

  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::atomic<int> active_tasks_count;
  std::condition_variable tasks_completion_cv;

  bool stop_threads;

public:
  TaskletManager(size_t num_threads);
  ~TaskletManager();

  void go(std::function<void()> task);
  void wait_for_completion();
};

template <typename T = std::function<void()>>
void go(TaskletManager *manager, T func) {
  if (manager)
    manager->go(std::move(func));
  else
    tasklet_panic("TaskletManager not initialized");
}

} // namespace Netlet::Concurrent

#endif
