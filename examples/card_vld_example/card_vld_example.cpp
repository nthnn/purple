#include <purple/helper/card.hpp>

#include <chrono>
#include <iostream>
#include <vector>

int main() {
  using namespace Purple::Helper;

  std::vector<std::string> test_cards = {
      "4000 1234 5678 9010", // Valid Visa
      "5432-1234-5678-9012", // Valid MasterCard
      "3782-8227-8288-828",  // Valid Amex
      "4111 1111 1111 1112", // Invalid Luhn
      "1234 5678 9012",      // Invalid prefix/length for major cards
      "6011-0000-0000-0000", // Valid Discover
      "4000123456789010123", // Valid Visa (19 digits)
      "6299123456789012"     // UnionPay
  };

  std::cout << "--- Card Number Validation ---" << std::endl;
  for (const auto &card : test_cards) {
    CardInfo info = CardValidator ::validate_card_num(card);

    std::cout << "Card: \"" << card << "\"" << std::endl;
    std::cout << "  Cleaned: " << info.card_num << std::endl;
    std::cout << "  Provider: "
              << CardValidator ::get_provider_name(info.provider) << std::endl;
    std::cout << "  Luhn Valid: " << (info.is_valid_luhn ? "Yes" : "No")
              << std::endl;
    std::cout << "  Validation Status: ";

    switch (info.validation_status) {
    case ValidationStatus::VALID:
      std::cout << "VALID";
      break;

    case ValidationStatus::INVALID_LUHN:
      std::cout << "INVALID_LUHN";
      break;

    case ValidationStatus::INVALID_LENGTH:
      std::cout << "INVALID_LENGTH";
      break;

    case ValidationStatus::INVALID_PREFIX:
      std::cout << "INVALID_PREFIX";
      break;

    case ValidationStatus::INVALID_CHARACTERS:
      std::cout << "INVALID_CHARACTERS";
      break;

    case ValidationStatus::EMPTY_CARD_NUMBER:
      std::cout << "EMPTY_CARD_NUMBER";
      break;

    default:
      std::cout << "UNKNOWN_ERROR";
      break;
    }

    std::cout << std::endl;
    std::cout << "  Masked: " << CardValidator::mask_card_num(card, '*', 4)
              << std::endl;
    std::cout << "  Formatted: " << CardValidator::format_card_num(card, '-')
              << std::endl;

    std::cout << "---------------------------------" << std::endl;
  }

  std::cout << "\n--- Expiry Date Validation ---" << std::endl;
  std::cout << "Current Date (approx): "
            << std::chrono::system_clock::to_time_t(
                   std::chrono::system_clock::now())
            << std::endl;

  std::vector<std::string> test_expirations = {
      "12/25",   // Valid
      "01/23",   // Expired
      "07/2025", // Valid (assuming current month is before July)
      "13/26",   // Invalid month
      "06/24"    // Expired (assuming current month is July 2025)
  };

  for (const auto &expiry : test_expirations) {
    ValidationStatus status = CardValidator ::validate_expiry_date(expiry);

    std::cout << "Expiry: \"" << expiry << "\" Status: ";

    switch (status) {
    case ValidationStatus::VALID:
      std::cout << "VALID";
      break;

    case ValidationStatus::INVALID_EXPIRY_FORMAT:
      std::cout << "INVALID_EXPIRY_FORMAT";
      break;

    case ValidationStatus::EXPIRED_CARD:
      std::cout << "EXPIRED_CARD";
      break;

    default:
      std::cout << "UNKNOWN_ERROR";
      break;
    }

    std::cout << std::endl;
  }

  std::cout << "\n--- CVV/CVC Validation ---" << std::endl;

  std::cout << "CVV \"123\" for VISA: "
            << (CardValidator::validate_cvcv_format(
                    "123", CardProvider::VISA) == ValidationStatus::VALID
                    ? "VALID"
                    : "INVALID")
            << std::endl;

  std::cout << "CVV \"1234\" for VISA: "
            << (CardValidator::validate_cvcv_format(
                    "1234", CardProvider::VISA) == ValidationStatus::VALID
                    ? "VALID"
                    : "INVALID")
            << std::endl;

  std::cout << "CVV \"1234\" for American Express: "
            << (CardValidator::validate_cvcv_format(
                    "1234", CardProvider::AMERICAN_EXPRESS) ==
                        ValidationStatus::VALID
                    ? "VALID"
                    : "INVALID")
            << std::endl;

  std::cout << "CVV \"12\" for MasterCard: "
            << (CardValidator::validate_cvcv_format(
                    "12", CardProvider::MASTERCARD) == ValidationStatus::VALID
                    ? "VALID"
                    : "INVALID")
            << std::endl;

  std::cout << "CVV \"abc\" for Discover: "
            << (CardValidator::validate_cvcv_format(
                    "abc", CardProvider::DISCOVER) == ValidationStatus::VALID
                    ? "VALID"
                    : "INVALID")
            << std::endl;

  return 0;
}
