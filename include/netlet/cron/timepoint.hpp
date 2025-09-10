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

#ifndef NETLET_CRON_TIMEPOINT_HPP
#define NETLET_CRON_TIMEPOINT_HPP

#include <chrono>
#include <string>

namespace Netlet::Cron {

using TimePoint = std::chrono::system_clock::time_point;
using CronSeconds = std::chrono::seconds;

std::string timepoint_string(const TimePoint &tp);
TimePoint now();

bool is_leap_year(int year);
int days_in_month(int year, int month);

} // namespace Netlet::Cron

#endif
