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

#ifndef PURPLE_FORMAT_JSON_HPP
#define PURPLE_FORMAT_JOSN_HPP

#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Purple::Format {

class JsonValue;

using JsonArray = std::vector<JsonValue>;
using JsonObject = std::unordered_map<std::string, JsonValue>;

enum class JsonValueType { Null, Boolean, Number, String, Array, Object };

class JsonParseException : public std::runtime_error {
public:
  explicit JsonParseException(const std::string &message)
      : std::runtime_error("JSON Parse Error: " + message) {}
};

class JsonValue {
private:
  JsonValueType type;

  std::variant<std::monostate, bool, double, std::string, JsonArray, JsonObject>
      data;

  void serialize_internal(std::ostream &os, bool pretty,
                          int indent_level) const;

public:
  JsonValue() : type(JsonValueType::Null), data(std::monostate{}) {}

  JsonValue(bool b) : type(JsonValueType::Boolean), data(b) {}

  JsonValue(int i)
      : type(JsonValueType::Number), data(static_cast<double>(i)) {}

  JsonValue(double d) : type(JsonValueType::Number), data(d) {}

  JsonValue(const std::string &s) : type(JsonValueType::String), data(s) {}

  JsonValue(const char *s)
      : type(JsonValueType::String), data(std::string(s)) {}

  JsonValue(const JsonArray &a) : type(JsonValueType::Array), data(a) {}

  JsonValue(JsonArray &&a) : type(JsonValueType::Array), data(std::move(a)) {}

  JsonValue(const std::map<std::string, JsonValue> &o)
      : type(JsonValueType::Object), data(JsonObject(o.begin(), o.end())) {}

  JsonValue(const JsonObject &o) : type(JsonValueType::Object), data(o) {}

  JsonValue(JsonObject &&o) : type(JsonValueType::Object), data(std::move(o)) {}

  JsonValue(const JsonValue &other);
  JsonValue(JsonValue &&other) noexcept;

  ~JsonValue() = default;

  JsonValue &operator=(const JsonValue &other);
  JsonValue &operator=(JsonValue &&other) noexcept;

  bool is_null() const;
  bool is_bool() const;
  bool is_number() const;
  bool is_string() const;
  bool is_array() const;
  bool is_object() const;
  JsonValueType get_type();

  bool get_bool() const;
  double get_number() const;
  std::string get_string() const;
  JsonArray &get_array();
  JsonObject &get_object();

  const JsonArray &get_array() const;
  const JsonObject &get_object() const;

  JsonValue &operator[](size_t index);
  JsonValue &operator[](const std::string &key);

  const JsonValue &operator[](size_t index) const;
  const JsonValue &operator[](const std::string &key) const;

  void reserve(size_t capacity);
  std::string serialize(bool pretty = false, int indent_level = 0) const;
};

class JsonParser {
private:
  std::string_view json_str;
  size_t pos;

  void skip_whitespace();
  char peek() const;
  char next();
  bool match(char c);

  JsonValue parse_value();
  JsonValue parse_null();
  JsonValue parse_bool();
  JsonValue parse_number();
  JsonValue parse_string();
  JsonValue parse_array();
  JsonValue parse_object();

  std::string parse_string_content();
  char parse_esc_sequence();

public:
  JsonValue parse(const std::string &json_str);

  JsonParser() : json_str(""), pos(0) {}
};

} // namespace Purple::Format

#endif
