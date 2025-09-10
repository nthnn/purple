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

#ifndef AETHERIUM_HELPER_CARD_HPP
#define AETHERIUM_HELPER_CARD_HPP

#include <string>
#include <vector>

namespace Aetherium::Helper {

enum class CardProvider {
  UNKNOWN,
  VISA,
  MASTERCARD,
  AMERICAN_EXPRESS,
  DISCOVER,
  DINERS_CLUB,
  JCB,
  UNIONPAY
};

enum class ValidationStatus {
  VALID,
  INVALID_LUHN,
  INVALID_LENGTH,
  INVALID_PREFIX,
  INVALID_CHARACTERS,
  EMPTY_CARD_NUMBER,
  INVALID_EXPIRY_FORMAT,
  EXPIRED_CARD,
  INVALID_CVV_LENGTH,
  UNKNOWN_ERROR
};

struct CardInfo {
  CardInfo()
      : card_num(""), length(0), is_valid_luhn(false),
        provider(CardProvider::UNKNOWN),
        validation_status(ValidationStatus::UNKNOWN_ERROR) {}

  std::string card_num;
  int length;
  bool is_valid_luhn;

  CardProvider provider;
  ValidationStatus validation_status;
};

class CardValidator {
public:
  static CardInfo validate_card_num(const std::string &card_num);

  static bool is_luhn_valid(const std::string &card_num);

  static std::string get_provider_name(CardProvider provider);

  static CardProvider detect_provider(const std::string &card_num);

  static ValidationStatus validate_expiry_date(int month, int year);

  static ValidationStatus validate_expiry_date(const std::string &mm_yy_format);

  static ValidationStatus validate_cvcv_format(const std::string &cvv,
                                               CardProvider provider);

  static std::string mask_card_num(const std::string &card_num,
                                   char mask_char = 'X',
                                   int unmasked_digits = 4);

  static std::string format_card_num(const std::string &card_num,
                                     char separator = ' ');

private:
  static CardProvider getProviderFromPrefixAndLength(const std::string &prefix,
                                                     int length);

  static std::string remove_nondigits(const std::string &str);

  static bool is_all_digits(const std::string &str);
  static bool parse_expiry_date(const std::string &mm_yy_format, int &month,
                                int &year);

  struct CardRule {
    CardProvider provider;
    std::vector<std::string> prefixes;
    std::vector<int> lengths;
    int cvvLength;

    CardRule(CardProvider p, const std::vector<std::string> &pr,
             const std::vector<int> &l, int cvvL)
        : provider(p), prefixes(pr), lengths(l), cvvLength(cvvL) {}
  };

  static const std::vector<CardRule> cardRules;
};

} // namespace Aetherium::Helper

#endif
