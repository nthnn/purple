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

#include <netlet/cron/parser.hpp>

#include <sstream>
#include <vector>

namespace Netlet::Cron {

CronParsedFields::CronParsedFields()
    : minutes(), hours(), days_of_month(), months(), days_of_week() {}

std::set<int> CronParser::parse_field(const std::string &field, int min_val,
                                      int max_val, bool parse_months,
                                      bool parse_days_of_week) {
  std::set<int> values;
  std::stringstream ss(field);
  std::string item;

  while (std::getline(ss, item, ',')) {
    if (item == "*")
      for (int i = min_val; i <= max_val; ++i)
        values.insert(i);
    else if (item.find('/') != std::string::npos) {
      size_t slashPos = item.find('/');
      std::string base = item.substr(0, slashPos);
      int step = std::stoi(item.substr(slashPos + 1));

      int startVal = min_val;
      int endVal = max_val;

      if (base != "*") {
        size_t dash_pos = base.find('-');
        if (dash_pos != std::string::npos) {
          startVal = CronParser::conv_name_to_value(
              base.substr(0, dash_pos), parse_months, parse_days_of_week);

          endVal = CronParser::conv_name_to_value(
              base.substr(dash_pos + 1), parse_months, parse_days_of_week);
        } else {
          startVal = CronParser::conv_name_to_value(base, parse_months,
                                                    parse_days_of_week);
          endVal = startVal;
        }
      }

      for (int i = startVal; i <= endVal; i += step)
        if (i >= min_val && i <= max_val)
          values.insert(i);
    } else if (item.find('-') != std::string::npos) {
      size_t dash_pos = item.find('-');
      int start = CronParser::conv_name_to_value(
          item.substr(0, dash_pos), parse_months, parse_days_of_week);
      int end = CronParser::conv_name_to_value(
          item.substr(dash_pos + 1), parse_months, parse_days_of_week);

      if (start > end) {
        for (int i = start; i <= max_val; ++i)
          values.insert(i);

        for (int i = min_val; i <= end; ++i)
          values.insert(i);
      } else
        for (int i = start; i <= end; ++i)
          values.insert(i);
    } else {
      int val = CronParser::conv_name_to_value(item, parse_months,
                                               parse_days_of_week);

      if (val >= min_val && val <= max_val)
        values.insert(val);
      else
        throw std::invalid_argument("Value " + item + " out of range [" +
                                    std::to_string(min_val) + "-" +
                                    std::to_string(max_val) + "]");
    }
  }

  if (values.empty())
    throw std::invalid_argument("Field " + field +
                                " resulted in no valid values.");

  return values;
}

int CronParser::conv_name_to_value(const std::string &name, bool parse_months,
                                   bool parse_days_of_week) {
  if (parse_months) {
    auto it = CronParser::month_names.find(name);
    if (it != CronParser::month_names.end())
      return it->second;
  }

  if (parse_days_of_week) {
    auto it = day_of_week_names.find(name);
    if (it != day_of_week_names.end())
      return it->second;
  }

  return std::stoi(name);
}

CronParsedFields CronParser::parse(const std::string &cron_string) {
  CronParsedFields fields;
  std::stringstream ss(cron_string);
  std::string segment;
  std::vector<std::string> segments;

  while (std::getline(ss, segment, ' '))
    segments.push_back(segment);

  if (segments.size() != 5)
    throw std::invalid_argument(
        "Invalid cron string format, expected 5 fields");

  try {
    fields.minutes = CronParser::parse_field(segments[0], 0, 59);

    fields.hours = CronParser::parse_field(segments[1], 0, 23);

    fields.days_of_month = CronParser::parse_field(segments[2], 1, 31);

    fields.months = CronParser::parse_field(segments[3], 1, 12, true);

    fields.days_of_week =
        CronParser::parse_field(segments[4], 0, 7, false, true);
  } catch (const std::invalid_argument &e) {
    throw std::invalid_argument("Error parsing cron field: " +
                                std::string(e.what()));
  }

  return fields;
}

const std::map<std::string, int> CronParser::month_names = {
    {"JAN", 1}, {"FEB", 2}, {"MAR", 3}, {"APR", 4},  {"MAY", 5},  {"JUN", 6},
    {"JUL", 7}, {"AUG", 8}, {"SEP", 9}, {"OCT", 10}, {"NOV", 11}, {"DEC", 12}};

const std::map<std::string, int> CronParser::day_of_week_names = {
    {"SUN", 0}, {"MON", 1}, {"TUE", 2}, {"WED", 3},
    {"THU", 4}, {"FRI", 5}, {"SAT", 6}, {"7", 0}};

} // namespace Netlet::Cron