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

#ifndef NETLET_FORMAT_DOTENV_HPP
#define NETLET_FORMAT_DOTENV_HPP

#include <map>
#include <string>

namespace Netlet::Format {

class DotEnv {
private:
  std::map<std::string, std::string> env_vars;

  static std::string trim(const std::string &str);
  static std::string unquote_and_unescape(const std::string &value);

public:
  DotEnv() : env_vars({}) {}

  bool load(const std::string &filepath);
  std::string get(const std::string &key) const;
  std::string get(const std::string &key,
                  const std::string &default_value) const;

  bool has(const std::string &key) const;
};

} // namespace Netlet::Format

#endif