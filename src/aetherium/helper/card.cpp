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

#include <aetherium/helper/card.hpp>

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace Aetherium::Helper {

const std::vector<
    CardValidator::CardRule
> CardValidator::cardRules = {
    CardValidator::CardRule(
        CardProvider::VISA,
        {"4"},
        {13, 16, 19},
        3
    ),

    CardValidator::CardRule(
        CardProvider::MASTERCARD,
        {
            "51", "52", "53", "54", "55",
            "2221", "2222", "2223", "2224",
            "2225", "2226", "2227", "2228",
            "2229", "223", "224", "225",
            "226", "227", "228", "229",
            "23", "24", "25", "26", "270",
            "271", "2720"
        },
        {16},
        3
    ),

    CardValidator::CardRule(
        CardProvider::AMERICAN_EXPRESS,
        {"34", "37"},
        {15},
        4
    ),

    CardValidator::CardRule(
        CardProvider::DISCOVER,
        {
            "6011", "622126", "622127",
            "622128", "622129", "62213",
            "62214", "62215", "62216",
            "62217", "62218", "62219",
            "6222", "6223", "6224", "6225",
            "6226", "6227", "6228", "62290",
            "62291", "622920", "622921",
            "622922", "622923", "622924",
            "622925", "644", "645", "646",
            "647", "648", "649", "65"
        },
        {16, 19},
        3
    ),

    CardValidator::CardRule(
        CardProvider::DINERS_CLUB,
        {
            "300", "301", "302", "303",
            "304", "305", "36", "38", "39"
        },
        {14},
        3
    ),

    CardValidator::CardRule(
        CardProvider::JCB,
        {
            "3528", "3529", "353",
            "354", "355", "356",
            "357", "358"
        },
        {16},
        3
    ),

    CardValidator::CardRule(
        CardProvider::UNIONPAY,
        {"62"},
        {16, 17, 18, 19},
        3
    )
};

CardInfo CardValidator::validate_card_num(
    const std::string& card_num
) {
    CardInfo info;
    std::string clean_card_num = CardValidator
        ::remove_nondigits(card_num);

    info.card_num = clean_card_num;
    info.length = clean_card_num.length();

    if(clean_card_num.empty()) {
        info.validation_status =
            ValidationStatus::EMPTY_CARD_NUMBER;
        return info;
    }

    if(!CardValidator::is_all_digits(
        clean_card_num
    )) {
        info.validation_status =
            ValidationStatus::INVALID_CHARACTERS;
        return info;
    }

    info.is_valid_luhn = CardValidator::is_luhn_valid(
        clean_card_num
    );
    if(!info.is_valid_luhn) {
        info.validation_status =
            ValidationStatus::INVALID_LUHN;
        return info;
    }

    info.provider = CardValidator::detect_provider(
        clean_card_num
    );

    if(info.provider == CardProvider::UNKNOWN) {
        info.validation_status =
            ValidationStatus::INVALID_PREFIX;
        return info;
    }

    bool len_matches = false;
    for(const auto& rule : cardRules)
        if(rule.provider == info.provider) {
            for(int len : rule.lengths)
                if(info.length == len) {
                    len_matches = true;
                    break;
                }

            break;
        }

    if(!len_matches) {
        info.validation_status =
            ValidationStatus::INVALID_LENGTH;
        return info;
    }

    info.validation_status = ValidationStatus::VALID;
    return info;
}

bool CardValidator::is_luhn_valid(
    const std::string& card_num
) {
    std::string clean_card_num = CardValidator
        ::remove_nondigits(card_num);

    int sum = 0;
    bool alternate = false;
    for(int i = clean_card_num.length() - 1; i >= 0; --i) {
        int digit = clean_card_num[i] - '0';

        if(alternate) {
            digit *= 2;
            if(digit > 9)
                digit = (digit % 10) + (digit / 10);
        }

        sum += digit;
        alternate = !alternate;
    }

    return (sum % 10 == 0);
}

CardProvider CardValidator::detect_provider(
    const std::string& card_num
) {
    std::string clean_card_num = CardValidator
        ::remove_nondigits(card_num);

    int len = clean_card_num.length();
    for(const auto& rule : cardRules)
        for(const std::string& prefix : rule.prefixes)
            if(clean_card_num.rfind(prefix, 0) == 0)
                for(int valid_len : rule.lengths)
                    if(len == valid_len)
                        return rule.provider;

    return CardProvider::UNKNOWN;
}

std::string CardValidator::get_provider_name(
    CardProvider provider
) {
    switch(provider) {
        case CardProvider::VISA:
            return "VISA";

        case CardProvider::MASTERCARD:
            return "MasterCard";

        case CardProvider::AMERICAN_EXPRESS:
            return "American Express";

        case CardProvider::DISCOVER:
            return "Discover";

        case CardProvider::DINERS_CLUB:
            return "Diners Club";

        case CardProvider::JCB:
            return "JCB";

        case CardProvider::UNIONPAY:
            return "UnionPay";

        case CardProvider::UNKNOWN:
        default:
            return "Unknown";
    }
}

ValidationStatus CardValidator::validate_expiry_date(
    int month,
    int year
) {
    if(month < 1 || month > 12)
        return ValidationStatus::INVALID_EXPIRY_FORMAT;

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono
        ::system_clock
        ::to_time_t(now);
    std::tm* ptm = std::localtime(&now_c);

    int current_month = ptm->tm_mon + 1;
    int current_year = ptm->tm_year + 1900;

    if(year < 100) {
        year += (current_year / 100) * 100;
        if(year < current_year)
             year += 100;
    }

    if(year < current_year)
        return ValidationStatus::EXPIRED_CARD;
    else if(year == current_year) {
        if(month < current_month)
            return ValidationStatus::EXPIRED_CARD;
    }

    return ValidationStatus::VALID;
}

ValidationStatus CardValidator::validate_expiry_date(
    const std::string& mm_yy_format
) {
    int month, year;

    return !CardValidator::parse_expiry_date(
        mm_yy_format,
        month,
        year
    ) ? ValidationStatus::INVALID_EXPIRY_FORMAT :
        CardValidator::validate_expiry_date(month, year);
}

ValidationStatus CardValidator::validate_cvcv_format(
    const std::string& cvv,
    CardProvider provider
) {
    if(!CardValidator::is_all_digits(cvv))
        return ValidationStatus::INVALID_CHARACTERS;

    int expected_cvv_len = 0;
    for(const auto& rule : cardRules)
        if(rule.provider == provider) {
            expected_cvv_len = rule.cvvLength;
            break;
        }

    if(expected_cvv_len == 0 &&
        provider != CardProvider::UNKNOWN)
        return (cvv.length() == 3 || cvv.length() == 4) ?
            ValidationStatus::VALID :
            ValidationStatus::INVALID_CVV_LENGTH;
    else if(provider == CardProvider::UNKNOWN)
        return (cvv.length() == 3 || cvv.length() == 4) ?
            ValidationStatus::VALID :
            ValidationStatus::INVALID_CVV_LENGTH;

    return static_cast<int>(cvv.length()) !=
        static_cast<int>(expected_cvv_len) ?
            ValidationStatus::INVALID_CVV_LENGTH :
            ValidationStatus::VALID;
}

std::string CardValidator::mask_card_num(
    const std::string& card_num,
    char mask_char,
    int unmasked_digits
) {
    std::string clean_card_num = CardValidator
        ::remove_nondigits(card_num);

    if(clean_card_num.length() <=
        static_cast<std::size_t>(unmasked_digits))
        return clean_card_num;

    std::string masked = "";
    int masked_len = clean_card_num.length() - unmasked_digits;

    for(int i = 0; i < masked_len; ++i)
        masked += mask_char;
    masked += clean_card_num.substr(masked_len);

    return masked;
}

std::string CardValidator::format_card_num(
    const std::string& card_num,
    char separator
) {
    std::string formatted_num = "";
    std::string clean_card_num = CardValidator
        ::remove_nondigits(card_num);

    CardProvider provider = CardValidator
        ::detect_provider(clean_card_num);

    if(provider == CardProvider::AMERICAN_EXPRESS) {
        if(clean_card_num.length() > 4) {
            formatted_num += clean_card_num.substr(0, 4);

            if(clean_card_num.length() > 4) {
                formatted_num += separator;
                formatted_num += clean_card_num.substr(4, 6);

                if(clean_card_num.length() > 10) {
                    formatted_num += separator;
                    formatted_num += clean_card_num.substr(10, 5);
                }
            }
        }
        else formatted_num = clean_card_num;
    }
    else for(size_t i = 0; i < clean_card_num.length(); ++i) {
        if(i > 0 && i % 4 == 0)
            formatted_num += separator;
        formatted_num += clean_card_num[i];
    }

    return formatted_num;
}

std::string CardValidator::remove_nondigits(
    const std::string& str
) {
    std::string clean_str;

    for(char c : str)
        if(std::isdigit(c))
            clean_str += c;

    return clean_str;
}

bool CardValidator::is_all_digits(
    const std::string& str
) {
    return std::all_of(
        str.begin(),
        str.end(),
        ::isdigit
    );
}

bool CardValidator::parse_expiry_date(
    const std::string& mm_yy_format,
    int& month,
    int& year
) {
    std::string clean_fmt = CardValidator
        ::remove_nondigits(mm_yy_format);

    if(clean_fmt.length() == 4)
        try {
            month = std::stoi(clean_fmt.substr(0, 2));
            year = std::stoi(clean_fmt.substr(2, 2));

            return true;
        }
        catch(const std::invalid_argument& e) {
            return false;
        }
        catch(const std::out_of_range& e) {
            return false;
        }
    else if(clean_fmt.length() == 3)
        try {
            month = std::stoi(clean_fmt.substr(0, 1));
            year = std::stoi(clean_fmt.substr(1, 2));

            return true;
        }
        catch(const std::invalid_argument& e) {
            return false;
        }
        catch(const std::out_of_range& e) {
            return false;
        }

    return false;
}

}
