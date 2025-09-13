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
 * @file job.hpp
 * @author Nathanne Isip
 * @brief Defines a single scheduled job for the Purple Cron system.
 *
 * A `CronJob` represents one task scheduled according to a cron-like
 * expression. It stores metadata (ID, description), scheduling rules,
 * the associated callback function, and its next execution time.
 *
 */
#ifndef PURPLE_CRON_JOB_HPP
#define PURPLE_CRON_JOB_HPP

#include <purple/cron/schedule.hpp>
#include <purple/cron/timepoint.hpp>

#include <functional>
#include <string>

namespace Purple::Cron {

/**
 * @class CronJob
 * @brief Represents a single cron-scheduled task.
 *
 * A `CronJob` encapsulates:
 * - A unique identifier (`id`)
 * - A human-readable description
 * - A scheduling rule (`CronSchedule`)
 * - A callback function to execute
 * - The next calculated runtime (`TimePoint`)
 * - An `enabled` flag to toggle execution
 *
 * Jobs are created with a cron expression string (e.g. `"*\/5 * * * *"`
 * for every 5 minutes). After execution, the `update_next_runtime()`
 * method is used to reschedule the next occurrence.
 */
class CronJob {
public:
  /**
   * @brief Unique identifier for the job.
   *
   * Typically a short string used to reference or manage this job
   * within a larger cron scheduler.
   */
  std::string id;

  /**
   * @brief Human-readable description of the job.
   *
   * Useful for logging, debugging, or monitoring dashboards.
   */
  std::string description;

  /**
   * @brief Parsed schedule for the job, based on a cron expression.
   *
   * Defines when the job is eligible to run.
   */
  CronSchedule schedule;

  /**
   * @brief The function to execute when the job triggers.
   */
  std::function<void()> callback;

  /**
   * @brief The next scheduled runtime of this job.
   *
   * This is calculated automatically from the schedule at
   * construction and updated via `update_next_runtime()`.
   */
  TimePoint next_runtime;

  /**
   * @brief Flag indicating whether the job is active.
   *
   * If `false`, the job will not be executed by the scheduler.
   */
  bool enabled;

  /**
   * @brief Constructs a new cron job.
   *
   * @param id Unique identifier string for the job.
   * @param desc Human-readable description of the job.
   * @param cron_str A cron expression string defining the schedule.
   * @param cb Callback function to be executed when triggered.
   *
   * @throw std::invalid_argument if the cron expression is invalid.
   */
  CronJob(const std::string &id, const std::string &desc,
          const std::string &cron_str, std::function<void()> cb);

  /**
   * @brief Updates the `next_runtime` to the following occurrence.
   *
   * Should be called after the job executes, to reschedule its
   * next execution time.
   *
   * Internally, it moves the reference time forward by 1 second
   * past the last runtime to avoid duplicate scheduling.
   */
  void update_next_runtime();
};

} // namespace Purple::Cron

#endif
