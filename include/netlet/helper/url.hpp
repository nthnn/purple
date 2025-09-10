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

#ifndef NETLET_HELPER_URL_HPP
#define NETLET_HELPER_URL_HPP

#include <map>
#include <regex>
#include <string>

namespace Netlet::Helper {

class UrlParser {
public:
  UrlParser(const std::string &url);

  std::string get_original_url() const;
  std::string get_scheme() const;
  std::string get_host() const;
  std::string get_port() const;
  std::string get_path() const;
  std::string get_fragment() const;
  std::string get_authority() const;
  std::string get_origin() const;

  const std::map<std::string, std::string> &get_query_params() const;

  void set_scheme(const std::string &scheme);
  void set_host(const std::string &host);
  void set_port(const std::string &port);
  void set_path(const std::string &path);
  void set_fragment(const std::string &fragment);

  std::string get_query_params_str() const;
  std::string get_param(const std::string &key) const;

  bool has_param(const std::string &key) const;
  bool has_query_params() const;
  void clear_queries();

  void add_query_param(const std::string &key, const std::string &value);

  bool remove_query_param(const std::string &key);

  bool is_secure() const;
  bool is_default_port() const;

  std::string get_file_name() const;
  std::string get_extension() const;
  std::string build_url() const;

private:
  std::string host;
  std::string port;
  std::string path;
  std::string scheme;
  std::string fragment;
  std::string original_url;
  std::map<std::string, std::string> query_params;

  void parse_url();
  void parse_query_params(const std::string &query_string);
};

} // namespace Netlet::Helper

#endif
