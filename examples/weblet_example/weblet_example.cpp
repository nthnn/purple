#include <purple/concurrent/tasklet.hpp>
#include <purple/cron/timepoint.hpp>
#include <purple/format/dotenv.hpp>
#include <purple/net/weblet.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

using namespace Purple::Concurrent;
using namespace Purple::Format;
using namespace Purple::Net;

Response handshake(DotEnv env, Request request,
                   std::map<std::string, std::string> parameters) {
  Response response;
  response.set_header("Content-Type", "application/json");
  response.contents = "{\"status\": \"ok\", \"message\": "
                      "\"Handshake successful from C++ NetWeb!\"}";

  if (request.cookies.count("sessionId"))
    std::cout << "  Handshake handler: Received sessionId cookie: "
              << request.cookies["sessionId"] << std::endl;
  else
    std::cout << "  Handshake handler: No sessionId cookie received."
              << std::endl;

  response.set_cookie("myTestCookie", "testValue123",
                      {{"Max-Age", "3600"}, {"HttpOnly", ""}});

  response.set_cookie("anotherCookie", "someOtherValue",
                      {{"Path", "/"}, {"Secure", ""}});

  std::cout << "  Handshake handler executed." << std::endl;
  return response;
}

Response fetch_employee(DotEnv env, Request request,
                        std::map<std::string, std::string> parameters) {
  Response response;
  response.set_header("Content-Type", "application/json");

  if (parameters.find("id") == parameters.end()) {
    response.contents = "{\"error\": \"No {id} found on the URL path\"}";
    response.status_code = 400;

    std::cout << "  Fetch Employee handler: Missing ID." << std::endl;
    return response;
  }

  std::string employee_id = parameters["id"];
  response.contents =
      std::string("{\"employee_id\": \"") + employee_id +
      "\", \"name\": \"John Doe\", \"position\": \"Software Engineer\"}";

  std::cout << "  Fetch Employee handler executed for ID: " << employee_id
            << std::endl;
  response.set_cookie("lastViewedEmployee", employee_id, {{"Max-Age", "600"}});

  return response;
}

Response upload_file_handler(DotEnv env, Request request,
                             std::map<std::string, std::string> parameters) {
  Response response;
  response.set_header("Content-Type", "application/json");

  std::ostringstream ss;
  ss << "{\"status\": \"success\", "
        "\"message\": \"Uploads processed.\", "
        "\"form_fields\": {";

  bool first_field = true;
  for (const auto &field : request.form_fields) {
    if (!first_field)
      ss << ",";

    ss << "\"" << field.first << "\": \"" << field.second << "\"";
    first_field = false;
  }
  ss << "}, \"uploaded_files\": [";

  bool first_file = true;
  if (!std::filesystem::exists("uploads")) {
    std::filesystem::create_directory("uploads");
    std::cout << "  Created 'uploads' directory." << std::endl;
  }

  for (const auto &pair : request.upload_files) {
    if (!first_file)
      ss << ",";

    const std::string &field_name = pair.first;
    const UploadedFile &file = pair.second;

    std::filesystem::path p(file.filename);
    std::string base_filename = p.filename().string();

    if (base_filename.empty())
      base_filename = "untitled_file";

    std::string save_path = "uploads/" + base_filename;
    std::string unique_save_path = save_path;

    int counter = 0;
    while (std::filesystem::exists(unique_save_path)) {
      counter++;
      unique_save_path = save_path + "_" + std::to_string(counter);
    }
    save_path = unique_save_path;

    std::ofstream outfile(save_path, std::ios::binary);
    if (outfile.is_open()) {
      outfile.write(reinterpret_cast<const char *>(file.data.data()),
                    file.data.size());
      outfile.close();

      ss << "{";
      ss << "\"field_name\": \"" << field_name << "\", ";
      ss << "\"original_filename\": \"" << file.filename << "\", ";
      ss << "\"content_type\": \"" << file.content_type << "\", ";
      ss << "\"size\": " << file.data.size() << ", ";
      ss << "\"saved_to\": \"" << save_path << "\" ";
      ss << "}";

      std::cout << "  File saved: " << save_path << std::endl;
    } else {
      std::cerr << "  Failed to save file: " << save_path << std::endl;

      ss << "{";
      ss << "\"field_name\": \"" << field_name << "\", ";
      ss << "\"original_filename\": \"" << file.filename << "\", ";
      ss << "\"error\": \"Failed to save file to '" << save_path << "'\"";
      ss << "}";
    }

    first_file = false;
  }
  ss << "]}";

  response.contents = ss.str();
  response.status_code = 200;

  return response;
}

