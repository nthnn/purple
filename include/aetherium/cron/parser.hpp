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

#ifndef AETHERIUM_CRON_PARSER_HPP
#define AETHERIUM_CRON_PARSER_HPP

#include <map>
#include <set>
#include <string>

namespace Aetherium::Cron {

struct CronParsedFields {
  std::set<int> minutes;
  std::set<int> hours;
  std::set<int> days_of_month;
  std::set<int> months;
  std::set<int> days_of_week;

  CronParsedFields();
};

class CronParser {
private:
  static const std::map<std::string, int> month_names;
  static const std::map<std::string, int> day_of_week_names;

  static std::set<int> parse_field(const std::string &field, int min_val,
                                   int max_val, bool parse_months = false,
                                   bool parse_days_of_week = false);

  static int conv_name_to_value(const std::string &name, bool parse_months,
                                bool parse_days_of_week);

public:
  static CronParsedFields parse(const std::string &cron_string);
};

} // namespace Aetherium::Cron

#endif
