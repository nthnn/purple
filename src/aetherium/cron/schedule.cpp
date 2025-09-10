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

#include <netlet/cron/job.hpp>
#include <netlet/cron/schedule.hpp>

#include <iostream>
#include <stdexcept>

namespace Netlet::Cron {

CronSchedule::CronSchedule(const std::string &str)
    : cron_str(str), parsed_fields() {
  this->parsed_fields = CronParser::parse(str);
}

TimePoint CronSchedule::get_next_runtime(TimePoint start_tm) const {
  std::time_t tt_start = std::chrono::system_clock ::to_time_t(start_tm);
  std::tm tm_start = *std::gmtime(&tt_start);

  if (tm_start.tm_sec > 0) {
    start_tm += CronSeconds(60 - tm_start.tm_sec);

    tt_start = std::chrono::system_clock::to_time_t(start_tm);
    tm_start = *std::gmtime(&tt_start);
  }

  TimePoint current_check_time = start_tm;
  for (int iteration = 0; iteration < 365 * 24 * 60 * 2; ++iteration) {
    std::time_t tt_current =
        std::chrono::system_clock ::to_time_t(current_check_time);
    std::tm tm_current = *std::gmtime(&tt_current);

    int current_dow = tm_current.tm_wday;
    if (this->parsed_fields.days_of_week.count(7) && current_dow == 0)
      current_dow = 7;

    if (this->parsed_fields.months.find(tm_current.tm_mon + 1) ==
        this->parsed_fields.months.end()) {
      tm_current.tm_mon++;

      if (tm_current.tm_mon > 11) {
        tm_current.tm_mon = 0;
        tm_current.tm_year++;
      }

      tm_current.tm_mday = 1;
      tm_current.tm_hour = 0;
      tm_current.tm_min = 0;
      tm_current.tm_sec = 0;

      current_check_time += CronSeconds(60);
      continue;
    }

    bool day_of_month_match =
        this->parsed_fields.days_of_month.count(tm_current.tm_mday);
    bool day_of_week_match =
        this->parsed_fields.days_of_week.count(current_dow);

    bool is_dom_wildcard = parsed_fields.days_of_month.size() == 31;
    bool is_dow_wildcard = parsed_fields.days_of_week.size() == 7 ||
                           parsed_fields.days_of_week.size() == 8;

    bool day_match_criteria_met = false;
    if (is_dom_wildcard && is_dow_wildcard)
      day_match_criteria_met = true;
    else if (is_dom_wildcard)
      day_match_criteria_met = day_of_week_match;
    else if (is_dow_wildcard)
      day_match_criteria_met = day_of_month_match;
    else
      day_match_criteria_met = day_of_month_match || day_of_week_match;

    if (!day_match_criteria_met) {
      current_check_time += std::chrono::hours(24) -
                            std::chrono::hours(tm_current.tm_hour) -
                            std::chrono::minutes(tm_current.tm_min) -
                            CronSeconds(tm_current.tm_sec);
      continue;
    }

    if (this->parsed_fields.hours.find(tm_current.tm_hour) ==
        this->parsed_fields.hours.end()) {
      current_check_time += std::chrono::hours(1) -
                            std::chrono::minutes(tm_current.tm_min) -
                            CronSeconds(tm_current.tm_sec);

      continue;
    }

    if (this->parsed_fields.minutes.find(tm_current.tm_min) ==
        this->parsed_fields.minutes.end()) {
      current_check_time += CronSeconds(60);
      continue;
    }

    return current_check_time;
  }

  throw std::runtime_error(
      "Could not find next runtime within a reasonable period");
}

const std::string &CronSchedule::get_cron_string() const {
  return this->cron_str;
}

void CronScheduler::run() {
  while (this->running) {
    TimePoint current_tm = now();
    std::vector<std::string> jobs_to_exec;

    {
      std::lock_guard<std::mutex> lock(this->jobs_mtx);

      for (auto &pair : this->jobs) {
        CronJob &job = pair.second;

        if (job.enabled && job.next_runtime <= current_tm)
          jobs_to_exec.push_back(job.id);
      }
    }

    for (const std::string &job_id : jobs_to_exec) {
      this->task_manager.go([this, job_id]() {
        std::function<void()> job_cb;
        bool job_found = false;

        {
          std::lock_guard<std::mutex> lock(this->jobs_mtx);

          auto it = this->jobs.find(job_id);
          if (it != this->jobs.end()) {
            job_cb = it->second.callback;
            job_found = true;
          }
        }

        if (!job_found)
          return;

        try {
          if (job_cb)
            job_cb();
        } catch (const std::exception &e) {
          std::cerr << "Error executing job '" << job_id << "': " << e.what()
                    << std::endl;
        } catch (...) {
          std::cerr << "Unknown error executing job '" << job_id << "'"
                    << std::endl;
        }

        {
          std::lock_guard<std::mutex> lock(this->jobs_mtx);

          auto it = this->jobs.find(job_id);
          if (it != this->jobs.end())
            it->second.update_next_runtime();
        }
      });
    }
    std::this_thread::sleep_for(CronSeconds(1));
  }
}

CronScheduler::CronScheduler(size_t working_threads)
    : jobs(), jobs_mtx(), schd_thread(), running(false),
      task_manager(working_threads) {}

CronScheduler::~CronScheduler() { this->stop(); }

void CronScheduler::start() {
  if (!this->running) {
    this->running = true;
    this->schd_thread = std::thread(&CronScheduler::run, this);
  }
}

void CronScheduler::stop() {
  if (this->running) {
    this->running = false;

    if (this->schd_thread.joinable())
      this->schd_thread.join();

    this->task_manager.wait_for_completion();
  }
}

bool CronScheduler::add_job(const std::string &id,
                            const std::string &description,
                            const std::string &cron_string,
                            std::function<void()> callback) {
  std::lock_guard<std::mutex> lock(this->jobs_mtx);
  if (this->jobs.count(id))
    return false;

  try {
    this->jobs.emplace(id, CronJob(id, description, cron_string, callback));

    return true;
  } catch (const std::invalid_argument &e) {
    return false;
  }
}

bool CronScheduler::remove_job(const std::string &id) {
  std::lock_guard<std::mutex> lock(this->jobs_mtx);

  auto it = this->jobs.find(id);
  if (it != this->jobs.end()) {
    this->jobs.erase(it);
    return true;
  }

  return false;
}

bool CronScheduler::set_job_enabled(const std::string &id, bool enabled) {
  std::lock_guard<std::mutex> lock(this->jobs_mtx);

  auto it = this->jobs.find(id);
  if (it != this->jobs.end()) {
    it->second.enabled = enabled;
    return true;
  }

  return false;
}

std::vector<CronJob> CronScheduler::get_all_jobs() const {
  std::lock_guard<std::mutex> lock(this->jobs_mtx);
  std::vector<CronJob> allJobs;

  for (const auto &pair : this->jobs)
    allJobs.push_back(pair.second);

  return allJobs;
}

} // namespace Netlet::Cron
