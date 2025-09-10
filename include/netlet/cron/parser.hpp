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

#ifndef NETLET_CRON_PARSER_HPP
#define NETLET_CRON_PARSER_HPP

#include <map>
#include <set>
#include <string>

namespace Netlet::Cron {

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

} // namespace Netlet::Cron

#endif
