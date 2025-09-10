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

/**
 * @file tasklet.hpp
 * @author Nathanne Isip <nathanneisip@gmail.com>
 * @brief Provides tasklet-based lightweight concurrency management.
 *
 * This header defines the tasklet runtime system, including task scheduling,
 * worker thread management, and panic handling. It enables parallel execution
 * of lightweight tasks using a thread pool managed by `TaskletManager`.
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

/**
 * @class TaskletPanicException
 * @brief Exception type thrown when a fatal tasklet-related error occurs.
 *
 * This exception signals unrecoverable runtime errors in the tasklet system,
 * such as attempting to schedule tasks without an initialized manager.
 */
class TaskletPanicException : public std::runtime_error {
public:
  /**
   * @brief Constructs a new panic exception with a descriptive message.
   * @param message The error message explaining the cause of the panic.
   */
  explicit TaskletPanicException(const std::string &message);
};

/**
 * @brief Triggers a panic in the tasklet system.
 *
 * This helper function throws a `TaskletPanicException` with the provided message.
 * It is used internally when the tasklet runtime encounters unrecoverable errors.
 *
 * @param message Descriptive error message for the panic.
 * @throws TaskletPanicException Always thrown when invoked.
 */
void tasklet_panic(const std::string &message);

/**
 * @class TaskletManager
 * @brief Manages task execution and worker threads for the tasklet runtime.
 *
 * The `TaskletManager` maintains a pool of worker threads that execute lightweight
 * tasks (`std::function<void()>`) submitted to the system. It supports both task
 * scheduling and synchronization for task completion.
 *
 * - Tasks are scheduled via the `go()` method.
 * - Worker threads continuously pull tasks from the queue.
 * - `wait_for_completion()` blocks until all pending tasks have finished execution.
 *
 * The manager can be cleanly destroyed, ensuring that all worker threads are joined.
 */
class TaskletManager {
private:
  std::mutex queue_mutex;                         ///< Mutex protecting the task queue.
  std::condition_variable condition;              ///< Condition variable for task availability.

  std::vector<std::thread> workers;               ///< Pool of worker threads.
  std::queue<std::function<void()>> tasks;        ///< Queue of scheduled tasks.

  std::atomic<int> active_tasks_count;            ///< Counter of currently running tasks.
  std::condition_variable tasks_completion_cv;    ///< Condition variable for task completion.

  bool stop_threads;                              ///< Flag to signal worker shutdown.

public:
  /**
   * @brief Constructs a new tasklet manager with a given number of worker threads.
   * @param num_threads Number of threads to spawn in the pool.
   */
  TaskletManager(size_t num_threads);

  /**
   * @brief Destructor. Cleans up all worker threads and pending tasks.
   *
   * Ensures a graceful shutdown of the tasklet runtime by:
   * - Signaling all threads to stop.
   * - Joining all worker threads.
   * - Clearing any remaining tasks.
   */
  ~TaskletManager();

  /**
   * @brief Schedules a task for execution.
   *
   * The task is pushed into the internal queue and executed asynchronously
   * by one of the worker threads.
   *
   * @param task Function object representing the task to execute.
   */
  void go(std::function<void()> task);

  /**
   * @brief Waits until all scheduled tasks have completed execution.
   *
   * This call blocks until the `active_tasks_count` reaches zero, ensuring
   * that the system is idle before proceeding.
   */
  void wait_for_completion();
};

/**
 * @brief Convenience function to schedule a task with a tasklet manager.
 *
 * This is a wrapper around `TaskletManager::go()` that checks whether the
 * manager is valid before scheduling. If the manager is `nullptr`, a panic
 * is triggered.
 *
 * @tparam T Callable type (defaults to `std::function<void()>`).
 * @param manager Pointer to the tasklet manager instance.
 * @param func Task function to execute.
 *
 * @throws TaskletPanicException If the manager is `nullptr`.
 */
template <typename T = std::function<void()>>
void go(TaskletManager *manager, T func) {
  if (manager)
    manager->go(std::move(func));
  else
    tasklet_panic("TaskletManager not initialized");
}

} // namespace Netlet::Concurrent

#endif
