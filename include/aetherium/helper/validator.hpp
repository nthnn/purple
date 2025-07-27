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

#ifndef AETHERIUM_HELPER_VALIDATOR_HPP
#define AETHERIUM_HELPER_VALIDATOR_HPP

#include <regex>
#include <string>

namespace Aetherium::Helper {

typedef enum {
    PASSWORD_NONE                   = 0,
    PASSWORD_REQUIRE_LOWERCASE      = 1 << 0,
    PASSWORD_REQUIRE_UPPERCASE      = 1 << 1,
    PASSWORD_REQUIRE_DIGIT          = 1 << 2,
    PASSWORD_REQUIRE_SPECIAL        = 1 << 3,
    PASSWORD_DEFAULT_COMPLEXITY     =
        PASSWORD_REQUIRE_LOWERCASE  |
        PASSWORD_REQUIRE_UPPERCASE  |
        PASSWORD_REQUIRE_DIGIT      |
        PASSWORD_REQUIRE_SPECIAL
} PasswordStrengthFlags;

class InputValidator {
private:
    static bool regex_match(
        const std::string& s,
        const std::regex& r
    );

public:
    static bool has_length(
        const std::string& str,
        size_t min_len,
        size_t max_len
    );

    static bool is_valid_password(const std::string& password);
    static bool is_valid_password(
        PasswordStrengthFlags flags,
        size_t length_required,
        const std::string& password
    );

    static bool is_valid_email(const std::string& email);
    static bool is_valid_username(const std::string& username);
    static bool is_valid_url(const std::string& url);

    static bool is_valid_ipv4(const std::string& ip_addr);
    static bool is_valid_ipv6(const std::string& ip_addr);
    static bool is_valid_ip_address(const std::string& ip_addr);

    static bool is_alphanumeric(const std::string& str);
    static bool is_alphabetic(const std::string& str);

    static bool is_numeric(const std::string& str);
    static bool is_integer(const std::string& str);
    static bool is_float(const std::string& str);

    static bool is_positive_integer(const std::string& str);
    static bool is_negative_integer(const std::string& str);
    static bool is_non_negative_integer(const std::string& str);
    static bool is_non_positive_integer(const std::string& str);

    static bool is_valid_md5(const std::string& hash);
    static bool is_valid_sha1(const std::string& hash);
    static bool is_valid_sha256(const std::string& hash);
    static bool is_valid_sha512(const std::string& hash);

    static bool is_valid_date(const std::string& date);
    static bool is_valid_uuid(const std::string& uuid);
    static bool is_valid_color(const std::string& hex_color);
    static bool is_valid_port(const std::string& port);

    static bool is_boolean(const std::string& bool_str);
    static bool is_base64(const std::string& b64_str);

    static bool is_valid_filename(const std::string& filename);
    static bool is_valid_path(const std::string& path);
};

}

#endif
