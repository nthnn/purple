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

#include <aetherium/cron/timepoint.hpp>

#include <iomanip>
#include <sstream>

namespace Aetherium::Cron {

using TimePoint = std::chrono::system_clock::time_point;
using CronSeconds = std::chrono::seconds;

std::string timepoint_string(const TimePoint& tp) {
    std::time_t tt = std::chrono::system_clock
        ::to_time_t(tp);

    std::tm tm = *std::gmtime(&tt);

    std::stringstream ss;
    ss << std::put_time(
        &tm,
        "%Y-%m-%d %H:%M:%S UTC"
    );

    return ss.str();
}

TimePoint now() {
    return std::chrono::system_clock::now();
}

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) ||
        (year % 400 == 0);
}

int days_in_month(int year, int month) {
    static const int days[] = {
        0, 31,
        28, 31,
        30, 31,
        30, 31,
        31, 30,
        31, 30,
        31
    };

    return month == 2 && is_leap_year(year)
        ? 29 : days[month];
}

}
