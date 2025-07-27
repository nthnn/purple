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

#include <aetherium/helper/validator.hpp>

namespace Aetherium::Helper {

bool InputValidator::regex_match(
    const std::string& s,
    const std::regex& r
) {
    return std::regex_match(s, r);
}

bool InputValidator::has_length(
    const std::string& str,
    size_t min_len,
    size_t max_len
) {
    return str.length() >= min_len &&
        str.length() <= max_len;
}

bool InputValidator::is_valid_password(
    const std::string& password
) {
    return is_valid_password(
        PASSWORD_DEFAULT_COMPLEXITY,
        8,
        password
    );
}

bool InputValidator::is_valid_password(
    PasswordStrengthFlags flags,
    size_t length_required,
    const std::string& password
) {
    if(password.length() < length_required)
        return false;

    if((flags & PASSWORD_REQUIRE_LOWERCASE) &&
        !std::regex_search(password, std::regex("[a-z]")))
        return false;
    if((flags & PASSWORD_REQUIRE_UPPERCASE) &&
        !std::regex_search(password, std::regex("[A-Z]")))
        return false;
    if((flags & PASSWORD_REQUIRE_DIGIT) &&
        !std::regex_search(password, std::regex("[0-9]")))
        return false;
    if((flags & PASSWORD_REQUIRE_SPECIAL) &&
        !std::regex_search(password, std::regex("[^a-zA-Z0-9\\s]")))
        return false;

    return true;
}

bool InputValidator::is_valid_username(
    const std::string& username
) {
    const std::regex username_regex(R"(^[a-zA-Z0-9_-]{3,20}$)");
    return InputValidator::regex_match(username, username_regex);
}

bool InputValidator::is_valid_email(
    const std::string& email
) {
    const std::regex email_regex(
        R"(^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$)"
    );

    return InputValidator::regex_match(email, email_regex);
}

bool InputValidator::is_valid_url(
    const std::string& url
) {
    const std::regex url_regex(
        R"(^(https?|ftp):\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)$)"
    );

    return InputValidator::regex_match(url, url_regex);
}

bool InputValidator::is_valid_ipv4(
    const std::string& ip_addr
) {
    const std::regex ipv4_regex(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
    );

    return InputValidator::regex_match(ip_addr, ipv4_regex);
}

bool InputValidator::is_valid_ipv6(
    const std::string& ip_addr
) {
    const std::regex ipv6_regex(
        R"(^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3,3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3,3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))$)"
    );

    return InputValidator::regex_match(ip_addr, ipv6_regex);
}

bool InputValidator::is_valid_ip_address(
    const std::string& ip_addr
) {
    return is_valid_ipv4(ip_addr) ||
        is_valid_ipv6(ip_addr);
}

bool InputValidator::is_alphanumeric(
    const std::string& str
) {
    const std::regex alphanumeric_regex(
        R"(^[a-zA-Z0-9]*$)"
    );

    return InputValidator::regex_match(str, alphanumeric_regex);
}

bool InputValidator::is_alphabetic(
    const std::string& str
) {
    const std::regex alphabetic_regex(R"(^[a-zA-Z]*$)");
    return InputValidator::regex_match(str, alphabetic_regex);
}

bool InputValidator::is_numeric(
    const std::string& str
) {
    const std::regex numeric_regex(R"(^[0-9]*$)");
    return InputValidator::regex_match(str, numeric_regex);
}

bool InputValidator::is_integer(
    const std::string& str
) {
    const std::regex integer_regex(R"(^[+-]?\d+$)");
    return InputValidator::regex_match(str, integer_regex);
}

bool InputValidator::is_float(
    const std::string& str
) {
    const std::regex float_regex(
        R"(^[+-]?(\d*\.\d+|\d+\.?\d*)$)"
    );

    return InputValidator::regex_match(str, float_regex);
}

bool InputValidator::is_positive_integer(
    const std::string& str
) {
    if(!InputValidator::is_integer(str))
        return false;

    try {
        return std::stoll(str) > 0;
    }
    catch(const std::out_of_range& oor) {
        return str.front() != '-';
    }
}

bool InputValidator::is_negative_integer(
    const std::string& str
) {
    if(!InputValidator::is_integer(str))
        return false;

    try {
        return std::stoll(str) < 0;
    }
    catch(const std::out_of_range& oor) {
        return str.front() == '-';
    }
}

bool InputValidator::is_non_negative_integer(
    const std::string& str
) {
    if(!InputValidator::is_integer(str))
        return false;

    try {
        return std::stoll(str) >= 0;
    }
    catch(const std::out_of_range& oor) {
        return str.front() != '-';
    }
}

bool InputValidator::is_non_positive_integer(
    const std::string& str
) {
    if(!InputValidator::is_integer(str))
        return false;

    try {
        return std::stoll(str) <= 0;
    }
    catch(const std::out_of_range& oor) {
        return str.front() == '-';
    }
}

bool InputValidator::is_valid_md5(
    const std::string& hash
) {
    const std::regex md5_regex(R"(^[0-9a-fA-F]{32}$)");
    return InputValidator::regex_match(hash, md5_regex);
}

bool InputValidator::is_valid_sha1(
    const std::string& hash
) {
    const std::regex sha1_regex(R"(^[0-9a-fA-F]{40}$)");
    return InputValidator::regex_match(hash, sha1_regex);
}

bool InputValidator::is_valid_sha256(
    const std::string& hash
) {
    const std::regex sha256_regex(R"(^[0-9a-fA-F]{64}$)");
    return InputValidator::regex_match(hash, sha256_regex);
}

bool InputValidator::is_valid_sha512(
    const std::string& hash
) {
    const std::regex sha512_regex(R"(^[0-9a-fA-F]{128}$)");
    return InputValidator::regex_match(hash, sha512_regex);
}

bool InputValidator::is_valid_date(
    const std::string& date
) {
    const std::regex date_regex(
        R"(^\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$)"
    );

    return InputValidator::regex_match(date, date_regex);
}

bool InputValidator::is_valid_uuid(
    const std::string& uuid
) {
    const std::regex uuid_regex(
        R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)"
    );

