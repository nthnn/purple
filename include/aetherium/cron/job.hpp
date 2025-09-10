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

#ifndef NETLET_CRON_JOB_HPP
#define NETLET_CRON_JOB_HPP

#include <netlet/cron/schedule.hpp>
#include <netlet/cron/timepoint.hpp>

#include <functional>
#include <string>

namespace Netlet::Cron {

class CronJob {
public:
  std::string id;
  std::string description;

  CronSchedule schedule;
  std::function<void()> callback;

  TimePoint next_runtime;
  bool enabled;

  CronJob(const std::string &id, const std::string &desc,
          const std::string &cron_str, std::function<void()> cb);

  void update_next_runtime();
};

} // namespace Netlet::Cron

#endif
