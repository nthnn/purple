#include <aetherium/helper/email.hpp>
#include <iostream>

using namespace Aetherium::Helper;

void print_email_addr(EmailAddress address) {
    std::cout << "  Full String: "
        << address.to_string()
        << std::endl;

    std::cout << "  Display Name: "
        << (address.get_display_name().empty() ?
            "(None)" : address.get_display_name())
        << std::endl;

    std::cout << "  Address: "
        << (address.get_address().empty() ?
            "(None)" : address.get_address())
        << std::endl;

    std::cout << "  Local Part: "
        << (address.get_local_part().empty() ?
            "(None)" : address.get_local_part())
        << std::endl;

    std::cout << "  Domain: "
        << (address.get_domain().empty() ?
            "(None)" : address.get_domain())
        << std::endl;

    std::cout << "  Valid: "
        << (address.is_valid() ? "Yes" : "No")
        << std::endl;
}

void print_email_message(EmailMessage message) {
    std::cout << "--- Email Headers ---"
        << std::endl;
    for(const auto& pair : message.get_headers())
        std::cout << pair.first
            << ": "
            << pair.second
            << std::endl;

    std::cout << "---------------------"
        << std::endl;
    std::cout << "Content-Type: "
        << message.get_content_type()
        << std::endl;

    if(!message.get_boundary().empty())
        std::cout << "Boundary: "
            << message.get_boundary()
            << std::endl;

    std::cout << "--- Email Body ---"
        << std::endl;
    std::cout << message.get_body()
        << std::endl;
    std::cout << "------------------"
        << std::endl;
}

int main() {
    std::string raw_email1 =
        "From: John Doe <john.doe@example.com>\r\n"
        "To: Jane Smith <jane.smith@example.org>\r\n"
        "Subject: Hello from C++ Email Parser!\r\n"
        "Date: Fri, 26 Jul 2025 10:00:00 -0700\r\n"
        "Content-Type: text/plain; charset=\"UTF-8\"\r\n"
        "Message-ID: <12345@example.com>\r\n"
        "\r\n"
        "This is a simple plain text email body.\r\n"
        "It demonstrates basic header and body parsing.\r\n"
        "Best regards,\r\n"
        "The C++ Email Parser";

    std::string raw_email2 =
        "From: Alice <alice@example.com>\r\n"
        "To: Bob <bob@example.com>\r\n"
        "Subject: Multipart Test Email\r\n"
        "Content-Type: multipart/alternative; boundary=\"----=_NextPart_000_0001_01D1A2B3.C4D5E6F7\"\r\n"
        "\r\n"
        "------=_NextPart_000_0001_01D1A2B3.C4D5E6F7\r\n"
        "Content-Type: text/plain; charset=\"us-ascii\"\r\n"
        "Content-Transfer-Encoding: 7bit\r\n"
        "\r\n"
        "This is the plain text part of a multipart message.\r\n"
        "------=_NextPart_000_0001_01D1A2B3.C4D5E6F7\r\n"
        "Content-Type: text/html; charset=\"us-ascii\"\r\n"
        "Content-Transfer-Encoding: quoted-printable\r\n"
        "\r\n"
        "<html><body><b>This is the HTML part.</b></body></html>\r\n"
        "------=_NextPart_000_0001_01D1A2B3.C4D5E6F7--\r\n";

    EmailParser parser;
    std::cout << "Parsing Email 1:"
        << std::endl;

    EmailMessage email1 = parser.parse(raw_email1);
    print_email_message(email1);

    std::cout << "\nParsing Email 2 (Multipart - basic parsing only):"
        << std::endl;

    EmailMessage email2 = parser.parse(raw_email2);
    print_email_message(email2);

    std::cout << "\nEmail 1 Subject: "
        << email1.get_header("Subject")
        << std::endl;
    std::cout << "Email 2 Content-Type: "
        << email2.get_header("Content-Type")
        << std::endl;
    std::cout << "Email 2 Boundary (extracted): "
        << email2.get_boundary()
        << std::endl;

    EmailMessage new_email;
    new_email.set_header(
        "From",
        "New Sender <new.sender@example.com>"
    );
    new_email.set_header(
        "To",
        "Recipient <recipient@example.com>"
    );
    new_email.set_header(
        "Subject",
        "This is a new email built from scratch!"
    );
    new_email.set_content_type("text/plain");
    new_email.set_body(
        "Hello,\n\n"
        "This email was constructed using the build() function.\n\n"
        "Regards,\nBuilder"
    );

    std::cout << "\n--- Building a New Email ---"
        << std::endl;

    std::string built_email = new_email.build();
    std::cout << built_email
        << std::endl;
    std::cout << "----------------------------"
        << std::endl;

    std::cout << "\n--- Parsing the Built Email ---"
        << std::endl;

    EmailMessage parsedbuilt_email =
        parser.parse(built_email);
    print_email_message(parsedbuilt_email);
    std::cout << "\n--- Demonstrating EmailAddress Class ---"
        << std::endl;

    EmailAddress addr1("John Doe <john.doe@example.com>");
    std::cout << "Parsing 'John Doe <john.doe@example.com>':"
        << std::endl;
    print_email_addr(addr1);

    EmailAddress addr2("jane.smith@example.org");
    std::cout << "\nParsing 'jane.smith@example.org':"
        << std::endl;
    print_email_addr(addr2);

    EmailAddress addr3(
        "\"Another User, Esq.\" <another.user@sub.domain.co.uk>"
    );

    std::cout << "\nParsing '\"Another User, Esq.\" <another.user@sub.domain.co.uk>':"
        << std::endl;
    print_email_addr(addr3);

    EmailAddress addr4("invalid-email");

    std::cout << "\nParsing 'invalid-email':"
        << std::endl;
    print_email_addr(addr4);

    EmailAddress addr5("test@localhost");

    std::cout << "\nParsing 'test@localhost':"
        << std::endl;
    print_email_addr(addr5);

    EmailAddress addr6;
    addr6.parse("custom.name <custom@example.net>");

    std::cout << "\nParsing 'custom.name <custom@example.net>' using parse method:"
        << std::endl;
    print_email_addr(addr6);

    std::cout << "Reconstructed: "
        << addr6.to_string()
        << std::endl;

    return 0;
}
