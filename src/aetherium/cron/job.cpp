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

#include <aetherium/cron/job.hpp>

namespace Aetherium::Cron {

CronJob::CronJob(const std::string &id, const std::string &desc,
                 const std::string &cron_str, std::function<void()> cb)
    : id(id), description(desc), schedule(cron_str), callback(cb),
      next_runtime(), enabled(true) {
  this->next_runtime = schedule.get_next_runtime(now());
}

void CronJob::update_next_runtime() {
  this->next_runtime =
      schedule.get_next_runtime(this->next_runtime + CronSeconds(1));
}

} // namespace Aetherium::Cron
