#include <netlet/helper/url.hpp>
#include <iostream>

using namespace Netlet::Helper;

void printComponents(UrlParser parser) {
  std::cout << "Original URL: " << parser.get_original_url() << std::endl;
  std::cout << "Reconstructed URL: " << parser.build_url() << std::endl;
  std::cout << "Scheme:     " << parser.get_scheme() << std::endl;
  std::cout << "Host:       " << parser.get_host() << std::endl;
  std::cout << "Port:       "
            << (parser.get_port().empty() ? "N/A" : parser.get_port())
            << std::endl;
  std::cout << "Authority:  " << parser.get_authority() << std::endl;
  std::cout << "Origin:     " << parser.get_origin() << std::endl;
  std::cout << "Path:       " << parser.get_path() << std::endl;
  std::cout << "File Name:  "
            << (parser.get_file_name().empty() ? "N/A" : parser.get_file_name())
            << std::endl;
  std::cout << "Extension:  "
            << (parser.get_extension().empty() ? "N/A" : parser.get_extension())
            << std::endl;
  std::cout << "Fragment:   "
            << (parser.get_fragment().empty() ? "N/A" : parser.get_fragment())
            << std::endl;
  std::cout << "Is Secure:  " << (parser.is_secure() ? "Yes" : "No")
            << std::endl;
  std::cout << "Is Default Port: " << (parser.is_default_port() ? "Yes" : "No")
            << std::endl;

  std::cout << "Query Params (" << parser.get_query_params_str()
            << "):" << std::endl;

  if (parser.get_query_params().empty())
    std::cout << "    N/A" << std::endl;
  else
    for (const auto &pair : parser.get_query_params())
      std::cout << "    " << pair.first << " = " << pair.second << std::endl;

  std::cout << "-----------------------------------" << std::endl;
}

int main() {
  std::vector<std::string> urls = {
      "https://www.example.com:8080/path/to/"
      "resource?param1=value1&param2=value2#section",
      "http://localhost/index.html",
      "ftp://user:pass@ftp.example.com/pub/file.txt",
      "https://github.com/microsoft/vscode",
      "http://192.168.1.1:80/status",
      "https://www.google.com/"
      "search?q=url+parser+cpp&oq=url+parser+cpp&aqs=chrome..69i57j0l7."
      "2878j0j7&sourceid=chrome&ie=UTF-8",
      "http://example.com",
      "http://example.com/",
      "http://example.com?key=value",
      "http://example.com#fragment",
      "http://example.com/document.pdf",
      "http://example.com/folder/image.png?size=large",
      "invalid-url",
      "http://",
      "https://user:password@sub.domain.com:8080/path/to/"
      "resource?query=string&another=value#fragment"};

  for (const std::string &url : urls) {
    try {
      UrlParser parser(url);
      printComponents(parser);

      std::cout << "--- Testing new functionalities for: " << url << " ---"
                << std::endl;
      std::cout << "  Modifying URL components..." << std::endl;

      parser.set_scheme("ftp");
      parser.set_host("new.host.com");
      parser.set_port("21");
      parser.set_path("/new/path/file.txt");
      parser.set_fragment("new_section");

      std::cout << "  After setters: " << parser.build_url() << std::endl;
      printComponents(parser);

      std::cout << "  Testing query parameter management..." << std::endl;
      std::cout << "  Initial query params: " << parser.get_query_params_str()
                << std::endl;
      std::cout << "  Has query params? "
                << (parser.has_query_params() ? "Yes" : "No") << std::endl;

      parser.add_query_param("new_param", "new_value");
      parser.add_query_param("param1", "updated_value");

      std::cout << "  After adding/updating: " << parser.get_query_params_str()
                << std::endl;
      std::cout << "  Has 'new_param'? "
                << (parser.has_param("new_param") ? "Yes" : "No") << std::endl;
      std::cout << "  Value of 'param1': " << parser.get_param("param1")
                << std::endl;

      parser.remove_query_param("param2");
      std::cout << "  After removing 'param2': "
                << parser.get_query_params_str() << std::endl;
      std::cout << "  Has 'param2'? "
                << (parser.has_param("param2") ? "Yes" : "No") << std::endl;

      parser.clear_queries();
      std::cout << "  After clearing all params: "
                << parser.get_query_params_str() << std::endl;
      std::cout << "  Has query params? "
                << (parser.has_query_params() ? "Yes" : "No") << std::endl;
      std::cout << "  Reconstructed URL after param changes: "
                << parser.build_url() << std::endl;

      std::cout << "-----------------------------------" << std::endl;
    } catch (const std::invalid_argument &e) {
      std::cerr << "Error parsing URL '" << url << "': " << e.what()
                << std::endl;
      std::cerr << "-----------------------------------" << std::endl;
    }
  }

  return 0;
}
