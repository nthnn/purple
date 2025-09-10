#include <aetherium/helper/validator.hpp>

#include <iostream>

int main() {
  using namespace Aetherium::Helper;

  std::cout << "Email Validation:" << std::endl;
  std::cout << "test@example.com: "
            << InputValidator::is_valid_email("test@example.com") << std::endl;
  std::cout << "invalid-email: "
            << InputValidator::is_valid_email("invalid-email") << std::endl;

  std::cout << "\nPassword Validation (Default):" << std::endl;
  std::cout << "StrongP@ss1: "
            << InputValidator::is_valid_password("StrongP@ss1") << std::endl;
  std::cout << "weakpass: " << InputValidator::is_valid_password("weakpass")
            << std::endl;
  std::cout << "NoDigit!: " << InputValidator::is_valid_password("NoDigit!")
            << std::endl;

  std::cout << "\nPassword Validation (Custom Flags):" << std::endl;
  std::cout << "onlylower (flags: LOWERCASE, length 5): "
            << InputValidator::is_valid_password(PASSWORD_REQUIRE_LOWERCASE, 5,
                                                 "onlylower")
            << std::endl;
  std::cout << "ComplexP@ss10 (flags: DEFAULT, length 10): "
            << InputValidator::is_valid_password(PASSWORD_DEFAULT_COMPLEXITY,
                                                 10, "ComplexP@ss10")
            << std::endl;
  std::cout << "ShortP@1 (flags: DEFAULT, length 10): "
            << InputValidator::is_valid_password(PASSWORD_DEFAULT_COMPLEXITY,
                                                 10, "ShortP@1")
            << std::endl;
  std::cout << "NoSpecial123 (flags: DEFAULT, length 8): "
            << InputValidator::is_valid_password(PASSWORD_DEFAULT_COMPLEXITY, 8,
                                                 "NoSpecial123")
            << std::endl;

  std::cout << "\nURL Validation:" << std::endl;
  std::cout << "https://www.google.com: "
            << InputValidator::is_valid_url("https://www.google.com")
            << std::endl;
  std::cout << "ftp://ftp.example.com/file.zip: "
            << InputValidator::is_valid_url("ftp://ftp.example.com/file.zip")
            << std::endl;
  std::cout << "invalid-url: " << InputValidator::is_valid_url("invalid-url")
            << std::endl;

  std::cout << "\nIP Address Validation:" << std::endl;
  std::cout << "192.168.1.1: "
            << InputValidator::is_valid_ip_address("192.168.1.1") << std::endl;
  std::cout << "2001:0db8:85a3:0000:0000:8a2e:0370:7334: "
            << InputValidator::is_valid_ip_address(
                   "2001:0db8:85a3:0000:0000:8a2e:0370:7334")
            << std::endl;
  std::cout << "invalid.ip.address: "
            << InputValidator::is_valid_ip_address("invalid.ip.address")
            << std::endl;

  std::cout << "\nAlphanumeric Validation:" << std::endl;
  std::cout << "User123: " << InputValidator::is_alphanumeric("User123")
            << std::endl;
  std::cout << "User 123!: " << InputValidator::is_alphanumeric("User 123!")
            << std::endl;

  std::cout << "\nNumeric Validation:" << std::endl;
  std::cout << "12345: " << InputValidator::is_numeric("12345") << std::endl;
  std::cout << "123.45: " << InputValidator::is_numeric("123.45") << std::endl;

  std::cout << "\nInteger Validation:" << std::endl;
  std::cout << "123: " << InputValidator::is_integer("123") << std::endl;
  std::cout << "-456: " << InputValidator::is_integer("-456") << std::endl;
  std::cout << "123.0: " << InputValidator::is_integer("123.0") << std::endl;

  std::cout << "\nPositive Integer Validation:" << std::endl;
  std::cout << "5: " << InputValidator::is_positive_integer("5") << std::endl;
  std::cout << "-5: " << InputValidator::is_positive_integer("-5") << std::endl;
  std::cout << "0: " << InputValidator::is_positive_integer("0") << std::endl;

  std::cout << "\nNegative Integer Validation:" << std::endl;
  std::cout << "-5: " << InputValidator::is_negative_integer("-5") << std::endl;
  std::cout << "5: " << InputValidator::is_negative_integer("5") << std::endl;
  std::cout << "0: " << InputValidator::is_negative_integer("0") << std::endl;

  std::cout << "\nNon-Negative Integer Validation:" << std::endl;
  std::cout << "5: " << InputValidator::is_non_negative_integer("5")
            << std::endl;
  std::cout << "0: " << InputValidator::is_non_negative_integer("0")
            << std::endl;
  std::cout << "-5: " << InputValidator::is_non_negative_integer("-5")
            << std::endl;

  std::cout << "\nNon-Positive Integer Validation:" << std::endl;
  std::cout << "-5: " << InputValidator::is_non_positive_integer("5")
            << std::endl;
  std::cout << "0: " << InputValidator::is_non_positive_integer("0")
            << std::endl;
  std::cout << "-5: " << InputValidator::is_non_positive_integer("-5")
            << std::endl;

  std::cout << "\nFloat Validation:" << std::endl;
  std::cout << "123.45: " << InputValidator::is_float("123.45") << std::endl;
  std::cout << "-0.789: " << InputValidator::is_float("-0.789") << std::endl;
  std::cout << "123: " << InputValidator::is_float("123") << std::endl;
  std::cout << "abc: " << InputValidator::is_float("abc") << std::endl;

  std::cout << "\nLength Validation:" << std::endl;
  std::cout << "hello (min 3, max 10): "
            << InputValidator::has_length("hello", 3, 10) << std::endl;
  std::cout << "hi (min 3, max 10): " << InputValidator::has_length("hi", 3, 10)
            << std::endl;

  std::cout << "\nHash Validation:" << std::endl;
  std::cout << "MD5 (valid): "
            << InputValidator::is_valid_md5("5d41402abc4b2a76b9719d911017c592")
            << std::endl;
  std::cout
      << "SHA256 (valid): "
      << InputValidator::is_valid_sha256(
             "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
      << std::endl;
  std::cout << "Invalid hash: " << InputValidator::is_valid_md5("notahash")
            << std::endl;

  std::cout << "\nDate Validation:" << std::endl;
  std::cout << "2023-10-26: " << InputValidator::is_valid_date("2023-10-26")
            << std::endl;
  std::cout << "2023/10/26: " << InputValidator::is_valid_date("2023/10/26")
            << std::endl;
  std::cout << "2023-13-01: " << InputValidator::is_valid_date("2023-13-01")
            << std::endl;
  std::cout << "2023-02-30: " << InputValidator::is_valid_date("2023-02-30")
            << std::endl;

  std::cout << "\nUUID Validation:" << std::endl;
  std::cout << "f47ac10b-58cc-4372-a567-0e02b2c3d479: "
            << InputValidator::is_valid_uuid(
                   "f47ac10b-58cc-4372-a567-0e02b2c3d479")
            << std::endl;
  std::cout << "invalid-uuid: " << InputValidator::is_valid_uuid("invalid-uuid")
            << std::endl;

  std::cout << "\nHex Color Validation:" << std::endl;
  std::cout << "#FFF: " << InputValidator::is_valid_color("#FFF") << std::endl;
  std::cout << "#FF00AA: " << InputValidator::is_valid_color("#FF00AA")
            << std::endl;
  std::cout << "#1234: " << InputValidator::is_valid_color("#1234")
            << std::endl;
  std::cout << "#AABBCCDD: " << InputValidator::is_valid_color("#AABBCCDD")
            << std::endl;
  std::cout << "red: " << InputValidator::is_valid_color("red") << std::endl;

  std::cout << "\nPort Number Validation:" << std::endl;
  std::cout << "80: " << InputValidator::is_valid_port("80") << std::endl;
  std::cout << "65535: " << InputValidator::is_valid_port("65535") << std::endl;
  std::cout << "0: " << InputValidator::is_valid_port("0") << std::endl;
  std::cout << "65536: " << InputValidator::is_valid_port("65536") << std::endl;
  std::cout << "abc: " << InputValidator::is_valid_port("abc") << std::endl;

  std::cout << "\nUsername Validation:" << std::endl;
  std::cout << "my_user-name: "
            << InputValidator::is_valid_username("my_user-name") << std::endl;
  std::cout << "us: " << InputValidator::is_valid_username("us") << std::endl;
  std::cout << "user with spaces: "
            << InputValidator::is_valid_username("user with spaces")
            << std::endl;

  std::cout << "\nBoolean String Validation:" << std::endl;
  std::cout << "true: " << InputValidator::is_boolean("true") << std::endl;
  std::cout << "False: " << InputValidator::is_boolean("False") << std::endl;
  std::cout << "1: " << InputValidator::is_boolean("1") << std::endl;
  std::cout << "0: " << InputValidator::is_boolean("0") << std::endl;
  std::cout << "yes: " << InputValidator::is_boolean("yes") << std::endl;

  std::cout << "\nFile Name Validation:" << std::endl;
  std::cout << "my_document.txt: "
            << InputValidator::is_valid_filename("my_document.txt")
            << std::endl;
  std::cout << "invalid/file.txt: "
            << InputValidator::is_valid_filename("invalid/file.txt")
            << std::endl;
  std::cout << "another\\file: "
            << InputValidator::is_valid_filename("another\\file") << std::endl;

  std::cout << "\nPath Validation:" << std::endl;
  std::cout << "/home/user/docs: "
            << InputValidator::is_valid_path("/home/user/docs") << std::endl;
  std::cout << "C:\\Program Files\\App: "
            << InputValidator::is_valid_path("C:\\Program Files\\App")
            << std::endl;
  std::cout << "invalid*path: " << InputValidator::is_valid_path("invalid*path")
            << std::endl;

  std::cout << "\nBase64 Validation:" << std::endl;
  std::cout << "SGVsbG8gV29ybGQ=: "
            << InputValidator::is_base64("SGVsbG8gV29ybGQ=") << std::endl;
  std::cout << "SGVsbG8gV29ybGQ: "
            << InputValidator::is_base64("SGVsbG8gV29ybGQ") << std::endl;
  std::cout << "Invalid@Char: " << InputValidator::is_base64("Invalid@Char")
            << std::endl;

  return 0;
}
