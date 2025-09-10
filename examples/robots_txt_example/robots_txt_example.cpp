#include <aetherium/format/robots_txt.hpp>
#include <iostream>

int main() {
  using namespace Aetherium::Format;

  std::string robots_txt_content_1 = R"(
# This is a comment
User-agent: Googlebot
Disallow: /private/
Allow: /private/public-data/
Disallow: /admin/
Crawl-delay: 10
Host: example.com

User-agent: *
Disallow: /temp/
Allow: /
Crawl-delay: 5

Sitemap: https://www.example.com/sitemap.xml
Sitemap: https://www.example.com/another-sitemap.xml
    )";

  RobotsTxt parsed_robots_1 = RobotsTxt::parse(robots_txt_content_1);
  std::cout << "--- Parsing robots.txt content ---" << std::endl;
  std::cout << "\n--- Parsed User-Agent Blocks ---" << std::endl;

  for (const auto &block : parsed_robots_1.get_user_agent_blocks()) {
    std::cout << "  User-Agents: ";
    for (const auto &ua : block.user_agents)
      std::cout << ua << " ";

    std::cout << std::endl;
    for (const auto &rule : block.rules)
      std::cout << "    "
                << (rule.type == DirectiveType::Allow ? "Allow" : "Disallow")
                << ": " << rule.path << std::endl;

    if (!block.crawl_delay.empty())
      std::cout << "    Crawl-delay: " << block.crawl_delay << std::endl;
    if (!block.host.empty())
      std::cout << "    Host: " << block.host << std::endl;
  }

  std::cout << "\n--- Parsed Sitemaps ---" << std::endl;
  for (const auto &sitemap : parsed_robots_1.get_sitemaps())
    std::cout << "  " << sitemap << std::endl;

  std::string built_robots = parsed_robots_1.build();
  std::cout << "\n--- Building robots.txt from parsed data ---" << std::endl;
  std::cout << built_robots << std::endl;

  std::cout << "\n--- Path Allowance Checks (Googlebot) ---" << std::endl;
  std::cout << "/private/ (Googlebot): "
            << (parsed_robots_1.is_path_allowed("Googlebot", "/private/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/private/public-data/ (Googlebot): "
            << (parsed_robots_1.is_path_allowed("Googlebot",
                                                "/private/public-data/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/admin/ (Googlebot): "
            << (parsed_robots_1.is_path_allowed("Googlebot", "/admin/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/ (Googlebot): "
            << (parsed_robots_1.is_path_allowed("Googlebot", "/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/temp/ (Googlebot): "
            << (parsed_robots_1.is_path_allowed("Googlebot", "/temp/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;

  std::cout << "\n--- Path Allowance Checks (*) ---" << std::endl;
  std::cout << "/temp/ (UnknownBot): "
            << (parsed_robots_1.is_path_allowed("UnknownBot", "/temp/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/ (UnknownBot): "
            << (parsed_robots_1.is_path_allowed("UnknownBot", "/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/private/ (UnknownBot): "
            << (parsed_robots_1.is_path_allowed("UnknownBot", "/private/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;

  std::cout << "\n--- Path Allowance Checks (Non-existent bot) ---"
            << std::endl;
  std::cout << "/any-path/ (MyCustomBot): "
            << (parsed_robots_1.is_path_allowed("MyCustomBot", "/any-path/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;

  std::string robots_txt_content_2 = R"(
User-agent: TestBot
Disallow: /folder/file.html
Allow: /folder/
Disallow: /path/$
Allow: /path
    )";
  RobotsTxt parsed_robots_2 = RobotsTxt::parse(robots_txt_content_2);

  std::cout << "\n--- Path Allowance Checks (TestBot with specific rules) ---"
            << std::endl;
  std::cout << "/folder/file.html (TestBot): "
            << (parsed_robots_2.is_path_allowed("TestBot", "/folder/file.html")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/folder/another.html (TestBot): "
            << (parsed_robots_2.is_path_allowed("TestBot",
                                                "/folder/another.html")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/path (TestBot): "
            << (parsed_robots_2.is_path_allowed("TestBot", "/path")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/path/ (TestBot): "
            << (parsed_robots_2.is_path_allowed("TestBot", "/path/")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;
  std::cout << "/path/sub (TestBot): "
            << (parsed_robots_2.is_path_allowed("TestBot", "/path/sub")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;

  std::string robots_txt_content_3 = R"(
User-agent: EvilBot
Disallow: /
    )";
  RobotsTxt parsed_robots_3 = RobotsTxt::parse(robots_txt_content_3);

  std::cout << "\n--- Path Allowance Checks (EvilBot - Disallow: /) ---"
            << std::endl;
  std::cout << "/ (EvilBot): "
            << (parsed_robots_3.is_path_allowed("EvilBot", "/") ? "Allowed"
                                                                : "Disallowed")
            << std::endl;
  std::cout << "/any-path (EvilBot): "
            << (parsed_robots_3.is_path_allowed("EvilBot", "/any-path")
                    ? "Allowed"
                    : "Disallowed")
            << std::endl;

  return 0;
}
