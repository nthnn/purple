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

#ifndef NETLET_HELPER_EMAIL_HPP
#define NETLET_HELPER_EMAIL_HPP

#include <map>
#include <string>

namespace Netlet::Helper {

class EmailAddress {
private:
  std::string display_name;
  std::string address;
  std::string local_part;
  std::string domain;
  bool valid;

  void parse_addr_parts();

public:
  EmailAddress();
  EmailAddress(const std::string &email_addr);

  void parse(const std::string &email_addr);

  const std::string &get_display_name() const;
  const std::string &get_address() const;
  const std::string &get_local_part() const;
  const std::string &get_domain() const;

  bool is_valid() const;
  std::string to_string() const;
};

class EmailMessage {
private:
  std::map<std::string, std::string> headers;

  std::string body;
  std::string content_type;
  std::string boundary;

public:
  EmailMessage();

  const std::map<std::string, std::string> &get_headers() const;

  std::string get_header(const std::string &name) const;
  const std::string &get_body() const;
  const std::string &get_content_type() const;
  const std::string &get_boundary() const;

  void set_headers(const std::map<std::string, std::string> &new_headers);

  void set_header(const std::string &name, const std::string &value);

  void set_body(const std::string &new_body);
  void set_content_type(const std::string &new_content_type);
  void set_boundary(const std::string &new_boundary);

  std::string build() const;
};

class EmailParser {
public:
  EmailParser() = default;
  EmailMessage parse(const std::string &raw_email);
};

} // namespace Netlet::Helper

#endif