void create_public_files() {
  std::filesystem::create_directory("public");
  std::ofstream not_found_file("public/not-found.html");

  if (not_found_file.is_open()) {
    not_found_file << "<!DOCTYPE html>"
                      "<head>"
                      "    <title>404 Not Found</title>"
                      "</head>"
                      "<body>"
                      "    <h1>404 - Not Found</h1>"
                      "    <p>The requested resource was not found.</p>"
                      "</body>"
                      "</html>";
    not_found_file.close();
  } else
    std::cerr << "Warning: Could not create public/not-found.html" << std::endl;

  std::ofstream index_file("public/index.html");
  if (index_file.is_open()) {
    index_file
        << "<!DOCTYPE html>"
           "<head>"
           "    <meta charset=\"UTF-8\" />"
           "    <meta name=\"viewport\" content=\"width=device-width, "
           "initial-scale=1.0\" />"
           "    <title>Welcome to Weblet</title>"
           "    <style>"
           "        body {"
           "            font-family: sans-serif;"
           "            margin: 40px;"
           "            background-color: #f4f4f4;"
           "            color: #333;"
           "        }"
           "        h1 {"
           "            color: #0056b3;"
           "        }"
           "        code {"
           "            background-color: #e0e0e0;"
           "            padding: 2px 4px;"
           "            border-radius: 3px;"
           "        }"
           "    </style>"
           "</head>"
           "<body>"
           "    <h1>Welcome to Weblet!</h1>"
           "    <p>This is a public index file served by your C++ backend.</p>"
           "    <p>Try these paths:</p>"
           "    <ul>"
           "        <li><a "
           "href='/api/handshake'><code>/api/handshake</code></a></li>"
           "        <li><a "
           "href='/api/employee/101'><code>/api/employee/101</code></a> "
           "(Built-in Handler)</li>"
           "        <li><a "
           "href='/api/dynamic-employee/202'><code>/api/dynamic-employee/202</"
           "code></a> (Dynamic Module Handler)</li>"
           "        <li><a href='/upload.html'><code>/upload.html</code></a> "
           "(File Upload Example)</li>"
           "        <li><a "
           "href='/nonexistent.html'><code>/nonexistent.html</code></a> (to "
           "see 404)</li>"
           "    </ul>"
           "</body>"
           "</html>";
    index_file.close();
  } else
    std::cerr << "Warning: Could not create public/index.html" << std::endl;

  std::ofstream upload_file_html("public/upload.html");
  if (upload_file_html.is_open()) {
    upload_file_html
        << "<!DOCTYPE html>"
           "<head>"
           "    <meta charset=\"UTF-8\" />"
           "    <meta name=\"viewport\" content=\"width=device-width, "
           "initial-scale=1.0\" />"
           "    <title>File Upload</title>"
           "    <style>"
           "        body {"
           "            font-family: sans-serif;"
           "            margin: 40px;"
           "            background-color: #f4f4f4;"
           "            color:#333;"
           "        }"
           "        h1 {"
           "            color:#0056b3;"
           "        }"
           "        form {"
           "            background-color: #fff;"
           "            padding: 20px;"
           "            border-radius: 8px;"
           "            box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
           "        }"
           "    </style>"
           "</head>"
           "<body>"
           "    <h1>Upload a File</h1>"
           "    <form action=\"/api/upload\" method=\"post\" "
           "enctype=\"multipart/form-data\">"
           "        <label for=\"myFile\">Choose File:</label>"
           "        <input type=\"file\" id=\"myFile\" name=\"myFile\">"
           "        <br /><br />"
           "        <label for=\"description\">Description:</label>"
           "        <input type=\"text\" id=\"description\" "
           "name=\"description\">"
           "        <br /><br />"
           "        <input type=\"submit\" value=\"Upload\">"
           "    </form>"
           "    <p><a href=\"/\">Back to Home</a></p>"
           "</body>"
           "</html>";
    upload_file_html.close();
  } else
    std::cerr << "Warning: Could not create public/upload.html" << std::endl;
}

void delete_public_files() {
  std::filesystem::remove("public/not-found.html");
  std::filesystem::remove("public/index.html");
  std::filesystem::remove("public/upload.html");
  std::filesystem::remove("public");

  if (std::filesystem::exists("uploads")) {
    for (const auto &entry : std::filesystem::directory_iterator("uploads"))
      std::filesystem::remove(entry.path());

    std::filesystem::remove("uploads");
    std::cout << "Cleaned up 'uploads' directory." << std::endl;
  }
}

int main() {
  Weblet server("0.0.0.0", 8080, false, 4, [](std::string message) {
    std::cout << "Error: " << message << std::endl;
  });

  server.handle("/api/handshake", handshake);
  server.handle("/api/employee/{id}", fetch_employee);
  server.handle("/api/upload", upload_file_handler);

  int employee_mod_id = server.add_module("./weblet_employee.so");
  if (employee_mod_id != 0) {
    server.handle("/api/dynamic-employee",
                  server.load_response(employee_mod_id, "employee_fetch"));

    server.handle("/api/dynamic-employee/{id}",
                  server.load_response(employee_mod_id, "employee_fetch"));
  } else
    std::cerr << "Could not load dynamic employee module." << std::endl;

  server.handle_public("./public");
  server.add_error_handler(404, "./public/not-found");
  create_public_files();

  std::cout << "Server is up!" << std::endl;
  server.start();

  std::this_thread::sleep_for(Purple::Cron::CronSeconds(30));

  server.stop();
  delete_public_files();

  return 0;
}
