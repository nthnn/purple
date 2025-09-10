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

#include <aetherium/helper/url.hpp>

namespace Aetherium::Helper {

UrlParser::UrlParser(const std::string &url)
    : host(""), port(""), path(""), scheme(""), fragment(""), original_url(url),
      query_params() {
  this->parse_url();
}

std::string UrlParser::get_original_url() const { return this->original_url; }

std::string UrlParser::get_scheme() const { return this->scheme; }

void UrlParser::set_scheme(const std::string &scheme) { this->scheme = scheme; }

std::string UrlParser::get_host() const { return this->host; }

void UrlParser::set_host(const std::string &host) { this->host = host; }

std::string UrlParser::get_port() const { return this->port; }

void UrlParser::set_port(const std::string &port) { this->port = port; }

std::string UrlParser::get_path() const { return this->path; }

void UrlParser::set_path(const std::string &path) { this->path = path; }

const std::map<std::string, std::string> &UrlParser::get_query_params() const {
  return this->query_params;
}

std::string UrlParser::get_fragment() const { return this->fragment; }

void UrlParser::set_fragment(const std::string &fragment) {
  this->fragment = fragment;
}

std::string UrlParser::get_authority() const {
  std::string authority = this->host;
  if (!this->port.empty())
    authority += ":" + this->port;

  return authority;
}

std::string UrlParser::get_origin() const {
  return this->scheme + "://" + this->get_authority();
}

std::string UrlParser::get_query_params_str() const {
  if (this->query_params.empty())
    return "";

  std::string query_str;
  for (const auto &pair : this->query_params) {
    if (!query_str.empty())
      query_str += "&";
    query_str += pair.first + "=" + pair.second;
  }

  return query_str;
}

std::string UrlParser::get_param(const std::string &key) const {
  auto it = this->query_params.find(key);
  return it != this->query_params.end() ? it->second : "";
}

bool UrlParser::has_param(const std::string &key) const {
  return this->query_params.count(key) > 0;
}

void UrlParser::add_query_param(const std::string &key,
                                const std::string &value) {
  this->query_params[key] = value;
}

bool UrlParser::remove_query_param(const std::string &key) {
  return this->query_params.erase(key) > 0;
}

void UrlParser::clear_queries() { this->query_params.clear(); }

bool UrlParser::has_query_params() const { return !this->query_params.empty(); }

bool UrlParser::is_secure() const {
  std::string lower_scheme = this->scheme;
  std::transform(lower_scheme.begin(), lower_scheme.end(), lower_scheme.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  return lower_scheme == "https";
}

bool UrlParser::is_default_port() const {
  if (this->port.empty())
    return true;

  return this->is_secure() ? this->port == "443" : this->port == "80";
}

std::string UrlParser::get_file_name() const {
  size_t last_slash = this->path.find_last_of('/');
  if (last_slash == std::string::npos || last_slash == this->path.length() - 1)
    return "";

  std::string file_name_with_extension = this->path.substr(last_slash + 1);

  size_t q_pos = file_name_with_extension.find('?');
  if (q_pos != std::string::npos)
    file_name_with_extension = file_name_with_extension.substr(0, q_pos);

  size_t h_pos = file_name_with_extension.find('#');
  if (h_pos != std::string::npos)
    file_name_with_extension = file_name_with_extension.substr(0, h_pos);

  return file_name_with_extension;
}

std::string UrlParser::get_extension() const {
  std::string file_name = get_file_name();
  if (file_name.empty())
    return "";

  size_t last_dot = file_name.find_last_of('.');
  if (last_dot == std::string::npos || last_dot == 0)
    return "";

  return file_name.substr(last_dot + 1);
}

std::string UrlParser::build_url() const {
  std::string url = this->get_scheme() + "://";
  url += this->get_host();

  std::string port_str = this->get_port();
  if (!port_str.empty())
    url += ":" + port_str;
  url += this->get_path();

  std::string query_str = this->get_query_params_str();
  if (!query_str.empty())
    url += "?" + query_str;

  std::string frag = this->get_fragment();
  if (!frag.empty())
    url += "#" + frag;

  return url;
}

void UrlParser::parse_url() {
  std::regex url_regex(
      R"(^(\w+):\/\/([^/?#:]+)(:\d+)?([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)");

  std::smatch matches;
  if (std::regex_match(this->original_url, matches, url_regex)) {
    this->scheme = matches[1].str();
    this->host = matches[2].str();

    if (matches[3].matched)
      this->port = matches[3].str().substr(1);

    this->path = matches[4].str();
    if (this->path.empty() &&
        (this->original_url.find('?') != std::string::npos ||
         this->original_url.find('#') != std::string::npos))
      this->path = "/";
    else if (this->path.empty() &&
             this->original_url.find("://") != std::string::npos &&
             this->original_url.find("://") + 3 == this->original_url.length())
      this->path = "/";
    else if (this->path.empty() &&
             this->original_url.find("://") != std::string::npos)
      this->path = "/";

    if (matches[5].matched)
      parse_query_params(matches[5].str());
    this->fragment = matches[6].str();
  } else
    throw std::invalid_argument("Invalid URL format: " + this->original_url);
}

void UrlParser::parse_query_params(const std::string &query_string) {
  std::regex param_regex("([^&]+)=([^&]*)");
  auto words_begin = std::sregex_iterator(query_string.begin(),
                                          query_string.end(), param_regex);

  auto words_end = std::sregex_iterator();
  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;

    if (match.size() == 3)
      this->query_params[match[1].str()] = match[2].str();
  }
}

} // namespace Aetherium::Helper
