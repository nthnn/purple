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

#ifndef PURPLE_CRON_SCHEDULE_HPP
#define PURPLE_CRON_SCHEDULE_HPP

#include <purple/concurrent/tasklet.hpp>
#include <purple/cron/parser.hpp>
#include <purple/cron/timepoint.hpp>

#include <mutex>
#include <string>
#include <thread>

namespace Purple::Cron {

class CronJob;

/**
 * @brief Represents a parsed cron schedule string and provides runtime
 * evaluation.
 *
 * A `CronSchedule` encapsulates a 5-field cron expression (minute, hour, day of
 * month, month, day of week). It uses `CronParser` internally to expand the
 * cron string into explicit sets of allowed values for each field.
 *
 */
class CronSchedule {
private:
  std::string cron_str;           ///< Original cron string representation.
  CronParsedFields parsed_fields; ///< Parsed cron fields.

public:
  /**
   * @brief Constructs a schedule from a cron expression string.
   *
   * @param str A standard 5-field cron expression (minute, hour, day of month,
   *            month, day of week).
   *
   * @throws std::invalid_argument if the string cannot be parsed.
   */
  CronSchedule(const std::string &str);

  /**
   * @brief Computes the next runtime that satisfies this schedule.
   *
   * Starting from the given `start_tm`, this function searches forward until it
   * finds the next valid time that matches all cron fields.
   *
   * @param start_tm The time point from which to begin searching.
   * @return The next valid `TimePoint` when the cron expression should run.
   *
   * @throws std::runtime_error if no matching time is found within a reasonable
   * range.
   */
  TimePoint get_next_runtime(TimePoint start_tm) const;

  /**
   * @brief Gets the original cron string used to construct this schedule.
   *
   * @return A reference to the stored cron string.
   */
  const std::string &get_cron_string() const;
};

/**
 * @brief A thread-based scheduler that manages and executes cron jobs.
 *
 * `CronScheduler` maintains a collection of `CronJob` objects, each associated
 * with a cron expression and a callback. It continuously checks for due jobs
 * in a background thread and executes them using a `TaskletManager`.
 *
 * Features:
 * - Thread-safe job management
 * - Concurrent execution of job callbacks
 * - Ability to enable/disable or remove jobs dynamically
 *
 */
class CronScheduler {
private:
  std::map<std::string, CronJob> jobs; ///< Registered jobs keyed by ID.
  mutable std::mutex jobs_mtx;         ///< Mutex to protect job map.

  std::thread schd_thread; ///< Background thread for scheduling loop.
  bool running;            ///< Indicates if the scheduler is active.

  Purple::Concurrent::TaskletManager
      task_manager; ///< Manages job execution threads.

  /**
   * @brief Internal loop that checks jobs and executes due tasks.
   *
   * Runs in a dedicated thread created when `start()` is called.
   */
  void run();

public:
  /**
   * @brief Constructs a scheduler with an optional number of worker threads.
   *
   * @param working_threads Number of worker threads for concurrent job
   * execution. Defaults to 0 (synchronous execution).
   */
  CronScheduler(size_t working_threads = 0);

  /**
   * @brief Destructor. Ensures the scheduler is stopped before destruction.
   */
  ~CronScheduler();

  /**
   * @brief Starts the scheduling loop in a background thread.
   *
   * Safe to call only once. Subsequent calls without calling `stop()` have no
   * effect.
   */
  void start();

  /**
   * @brief Stops the scheduling loop and waits for all jobs to complete.
   *
   * Blocks until the background thread exits and all running jobs finish.
   */
  void stop();

  /**
   * @brief Adds a new cron job to the scheduler.
   *
   * @param id Unique identifier for the job.
   * @param description Human-readable description of the job.
   * @param cron_string Standard cron expression controlling the job’s schedule.
   * @param callback The function to execute when the job triggers.
   *
   * @return `true` if the job was successfully added, `false` if the ID
   *         already exists or the cron expression is invalid.
   */
  bool add_job(const std::string &id, const std::string &description,
               const std::string &cron_string, std::function<void()> callback);

  /**
   * @brief Removes a job from the scheduler.
   *
   * @param id The identifier of the job to remove.
   * @return `true` if the job was found and removed, otherwise `false`.
   */
  bool remove_job(const std::string &id);

  /**
   * @brief Enables or disables a job.
   *
   * @param id The identifier of the job.
   * @param enabled `true` to enable, `false` to disable.
   * @return `true` if the job exists and its state was updated, otherwise
   * `false`.
   */
  bool set_job_enabled(const std::string &id, bool enabled);

  /**
   * @brief Retrieves a snapshot of all registered jobs.
   *
   * @return A vector of `CronJob` objects representing the current state.
   */
  std::vector<CronJob> get_all_jobs() const;
};

} // namespace Purple::Cron

#endif