    return InputValidator::regex_match(
        uuid,
        uuid_regex
    );
}

bool InputValidator::is_valid_color(
    const std::string& hex_color
) {
    const std::regex hex_color_regex(
        R"(^#([0-9a-fA-F]{3}|[0-9a-fA-F]{4}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$)"
    );

    return InputValidator::regex_match(
        hex_color,
        hex_color_regex
    );
}

bool InputValidator::is_valid_port(
    const std::string& port
) {
    if(!InputValidator::is_integer(port))
        return false;

    try {
        int port_num = std::stoi(port);
        return port_num >= 1 && port_num <= 65535;
    }
    catch(const std::out_of_range& oor) {
        return false;
    }
    catch(const std::invalid_argument& ia) {
        return false;
    }
}

bool InputValidator::is_boolean(
    const std::string& bool_str
) {
    std::string lowerString;
    lowerString.resize(bool_str.length());

    std::transform(
        bool_str.begin(),
        bool_str.end(),
        lowerString.begin(),
        ::tolower
    );

    return (lowerString == "true" ||
        lowerString == "false" ||
        lowerString == "1" ||
        lowerString == "0");
}

bool InputValidator::is_base64(
    const std::string& b64_str
) {
    const std::regex base64_regex(
        R"(^[A-Za-z0-9+/]*={0,2}$)"
    );

    if(!regex_match(b64_str, base64_regex))
        return false;

    return b64_str.length() % 4 == 0;
}

bool InputValidator::is_valid_filename(
    const std::string& filename
) {
    const std::regex invalid_chars_regex(
        R"([\\/:*?"<>|])"
    );

    return !std::regex_search(
        filename,
        invalid_chars_regex
    ) && !filename.empty();
}

bool InputValidator::is_valid_path(const std::string& path) {
    const std::regex path_regex(
        R"(^([a-zA-Z0-9\s._-]+[\\/]?)*[a-zA-Z0-9\s._-]*$|^[\\/]$)"
    );

    return InputValidator::regex_match(path, path_regex);
}

}
