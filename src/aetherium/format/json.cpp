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

#include <netlet/format/json.hpp>

#include <cmath>
#include <iomanip>
#include <sstream>

namespace Netlet::Format {

JsonValue::JsonValue(const JsonValue &other)
    : type(other.type), data(std::monostate{}) {
  switch (this->type) {
  case JsonValueType::Null:
    this->data = std::monostate();
    break;

  case JsonValueType::Boolean:
    this->data = std::get<bool>(other.data);
    break;

  case JsonValueType::Number:
    this->data = std::get<double>(other.data);
    break;

  case JsonValueType::String:
    this->data = std::get<std::string>(other.data);
    break;

  case JsonValueType::Array:
    this->data = std::get<JsonArray>(other.data);
    break;

  case JsonValueType::Object:
    this->data = std::get<JsonObject>(other.data);
    break;
  }
}

JsonValue &JsonValue::operator=(const JsonValue &other) {
  if (this != &other) {
    this->type = other.type;

    switch (this->type) {
    case JsonValueType::Null:
      this->data = std::monostate();
      break;

    case JsonValueType::Boolean:
      this->data = std::get<bool>(other.data);
      break;

    case JsonValueType::Number:
      this->data = std::get<double>(other.data);
      break;

    case JsonValueType::String:
      this->data = std::get<std::string>(other.data);
      break;

    case JsonValueType::Array:
      this->data = std::get<JsonArray>(other.data);
      break;

    case JsonValueType::Object:
      this->data = std::get<JsonObject>(other.data);
      break;
    }
  }

  return *this;
}

JsonValue::JsonValue(JsonValue &&other) noexcept
    : type(other.type), data(std::move(other.data)) {
  other.type = JsonValueType::Null;
}

JsonValue &JsonValue::operator=(JsonValue &&other) noexcept {
  if (this != &other) {
    this->type = other.type;
    this->data = std::move(other.data);

    other.type = JsonValueType::Null;
  }

  return *this;
}

bool JsonValue::is_null() const { return this->type == JsonValueType::Null; }

bool JsonValue::is_bool() const { return this->type == JsonValueType::Boolean; }

bool JsonValue::is_number() const {
  return this->type == JsonValueType::Number;
}

bool JsonValue::is_string() const {
  return this->type == JsonValueType::String;
}

bool JsonValue::is_array() const { return this->type == JsonValueType::Array; }

bool JsonValue::is_object() const {
  return this->type == JsonValueType::Object;
}

JsonValueType JsonValue::get_type() { return this->type; }

bool JsonValue::get_bool() const {
  if (!this->is_bool())
    throw JsonParseException("Value is not a boolean.");

  return std::get<bool>(this->data);
}

double JsonValue::get_number() const {
  if (!this->is_number())
    throw JsonParseException("Value is not a number.");

  return std::get<double>(this->data);
}

std::string JsonValue::get_string() const {
  if (!this->is_string())
    throw JsonParseException("Value is not a string.");

  return std::get<std::string>(this->data);
}

const JsonArray &JsonValue::get_array() const {
  if (!this->is_array())
    throw JsonParseException("Value is not an array.");

  return std::get<JsonArray>(this->data);
}

JsonArray &JsonValue::get_array() {
  if (!this->is_array())
    throw JsonParseException("Value is not an array.");

  return std::get<JsonArray>(this->data);
}

const JsonObject &JsonValue::get_object() const {
  if (!this->is_object())
    throw JsonParseException("Value is not an object.");

  return std::get<JsonObject>(this->data);
}

JsonObject &JsonValue::get_object() {
  if (!this->is_object())
    throw JsonParseException("Value is not an object.");

  return std::get<JsonObject>(this->data);
}

JsonValue &JsonValue::operator[](size_t index) {
  if (!this->is_array()) {
    if (this->is_null()) {
      this->type = JsonValueType::Array;
      this->data = JsonArray();
    } else
      throw JsonParseException(
          "Value is not an array, cannot access by index.");
  }

  JsonArray &arr = std::get<JsonArray>(this->data);
  if (index >= arr.size())
    arr.resize(index + 1);

  return arr[index];
}

const JsonValue &JsonValue::operator[](size_t index) const {
  if (!this->is_array())
    throw JsonParseException("Value is not an array, cannot access by index.");

  const JsonArray &arr = std::get<JsonArray>(this->data);
  if (index >= arr.size())
    throw std::out_of_range("Array index out of bounds.");

  return arr[index];
}

JsonValue &JsonValue::operator[](const std::string &key) {
  if (!this->is_object()) {
    if (this->is_null()) {
      this->type = JsonValueType::Object;
      this->data = JsonObject();
    } else
      throw JsonParseException("Value is not an object, cannot access by key.");
  }

  JsonObject &obj = std::get<JsonObject>(this->data);
  return obj[key];
}

const JsonValue &JsonValue::operator[](const std::string &key) const {
  if (!this->is_object())
    throw JsonParseException("Value is not an object, cannot access by key.");

  const JsonObject &obj = std::get<JsonObject>(this->data);
  auto it = obj.find(key);

  if (it == obj.end())
    throw std::out_of_range("Object key not found: " + key);
  return it->second;
}

void JsonValue::reserve(size_t capacity) {
  if (this->is_array())
    std::get<JsonArray>(this->data).reserve(capacity);
  else if (this->is_object())
    std::get<JsonObject>(this->data).reserve(capacity);
}

std::string JsonValue::serialize(bool pretty, int indent_level) const {
  std::stringstream ss;
  this->serialize_internal(ss, pretty, indent_level);

  return ss.str();
}

void JsonValue::serialize_internal(std::ostream &os, bool pretty,
                                   int indent_level) const {
  auto indent = [&](int level) {
    if (pretty)
      for (int i = 0; i < level; ++i)
        os << "    ";
  };
  auto newline = [&]() {
    if (pretty)
      os << "\n";
  };

  switch (this->type) {
  case JsonValueType::Null:
    os << "null";
    break;

  case JsonValueType::Boolean:
    os << (std::get<bool>(this->data) ? "true" : "false");
    break;

  case JsonValueType::Number: {
    double val = std::get<double>(this->data);

    if (std::isinf(val) || std::isnan(val))
      os << "null";
    else {
      std::string num_str = std::to_string(val);
      size_t dot_pos = num_str.find('.');

      if (dot_pos != std::string::npos) {
        num_str.erase(num_str.find_last_not_of('0') + 1, std::string::npos);

        if (num_str.back() == '.')
          num_str.pop_back();
      }

      os << num_str;
    }

    break;
  }

  case JsonValueType::String: {
    os << "\"";

    const std::string &s = std::get<std::string>(this->data);
    for (char c : s) {
      switch (c) {
      case '"':
        os << "\\\"";
        break;
      case '\\':
        os << "\\\\";
        break;
      case '\b':
        os << "\\b";
        break;
      case '\f':
        os << "\\f";
        break;
      case '\n':
        os << "\\n";
        break;
      case '\r':
        os << "\\r";
        break;
      case '\t':
        os << "\\t";
        break;

      default:
        if (static_cast<unsigned char>(c) < 0x20 ||
            static_cast<unsigned char>(c) > 0x7E) {
          std::stringstream hex_stream;
          hex_stream << std::hex << std::setw(4) << std::setfill('0')
                     << static_cast<int>(static_cast<unsigned char>(c));

          os << "\\u" << hex_stream.str();
        } else
          os << c;

        break;
      }
    }

    os << "\"";
    break;
  }

  case JsonValueType::Array: {
    const JsonArray &arr = std::get<JsonArray>(this->data);
    os << "[";

    if (!arr.empty())
      newline();

    for (size_t i = 0; i < arr.size(); ++i) {
      indent(indent_level + 1);
      arr[i].serialize_internal(os, pretty, indent_level + 1);

      if (i < arr.size() - 1)
        os << ",";
      newline();
    }

    if (!arr.empty())
      indent(indent_level);
    os << "]";

    break;
  }

  case JsonValueType::Object: {
    const JsonObject &obj = std::get<JsonObject>(this->data);
    os << "{";

    if (!obj.empty())
      newline();

    size_t i = 0;
    for (const auto &pair : obj) {
      indent(indent_level + 1);
      os << "\"";

      const std::string &key = pair.first;
      for (char c : key) {
        switch (c) {
        case '"':
          os << "\\\"";
          break;
        case '\\':
          os << "\\\\";
          break;
        case '\b':
          os << "\\b";
          break;
        case '\f':
          os << "\\f";
          break;
        case '\n':
          os << "\\n";
          break;
        case '\r':
          os << "\\r";
          break;
        case '\t':
          os << "\\t";
          break;

        default:
          if (static_cast<unsigned char>(c) < 0x20 ||
              static_cast<unsigned char>(c) > 0x7E) {
            std::stringstream hex_stream;
            hex_stream << std::hex << std::setw(4) << std::setfill('0')
                       << static_cast<int>(static_cast<unsigned char>(c));

            os << "\\u" << hex_stream.str();
          } else
            os << c;

          break;
        }
      }

      os << "\"";
      os << (pretty ? ": " : ":");

      pair.second.serialize_internal(os, pretty, indent_level + 1);

      if (i < obj.size() - 1)
        os << ",";
      newline();

      i++;
    }

    if (!obj.empty())
      indent(indent_level);
    os << "}";

    break;
  }
  }
}

JsonValue JsonParser::parse(const std::string &json_str) {
  this->json_str = json_str;
  this->pos = 0;

  this->skip_whitespace();
  if (this->pos >= this->json_str.length())
    throw JsonParseException("Empty JSON string.");

  JsonValue result = this->parse_value();
  this->skip_whitespace();

  if (this->pos < this->json_str.length())
    throw JsonParseException("Unexpected characters after JSON root element.");
  return result;
}

void JsonParser::skip_whitespace() {
  while (
      this->pos < this->json_str.length() &&
      (this->json_str[this->pos] == ' ' || this->json_str[this->pos] == '\t' ||
       this->json_str[this->pos] == '\n' || this->json_str[this->pos] == '\r'))
    this->pos++;
}

char JsonParser::peek() const {
  return (this->pos >= this->json_str.length()) ? '\0'
                                                : this->json_str[this->pos];
}

char JsonParser::next() {
  if (this->pos >= this->json_str.length())
    throw JsonParseException("Unexpected end of input.");

  return this->json_str[this->pos++];
}

bool JsonParser::match(char c) {
  if (this->peek() == c) {
    this->next();
    return true;
  }

  return false;
}

JsonValue JsonParser::parse_value() {
  this->skip_whitespace();

  char c = this->peek();
  switch (c) {
  case 'n':
    return this->parse_null();

  case 't':
  case 'f':
    return this->parse_bool();

  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return this->parse_number();

  case '"':
    return this->parse_string();

  case '[':
    return this->parse_array();

  case '{':
    return this->parse_object();

  default:
    throw JsonParseException("Unexpected character '" + std::string(1, c) +
                             "' at position " + std::to_string(this->pos));
  }
}

JsonValue JsonParser::parse_null() {
  if (this->pos + 4 > this->json_str.length() ||
      this->json_str.substr(this->pos, 4) != "null")
    throw JsonParseException("Expected 'null'.");

  this->pos += 4;
  return JsonValue();
}

JsonValue JsonParser::parse_bool() {
  if (this->pos + 4 <= this->json_str.length() &&
      this->json_str.substr(this->pos, 4) == "true") {
    this->pos += 4;
    return JsonValue(true);
  }

  if (this->pos + 5 <= this->json_str.length() &&
      this->json_str.substr(this->pos, 5) == "false") {
    this->pos += 5;
    return JsonValue(false);
  }

  throw JsonParseException("Expected 'true' or 'false'.");
}

JsonValue JsonParser::parse_number() {
  size_t start_pos = this->pos;
  bool is_negative = false;

  if (this->peek() == '-') {
    is_negative = true;
    this->next();
  }

  if (!std::isdigit(this->peek()))
    throw JsonParseException(
        "Invalid number format: expected digit at position " +
        std::to_string(this->pos));

  if (this->peek() == '0') {
    this->next();

    if (std::isdigit(this->peek()))
      throw JsonParseException(
          "Invalid number: leading zero not allowed for non-zero numbers.");
  } else
    while (std::isdigit(this->peek()))
      this->next();

  if (this->peek() == '.') {
    this->next();
    if (!std::isdigit(this->peek()))
      throw JsonParseException(
          "Invalid number: expected digit after decimal point at position " +
          std::to_string(this->pos));

    while (std::isdigit(this->peek()))
      this->next();
  }

  if (this->peek() == 'e' || this->peek() == 'E') {
    this->next();
    if (this->peek() == '+' || this->peek() == '-')
      this->next();

    if (!std::isdigit(this->peek()))
      throw JsonParseException(
          "Invalid number: expected digit after exponent sign at position " +
          std::to_string(this->pos));

    while (std::isdigit(this->peek()))
      this->next();
  }

  std::string_view num_view =
      this->json_str.substr(start_pos, this->pos - start_pos);

  try {
    double value = std::stod(std::string(num_view));

    if (is_negative)
      value = -value;
    return JsonValue(value);
  } catch (const std::out_of_range &e) {
    throw JsonParseException("Number out of range: " + std::string(num_view));
  } catch (const std::invalid_argument &e) {
    throw JsonParseException("Invalid number format: " + std::string(num_view));
  }
}

std::string JsonParser::parse_string_content() {
  std::string result;
  size_t start_segment = this->pos;

  while (this->peek() != '"') {
    if (this->pos >= this->json_str.length())
      throw JsonParseException("Unterminated string.");

    char c = this->peek();
    if (c == '\\') {
      if (this->pos > start_segment)
        result.append(this->json_str.data() + start_segment,
                      this->pos - start_segment);
      this->next();

      result += this->parse_esc_sequence();
      start_segment = this->pos;
    } else if (static_cast<unsigned char>(c) < 0x20)
      throw JsonParseException(
          "Unescaped control character in string at position " +
          std::to_string(this->pos));
    else
      this->next();
  }

  if (this->pos > start_segment)
    result.append(this->json_str.data() + start_segment,
                  this->pos - start_segment);

  return result;
}

char JsonParser::parse_esc_sequence() {
  if (this->pos >= this->json_str.length())
    throw JsonParseException("Incomplete escape sequence.");

  char escape_char = this->next();
  switch (escape_char) {
  case '"':
    return '"';

  case '\\':
    return '\\';

  case '/':
    return '/';

  case 'b':
    return '\b';

  case 'f':
    return '\f';

  case 'n':
    return '\n';

  case 'r':
    return '\r';

  case 't':
    return '\t';

  case 'u': {
    std::string hex_str;
    hex_str.reserve(4);

    for (int i = 0; i < 4; ++i) {
      if (this->pos >= this->json_str.length())
        throw JsonParseException("Incomplete unicode escape sequence.");

      char hex_char = this->next();
      if (!std::isxdigit(hex_char))
        throw JsonParseException(
            "Invalid hex digit in unicode escape sequence at position " +
            std::to_string(this->pos - 1));

      hex_str += hex_char;
    }

    int unicode_val = std::stoi(hex_str, nullptr, 16);
    if (unicode_val >= 0x00 && unicode_val <= 0x7F)
      return static_cast<char>(unicode_val);
    else
      throw JsonParseException(
          "Unsupported non-ASCII unicode escape sequence \\u" + hex_str +
          ". Full UTF-8 not implemented for parsing.");
  }

  default:
    throw JsonParseException("Invalid escape sequence '\\" +
                             std::string(1, escape_char) + "' at position " +
                             std::to_string(this->pos - 1));
  }
}

JsonValue JsonParser::parse_string() {
  if (!this->match('"'))
    throw JsonParseException("Expected '\"' to start string at position " +
                             std::to_string(this->pos));

  std::string s = this->parse_string_content();
  if (!this->match('"'))
    throw JsonParseException("Expected '\"' to end string at position " +
                             std::to_string(this->pos));

  return JsonValue(s);
}

JsonValue JsonParser::parse_array() {
  if (!this->match('['))
    throw JsonParseException("Expected '[' to start array at position " +
                             std::to_string(this->pos));

  JsonArray arr;
  this->skip_whitespace();

  if (this->peek() == ']') {
    this->next();
    return JsonValue(arr);
  }

  arr.reserve(4);
  while (true) {
    arr.push_back(this->parse_value());
    this->skip_whitespace();

    if (this->peek() == ',') {
      this->next();
      this->skip_whitespace();
    } else if (this->peek() == ']') {
      this->next();
      break;
    } else
      throw JsonParseException(
          "Expected ',' or ']' after array element at position " +
          std::to_string(this->pos));
  }

  return JsonValue(arr);
}

JsonValue JsonParser::parse_object() {
  if (!this->match('{'))
    throw JsonParseException("Expected '{' to start object at position " +
                             std::to_string(this->pos));

  JsonObject obj;
  this->skip_whitespace();

  if (this->peek() == '}') {
    this->next();
    return JsonValue(obj);
  }

  obj.reserve(4);
  while (true) {
    this->skip_whitespace();
    if (!this->match('"'))
      throw JsonParseException("Expected '\"' for object key at position " +
                               std::to_string(this->pos));

    std::string key = this->parse_string_content();
    if (!this->match('"'))
      throw JsonParseException("Expected '\"' to end object key at position " +
                               std::to_string(this->pos));

    this->skip_whitespace();
    if (!this->match(':'))
      throw JsonParseException("Expected ':' after object key at position " +
                               std::to_string(this->pos));

    this->skip_whitespace();
    obj[key] = this->parse_value();
    this->skip_whitespace();

    if (this->peek() == ',') {
      this->next();
      this->skip_whitespace();
    } else if (this->peek() == '}') {
      this->next();
      break;
    } else
      throw JsonParseException(
          "Expected ',' or '}' after object value at position " +
          std::to_string(this->pos));
  }

  return JsonValue(obj);
}

} // namespace Netlet::Format
