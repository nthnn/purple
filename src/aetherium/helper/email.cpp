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

#include <aetherium/helper/email.hpp>

#include <algorithm>
#include <regex>

namespace Aetherium::Helper {

std::string email_str_trim(
    const std::string& str
) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if(std::string::npos == first)
        return str;

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void EmailAddress::parse_addr_parts() {
    size_t at_pos = this->address.find('@');
    if(at_pos != std::string::npos) {
        this->local_part = this->address.substr(0, at_pos);
        this->domain = this->address.substr(at_pos + 1);
    }
    else {
        this->local_part = this->address;
        this->domain = "";
    }
}

EmailAddress::EmailAddress() :
    display_name(""),
    address(""),
    local_part(""),
    domain(""),
    valid(false) { }

EmailAddress::EmailAddress(
    const std::string& email_addr
) : display_name(""),
    address(""),
    local_part(""),
    domain(""),
    valid(false)
{
    this->parse(email_addr);
}

void EmailAddress::parse(
    const std::string& email_addr
) {
    std::string clean_str = email_str_trim(
        email_addr
    );
    size_t lt_pos = clean_str.find('<');
    size_t gt_pos = clean_str.find('>');

    if(lt_pos != std::string::npos &&
        gt_pos != std::string::npos && gt_pos > lt_pos
    ) {
        this->display_name = email_str_trim(
            clean_str.substr(0, lt_pos)
        );

        if(this->display_name.length() >= 2 &&
            this->display_name.front() == '"' &&
            this->display_name.back() == '"')
            this->display_name = this->display_name.substr(
                1,
                this->display_name.length() - 2
            );

        this->address = email_str_trim(clean_str.substr(
            lt_pos + 1,
            gt_pos - (lt_pos + 1)
        ));
    }
    else this->address = clean_str;

    this->parse_addr_parts();
    this->valid = is_valid();
}

const std::string& EmailAddress::get_display_name() const {
    return this->display_name;
}

const std::string& EmailAddress::get_address() const {
    return this->address;
}

const std::string& EmailAddress::get_local_part() const {
    return this->local_part;
}

const std::string& EmailAddress::get_domain() const {
    return this->domain;
}

bool EmailAddress::is_valid() const {
    const std::regex email_regex(
        R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"
    );

    return std::regex_match(
        this->address,
        email_regex
    );
}

std::string EmailAddress::to_string() const {
    if(!this->is_valid())
        return "";

    if(!this->display_name.empty())
        return (this->display_name.find(' ') != std::string::npos ||
            this->display_name.find(',') != std::string::npos ||
            this->display_name.find('<') != std::string::npos ||
            this->display_name.find('>') != std::string::npos ||
            this->display_name.find('"') != std::string::npos
        ) ? "\"" + this->display_name + "\" <" + this->address + ">" :
            this->display_name + " <" + this->address + ">";

    return this->address;
}

EmailMessage::EmailMessage() :
    headers(),
    body(""),
    content_type("text/plain"),
    boundary("") { }

const std::map<
    std::string,
    std::string
>& EmailMessage::get_headers() const {
    return this->headers;
}

std::string EmailMessage::get_header(
    const std::string& name
) const {
    for(const auto& pair : this->headers) {
        std::string lower_key = pair.first;
        std::transform(
            lower_key.begin(),
            lower_key.end(),
            lower_key.begin(),
            ::tolower
        );

        std::string lower_name = name;
        std::transform(
            lower_name.begin(),
            lower_name.end(),
            lower_name.begin(),
            ::tolower
        );

        if(lower_key == lower_name)
            return pair.second;
    }

    return "";
}

const std::string& EmailMessage::get_body() const {
    return this->body;
}

const std::string& EmailMessage::get_content_type() const {
    return this->content_type;
}

const std::string& EmailMessage::get_boundary() const {
    return this->boundary;
}

void EmailMessage::set_headers(
    const std::map<
        std::string,
        std::string
    >& new_headers
) {
    this->headers = new_headers;
}

void EmailMessage::set_header(
    const std::string& name,
    const std::string& value
) {
    this->headers[name] = value;
}

void EmailMessage::set_body(const std::string& new_body) {
    this->body = new_body;
}

void EmailMessage::set_content_type(
    const std::string& new_content_type
) {
    this->content_type = new_content_type;
}

void EmailMessage::set_boundary(
    const std::string& new_boundary
) {
    this->boundary = new_boundary;
}

std::string EmailMessage::build() const {
    std::ostringstream oss;
    for(const auto& pair : this->headers)
        oss << pair.first
            << ": "
            << pair.second
            << "\r\n";

    oss << "\r\n";
    oss << this->body;

    return oss.str();
}

EmailMessage EmailParser::parse(const std::string& raw_email) {
    EmailMessage email;
    std::istringstream iss(raw_email);
    std::string line;

    bool in_headers = true;
    std::string current_header_name;
    std::string current_header_value;

    while(std::getline(iss, line)) {
        if(in_headers && email_str_trim(line).empty()) {
            if(!current_header_name.empty())
                email.set_header(
                    current_header_name,
                    email_str_trim(current_header_value)
                );

            in_headers = false;
            continue;
        }

        if(in_headers) {
            if(!line.empty() &&
                (line[0] == ' ' || line[0] == '\t')) {
                if(!current_header_name.empty())
                    current_header_value += " " +
                        email_str_trim(line);
            }
            else {
                if(!current_header_name.empty())
                    email.set_header(
                        current_header_name,
                        email_str_trim(current_header_value)
                    );

                size_t colon_pos = line.find(':');
                if(colon_pos != std::string::npos) {
                    current_header_name =
                        email_str_trim(line.substr(0, colon_pos));
                    current_header_value =
                        email_str_trim(line.substr(colon_pos + 1));
                }
                else {
                    if(!current_header_name.empty())
                            current_header_value += " " +
                            email_str_trim(line);
                }
            }
        }
        else email.set_body(
            email.get_body() + line + "\n"
        );
    }

    if(in_headers && !current_header_name.empty())
        email.set_header(
            current_header_name,
            email_str_trim(current_header_value)
        );

    std::string content_type_header =
        email.get_header("Content-Type");
    if(!content_type_header.empty()) {
        size_t semicolon_pos = content_type_header.find(';');
        if(semicolon_pos != std::string::npos) {
            email.set_content_type(email_str_trim(
                content_type_header.substr(0, semicolon_pos)
            ));

            std::string params = content_type_header.substr(semicolon_pos + 1);
            std::string param;

            std::istringstream paramStream(params);
            while(std::getline(paramStream, param, ';')) {
                size_t eq_pos = param.find('=');
                if(eq_pos != std::string::npos) {
                    std::string key = email_str_trim(
                        param.substr(0, eq_pos)
                    );
                    std::string value = email_str_trim(
                        param.substr(eq_pos + 1)
                    );

                    if(value.length() >= 2 &&
                        value.front() == '"' &&
                        value.back() == '"')
                        value = value.substr(1, value.length() - 2);

                    if(key == "boundary")
                        email.set_boundary(value);
                }
            }
        }
        else email.set_content_type(email_str_trim(
            content_type_header
        ));
    }

    return email;
}

}
