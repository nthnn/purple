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
 * @file parser.hpp
 * @author Nathanne Isip
 * @brief Provides parsing utilities for cron expression strings.
 *
 * The cron parser takes a 5-field cron expression (minute, hour,
 * day of month, month, day of week) and converts it into sets
 * of valid integer values that can be evaluated by the scheduler.
 *
 * Supported syntax:
 * - `*` → any valid value
 * - `*\/n` → step values (every n units)
 * - `a-b` → range of values
 * - `a-b/n` → stepped range
 * - `a,b,c` → explicit list
 * - Month names (`JAN`, `FEB`, …) and day names (`MON`, `TUE`, …)
 *
 */
#ifndef PURPLE_CRON_PARSER_HPP
#define PURPLE_CRON_PARSER_HPP

#include <map>
#include <set>
#include <string>

namespace Purple::Cron {

/**
 * @struct CronParsedFields
 * @brief Holds the parsed values for each cron expression field.
 *
 * Each field is represented as a set of integers corresponding
 * to the allowed values at runtime. These sets are used by
 * `CronSchedule` to determine matching times.
 */
struct CronParsedFields {
  /**
   * @brief Allowed minute values (0–59).
   */
  std::set<int> minutes;

  /**
   * @brief Allowed hour values (0–23).
   */
  std::set<int> hours;

  /**
   * @brief Allowed day-of-month values (1–31).
   */
  std::set<int> days_of_month;

  /**
   * @brief Allowed month values (1–12, or names).
   */
  std::set<int> months;

  /**
   * @brief Allowed day-of-week values (0–6, Sunday=0).
   */
  std::set<int> days_of_week;

  /**
   * @brief Constructs an empty field container.
   */
  CronParsedFields();
};

/**
 * @class CronParser
 * @brief Parses cron expression strings into structured fields.
 *
 * The parser validates and expands cron expressions into
 * sets of integers, which represent the times at which a job
 * is eligible to run.
 *
 * Internally it supports name-to-value conversion for months
 * and weekdays, as well as range, list, step, and wildcard
 * syntaxes.
 */
class CronParser {
private:
  /**
   * @brief Lookup table for month name abbreviations.
   *
   * Maps `JAN` → 1, `FEB` → 2, …, `DEC` → 12.
   */
  static const std::map<std::string, int> month_names;

  /**
   * @brief Lookup table for day-of-week name abbreviations.
   *
   * Maps `SUN` → 0, `MON` → 1, …, `SAT` → 6.
   * Also accepts `"7"` as an alias for Sunday (`0`).
   */
  static const std::map<std::string, int> day_of_week_names;

  /**
   * @brief Parses an individual cron field into a set of integers.
   *
   * @param field The raw field string (e.g. `"*\/5"`, `"MON-FRI"`).
   * @param min_val Minimum allowed value for this field.
   * @param max_val Maximum allowed value for this field.
   * @param parse_months Whether to allow month names.
   * @param parse_days_of_week Whether to allow day-of-week names.
   * @return A set of valid integer values for the field.
   *
   * @throw std::invalid_argument if no valid values are produced or
   * if a value is out of range.
   */
  static std::set<int> parse_field(const std::string &field, int min_val,
                                   int max_val, bool parse_months = false,
                                   bool parse_days_of_week = false);

  /**
   * @brief Converts a name or numeric string to a numeric value.
   *
   * @param name Input string (e.g. `"JAN"`, `"MON"`, `"12"`).
   * @param parse_months Whether to allow month name resolution.
   * @param parse_days_of_week Whether to allow day name resolution.
   * @return Numeric value of the name.
   *
   * @throw std::invalid_argument if the name cannot be resolved.
   */
  static int conv_name_to_value(const std::string &name, bool parse_months,
                                bool parse_days_of_week);

public:
  /**
   * @brief Parses a full cron expression string into fields.
   *
   * @param cron_string A 5-field cron string:
   *        `minute hour day_of_month month day_of_week`.
   *
   * @return A `CronParsedFields` struct containing the allowed values.
   *
   * @throw std::invalid_argument if the format is invalid or
   * if any field cannot be parsed.
   */
  static CronParsedFields parse(const std::string &cron_string);
};

} // namespace Purple::Cron

#endif
