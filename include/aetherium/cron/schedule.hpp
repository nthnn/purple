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

#ifndef AETHERIUM_CRON_SCHEDULE_HPP
#define AETHERIUM_CRON_SCHEDULE_HPP

#include <aetherium/concurrent/tasklet.hpp>
#include <aetherium/cron/parser.hpp>
#include <aetherium/cron/timepoint.hpp>

#include <mutex>
#include <string>
#include <thread>

namespace Aetherium::Cron {

class CronJob;

class CronSchedule {
private:
  std::string cron_str;
  CronParsedFields parsed_fields;

public:
  CronSchedule(const std::string &str);

  TimePoint get_next_runtime(TimePoint start_tm) const;
  const std::string &get_cron_string() const;
};

class CronScheduler {
private:
  std::map<std::string, CronJob> jobs;
  mutable std::mutex jobs_mtx;

  std::thread schd_thread;
  bool running;

  Aetherium::Concurrent::TaskletManager task_manager;

  void run();

public:
  CronScheduler(size_t working_threads = 0);
  ~CronScheduler();

  void start();
  void stop();

  bool add_job(const std::string &id, const std::string &description,
               const std::string &cron_string, std::function<void()> callback);
  bool remove_job(const std::string &id);

  bool set_job_enabled(const std::string &id, bool enabled);

  std::vector<CronJob> get_all_jobs() const;
};

} // namespace Aetherium::Cron

#endif
