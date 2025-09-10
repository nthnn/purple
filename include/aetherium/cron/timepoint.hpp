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

#ifndef AETHERIUM_CRON_TIMEPOINT_HPP
#define AETHERIUM_CRON_TIMEPOINT_HPP

#include <chrono>
#include <string>

namespace Aetherium::Cron {

using TimePoint = std::chrono::system_clock::time_point;
using CronSeconds = std::chrono::seconds;

std::string timepoint_string(const TimePoint &tp);
TimePoint now();

bool is_leap_year(int year);
int days_in_month(int year, int month);

} // namespace Aetherium::Cron

#endif
