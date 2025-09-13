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

#ifndef PURPLE_CRON_TIMEPOINT_HPP
#define PURPLE_CRON_TIMEPOINT_HPP

#include <chrono>
#include <string>

namespace Purple::Cron {

/**
 * @brief Alias representing a point in time using the system clock.
 *
 * This type alias simplifies working with
 * `std::chrono::system_clock::time_point` throughout the Purple::Cron module.
 * It represents an absolute moment in time (typically expressed as UTC).
 */
using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Alias for durations expressed in seconds.
 *
 * This alias provides a convenient shorthand for expressing time intervals
 * in seconds within the cron system.
 */
using CronSeconds = std::chrono::seconds;

/**
 * @brief Converts a time point into a human-readable UTC string.
 *
 * Formats a `TimePoint` into a string in the format:
 * `"YYYY-MM-DD HH:MM:SS UTC"`.
 *
 * @param tp The `TimePoint` to format.
 * @return A string representation of the time in UTC.
 */
std::string timepoint_string(const TimePoint &tp);

/**
 * @brief Gets the current system time as a `TimePoint`.
 *
 * This is equivalent to calling `std::chrono::system_clock::now()`
 * but provides a consistent abstraction inside the Purple::Cron module.
 *
 * @return The current system time.
 */
TimePoint now();

/**
 * @brief Determines whether a given year is a leap year.
 *
 * A leap year is defined by the Gregorian calendar rules:
 * - Divisible by 4 and not divisible by 100, OR
 * - Divisible by 400.
 *
 * @param year The year to check.
 * @return `true` if the year is a leap year, otherwise `false`.
 */
bool is_leap_year(int year);

/**
 * @brief Returns the number of days in a given month for a specific year.
 *
 * Accounts for leap years when calculating February’s days.
 *
 * @param year The year (used to account for leap years).
 * @param month The month (1–12).
 * @return Number of days in the given month.
 */
int days_in_month(int year, int month);

} // namespace Purple::Cron

#endif
