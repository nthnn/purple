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

#include <netlet/format/dotenv.hpp>

#include <fstream>
#include <iostream>

namespace Netlet::Format {

std::string DotEnv::trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\n\r\f\v");
  if (std::string::npos == first)
    return str;

  size_t last = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(first, (last - first + 1));
}

std::string DotEnv::unquote_and_unescape(const std::string &value) {
  if (value.empty())
    return "";

  std::string result = value;
  if ((result.length() >= 2 && result.front() == '\'' &&
       result.back() == '\'') ||
      (result.length() >= 2 && result.front() == '"' && result.back() == '"')) {
    char quote_char = result.front();
    result = result.substr(1, result.length() - 2);

    if (quote_char == '"') {
      std::string unescaped_result;
      for (size_t i = 0; i < result.length(); ++i) {
        if (result[i] == '\\' && i + 1 < result.length()) {
          switch (result[i + 1]) {
          case 'n':
            unescaped_result += '\n';
            break;

          case 'r':
            unescaped_result += '\r';
            break;

          case 't':
            unescaped_result += '\t';
            break;

          case '\\':
            unescaped_result += '\\';
            break;

          case '"':
            unescaped_result += '"';
            break;

          default:
            unescaped_result += result[i];
            unescaped_result += result[i + 1];
            break;
          }

          i++;
        } else
          unescaped_result += result[i];
      }

      result = unescaped_result;
    }
  }

  return result;
}

bool DotEnv::load(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open())
    return false;

  std::string line;
  int line_num = 0;

  while (std::getline(file, line)) {
    line_num++;
    line = trim(line);

    if (line.empty() || line[0] == '#')
      continue;

    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos)
      continue;

    std::string key = trim(line.substr(0, eq_pos));
    std::string value = trim(line.substr(eq_pos + 1));

    value = unquote_and_unescape(value);
    env_vars[key] = value;
  }

  file.close();
  return true;
}

std::string DotEnv::get(const std::string &key) const {
  auto it = env_vars.find(key);
  if (it != env_vars.end())
    return it->second;

  throw std::out_of_range("Environment variable '" + key + "' not found.");
}

std::string DotEnv::get(const std::string &key,
                        const std::string &default_value) const {
  auto it = env_vars.find(key);
  return it != env_vars.end() ? it->second : default_value;
}

bool DotEnv::has(const std::string &key) const {
  return env_vars.count(key) > 0;
}

} // namespace Netlet::Format
