/*
 * Copyright (c) 2025 - Nathanne Isip
 * This file is part of Purple.
 *
 * Purple is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Purple is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Purple. If not, see <https://www.gnu.org/licenses/>.
 */

#include <purple/concurrent/tasklet.hpp>
#include <purple/net/mime.hpp>
#include <purple/net/weblet.hpp>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>

#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Purple::Net {

void Response::set_header(const std::string &key, const std::string &value) {
  this->headers[key] = value;
}

void Response::set_cookie(
    const std::string &name, const std::string &value,
    const std::map<std::string, std::string> &attributes) {
  std::string cookieString = name + "=" + value;
  for (const auto &attr : attributes) {
    cookieString += "; " + attr.first;

    if (!attr.second.empty())
      cookieString += "=" + attr.second;
  }

  this->cookies[name] = cookieString;
}

SocketCloser::~SocketCloser() {
  if (this->fd != -1)
    close(this->fd);
}

SocketCloser::operator int() const { return this->fd; }

Weblet::~Weblet() {
  this->stop();

  for (auto const &[id, handle] : this->loaded_mods)
    if (handle)
      dlclose(handle);
}

void Weblet::handle(const std::string &path_pattern, RequestHandler handler) {
  std::string regexPattern = path_pattern;
  std::vector<std::string> path_names;

  std::regex paramRegex(R"(\{([a-zA-Z0-9_]+)\})");
  std::sregex_iterator it(path_pattern.begin(), path_pattern.end(), paramRegex);

  std::sregex_iterator end;
  while (it != end) {
    std::smatch match = *it;
    path_names.push_back(match[1].str());

    regexPattern = std::regex_replace(regexPattern, paramRegex, "([^/]*)",
                                      std::regex_constants::format_first_only);
    it++;
  }

  regexPattern = "^" + regexPattern + "$";
  this->routes.push_back({std::regex(regexPattern), path_names, handler});
}

void Weblet::handle_public(const std::string &public_dir) {
  this->public_dir = public_dir;
}

void Weblet::add_error_handler(int error_code, const std::string &filepath) {
  this->error_handlers[error_code] = filepath;
}

int Weblet::add_module(std::string shared_obj) {
  void *handle = dlopen(shared_obj.c_str(), RTLD_LAZY);

  if (!handle)
    return 0;

  int id = this->next_mod_id++;
  this->loaded_mods[id] = handle;

  return id;
}

RequestHandler Weblet::load_response(int shared_mods,
                                     std::string response_name) {
  if (this->loaded_mods.find(shared_mods) == this->loaded_mods.end() ||
      !this->loaded_mods[shared_mods]) {
    this->handler_exception("Shared module with ID " +
                            std::to_string(shared_mods) +
                            " not found or invalid");

    return [](Purple::Format::DotEnv, Request,
              std::map<std::string, std::string>) -> Response {
      Response res;
      res.status_code = 500;
      res.status_message = "Internal Server Error";
      res.contents = "Error: Dynamic module not loaded.";

      return res;
    };
  }

  void *handle = this->loaded_mods[shared_mods];
  typedef Response (*DynamicHandlerPtr)(Purple::Format::DotEnv, Request,
                                        std::map<std::string, std::string>);

  DynamicHandlerPtr func_ptr =
      (DynamicHandlerPtr)dlsym(handle, response_name.c_str());

  if (!func_ptr) {
    this->handler_exception("Error finding function '" + response_name +
                            "' in module ID " + std::to_string(shared_mods) +
                            ": " + std::string(dlerror()));

    return [](Purple::Format::DotEnv, Request,
              std::map<std::string, std::string>) -> Response {
      Response res;
      res.status_code = 500;
      res.status_message = "Internal Server Error";
      res.contents = "Error: Dynamic handler function not found.";

      return res;
    };
  }

  return RequestHandler(func_ptr);
}

void Weblet::start() {
  Purple::Concurrent::go<std::function<void()>>(&this->tasklet_manager, [this] {
    server_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (server_desc == -1)
      throw WebletException("Socket failed");

    int opt = 1;
    if (setsockopt(this->server_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
      close(this->server_desc);
      throw WebletException("Socket control behavior error");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr =
        (this->hostname == "localhost" || this->hostname == "127.0.0.1")
            ? INADDR_ANY
            : inet_addr(this->hostname.c_str());
    address.sin_port = htons(this->port);

    if (bind(this->server_desc, (struct sockaddr *)&address, sizeof(address)) <
        0) {
      close(this->server_desc);
      throw WebletException("Socket binding failed");
    }

    if (listen(this->server_desc, 10) < 0) {
      close(this->server_desc);
      throw WebletException("Socket listening failed");
    }

    while (true) {
      sockaddr_in client_address;
      socklen_t client_addr_len = sizeof(client_address);

      int accepted_fd = accept(server_desc, (struct sockaddr *)&client_address,
                               &client_addr_len);

      if (accepted_fd < 0) {
        if (errno == EBADF || errno == EINVAL)
          break;

        this->handler_exception("Failed to accept socket: " +
                                std::string(strerror(errno)));
        continue;
      }

      SocketCloser client_socket(accepted_fd);
      this->handle_client(static_cast<int>(client_socket));
    }

    if (this->server_desc != -1) {
      close(this->server_desc);
      this->server_desc = -1;
    }
  });
}

void Weblet::stop() {
  if (this->server_desc != -1) {
    shutdown(this->server_desc, SHUT_RDWR);
    close(this->server_desc);

    this->server_desc = -1;
  }

  this->tasklet_manager.wait_for_completion();
}

bool Weblet::is_running() { return this->server_desc != -1; }

ssize_t Weblet::safe_send(int sock_desc, const std::string &data, int flags) {
  size_t total_sent = 0;
  size_t len = data.length();

  while (total_sent < len) {
    ssize_t bytes_sent =
        send(sock_desc, data.c_str() + total_sent, len - total_sent, flags);

    if (bytes_sent <= 0) {
      if (bytes_sent == 0)
        this->handler_exception("Peer closed connection unexpectedly");
      else
        this->handler_exception("Connection failed safe send function failed");

      return -1;
    }

    total_sent += bytes_sent;
  }

  return total_sent;
}

ssize_t Weblet::safe_recv_to_vec(int sock_desc, std::vector<char> &buffer,
                                 size_t len_to_read, int flags) {
  size_t start_pos = buffer.size();
  buffer.resize(start_pos + len_to_read);

  size_t total_received = 0;
  while (total_received < len_to_read) {
    ssize_t bytes_received =
        recv(sock_desc, buffer.data() + start_pos + total_received,
             len_to_read - total_received, flags);

    if (bytes_received <= 0) {
      buffer.resize(start_pos + total_received);

      if (bytes_received == 0)
        this->handler_exception("Peer closed connection");
      else
        this->handler_exception("Safe receiving mechanism vector failed");

      return bytes_received;
    }

    total_received += bytes_received;
  }

  buffer.resize(start_pos + total_received);
  return total_received;
}

void Weblet::parse_req_headers(std::istringstream &headers_stream,
                               Request &request) {
  std::string line;
  while (std::getline(headers_stream, line) && line != "\r") {
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      std::string header_name = line.substr(0, colon_pos);
      std::string header_value = line.substr(colon_pos + 2);

      if (!header_value.empty() && header_value.back() == '\r')
        header_value.pop_back();

      request.headers[header_name] = header_value;
      if (header_name == "Cookie") {
        std::istringstream cookie_stream(header_value);
        std::string cookie_pair;
        while (std::getline(cookie_stream, cookie_pair, ';')) {
          size_t eq_pos = cookie_pair.find('=');

          if (eq_pos != std::string::npos) {
            std::string cookie_name = cookie_pair.substr(0, eq_pos);
            std::string cookie_value = cookie_pair.substr(eq_pos + 1);

            cookie_name.erase(0, cookie_name.find_first_not_of(" \t"));
            cookie_name.erase(cookie_name.find_last_not_of(" \t") + 1);
            cookie_value.erase(0, cookie_value.find_first_not_of(" \t"));
            cookie_value.erase(cookie_value.find_last_not_of(" \t") + 1);

            request.cookies[cookie_name] = cookie_value;
          }
        }
      }
    }
  }
}

void Weblet::parse_url_enc_data(const std::string &body, Request &request) {
  std::istringstream form_data_stream(body);
  std::string pair;

  auto url_decode =
      [exception_fn = this->handler_exception](const std::string &encoded_str) {
        std::string decoded_str;
        for (size_t i = 0; i < encoded_str.length(); ++i) {
          if (encoded_str[i] == '+')
            decoded_str += ' ';
          else if (encoded_str[i] == '%' && i + 2 < encoded_str.length()) {
            try {
              decoded_str += static_cast<char>(
                  std::stoi(encoded_str.substr(i + 1, 2), nullptr, 16));
              i += 2;
            } catch (const std::exception &e) {
              exception_fn("Malformed URL encoding encountered: " +
                           encoded_str.substr(i, 3));
              decoded_str += encoded_str[i];
            }
          } else
            decoded_str += encoded_str[i];
        }

        return decoded_str;
      };

  while (std::getline(form_data_stream, pair, '&')) {
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      std::string key = pair.substr(0, eq_pos);
      std::string value = pair.substr(eq_pos + 1);

      request.form_fields[url_decode(key)] = url_decode(value);
    }
  }
}

void Weblet::parse_multipart_data(const std::string &body,
                                  const std::string &boundary,
                                  Request &request) {
  std::string delimiter = "--" + boundary;
  std::string_view body_view(body);

  size_t current_pos = 0;
  while (true) {
    size_t part_start_pos = body_view.find(delimiter, current_pos);

    if (part_start_pos == std::string::npos)
      break;

    part_start_pos += delimiter.length();
    if (body_view.length() >= part_start_pos + 2 &&
        body_view.substr(part_start_pos, 2) == "--")
      break;

    if (body_view.length() >= part_start_pos + 2 &&
        body_view.substr(part_start_pos, 2) == "\r\n")
      part_start_pos += 2;
    else {
      this->handler_exception("Malformed multipart part: boundary not followed "
                              "by CRLF; will be skipped");

      current_pos = part_start_pos;
      continue;
    }

    size_t part_end_pos = body_view.find(delimiter, part_start_pos);

    if (part_end_pos == std::string::npos) {
      this->handler_exception("Malformed multipart body: part without end "
                              "delimiter; skipping remaining body");
      break;
    }

    std::string_view part_content_view =
        body_view.substr(part_start_pos, part_end_pos - part_start_pos);
    size_t headers_end = part_content_view.find("\r\n\r\n");

    if (headers_end == std::string::npos) {
      this->handler_exception(
          "Malformed multipart part: no header-body separator; skipping part");

      current_pos = part_end_pos;
      continue;
    }

    std::string_view part_headers_view =
        part_content_view.substr(0, headers_end);
    std::string_view part_body_view = part_content_view.substr(headers_end + 4);

    if (part_body_view.length() >= 2 &&
        part_body_view.substr(part_body_view.length() - 2) == "\r\n")
      part_body_view = part_body_view.substr(0, part_body_view.length() - 2);

    std::map<std::string, std::string> part_headers;
    std::string part_headers_string(part_headers_view);
    std::istringstream part_headers_stream(part_headers_string);

    std::string header_line;
    while (std::getline(part_headers_stream, header_line) &&
           header_line != "\r") {
      size_t colon_pos = header_line.find(':');

      if (colon_pos != std::string::npos) {
        std::string header_name = header_line.substr(0, colon_pos);
        std::string header_value = header_line.substr(colon_pos + 2);

        if (!header_value.empty() && header_value.back() == '\r')
          header_value.pop_back();

        part_headers[header_name] = header_value;
      }
    }

    if (part_headers.count("Content-Disposition")) {
      std::string disposition_str = part_headers["Content-Disposition"];

      std::smatch match_name;
      std::regex name_regex("name=\"([^\"]+)\"");
      std::string field_name_str;

      if (std::regex_search(disposition_str, match_name, name_regex)) {
        field_name_str = match_name[1].str();
      } else {
        this->handler_exception("Multipart part Content-Disposition without "
                                "'name' attribute; skipping part");

        current_pos = part_end_pos;
        continue;
      }

      std::smatch match_filename;
      std::regex filename_regex("filename=\"([^\"]+)\"");

      if (std::regex_search(disposition_str, match_filename, filename_regex)) {
        UploadedFile file;
        file.filename = match_filename[1].str();
        file.content_type = part_headers.count("Content-Type")
                                ? part_headers["Content-Type"]
                                : "application/octet-stream";
        file.data.assign(part_body_view.begin(), part_body_view.end());

        request.upload_files[field_name_str] = file;
      } else
        request.form_fields[field_name_str] = std::string(part_body_view);
    } else
      this->handler_exception(
          "Multipart part without Content-Disposition header; skipping part");

    current_pos = part_end_pos;
  }
}

std::string Weblet::build_response_str(const Response &response) {
  std::ostringstream response_stream;

  response_stream << "HTTP/1.1 " << response.status_code << " "
                  << response.status_message << "\r\n";
  response_stream << "Content-Length: " << response.contents.length() << "\r\n";

  for (const auto &header : response.headers)
    response_stream << header.first << ": " << header.second << "\r\n";

  for (const auto &cookie : response.cookies)
    response_stream << "Set-Cookie: " << cookie.second << "\r\n";

  response_stream << "\r\n";
  response_stream << response.contents;

  return response_stream.str();
}

void Weblet::handle_client(int client_socket_fd) {
  std::vector<char> raw_request_bytes;
  raw_request_bytes.reserve(4096);

  size_t total_received_bytes = 0;
  size_t header_end_pos = std::string::npos;

  const size_t MAX_HEADER_SIZE = 16384;
  while (header_end_pos == std::string::npos &&
         total_received_bytes < MAX_HEADER_SIZE) {
    if (raw_request_bytes.capacity() - total_received_bytes < 4096)
      raw_request_bytes.reserve(raw_request_bytes.capacity() + 4096);
    raw_request_bytes.resize(total_received_bytes + 4096);

    ssize_t bytes_read_chunk =
        recv(client_socket_fd, raw_request_bytes.data() + total_received_bytes,
             4096, 0);

    if (bytes_read_chunk <= 0) {
      if (total_received_bytes == 0)
        return;

      this->handler_exception(
          "Connection closed or error during header read after " +
          std::to_string(total_received_bytes) + " bytes");
      break;
    }

    total_received_bytes += bytes_read_chunk;
    raw_request_bytes.resize(total_received_bytes);

    std::string_view current_data_view(raw_request_bytes.data(),
                                       total_received_bytes);
    header_end_pos = current_data_view.find("\r\n\r\n");
  }

  Request request;
  Response response;

  if (header_end_pos == std::string::npos) {
    this->handler_exception("Headers too large or malformed");
    response = this->handle_error(
        400, "Bad Request: Request headers too large or malformed.");

    this->safe_send(client_socket_fd, build_response_str(response));
    return;
  }

  std::string_view full_request_view(raw_request_bytes.data(),
                                     total_received_bytes);
  std::string_view request_body_initial_view =
      full_request_view.substr(header_end_pos + 4);
  std::string request_headers_string(
      full_request_view.substr(0, header_end_pos));

  std::istringstream headers_only_stream(request_headers_string);
  std::string first_line;

  std::getline(headers_only_stream, first_line);

  std::istringstream first_line_stream(first_line);
  first_line_stream >> request.method >> request.request_path;

  if (!request.request_path.empty() && request.request_path.back() == '\r')
    request.request_path.pop_back();

  request.full_url = request.request_path;
  this->parse_req_headers(headers_only_stream, request);

  long content_length = 0;
  if (request.headers.count("Content-Length"))
    try {
      content_length = std::stol(request.headers["Content-Length"]);
    } catch (const std::exception &e) {
      this->handler_exception("Error parsing Content-Length: " +
                              std::string(e.what()));

      response =
          handle_error(400, "Bad Request: Invalid Content-Length header.");
      this->safe_send(client_socket_fd, this->build_response_str(response));

      return;
    }

  std::string request_body_str(request_body_initial_view);
  size_t body_already_read = request_body_initial_view.length();

  if (static_cast<size_t>(content_length) > body_already_read) {
    size_t remaining_bytes_to_read = content_length - body_already_read;
    raw_request_bytes.reserve(total_received_bytes + remaining_bytes_to_read);

    ssize_t read_result = this->safe_recv_to_vec(
        client_socket_fd, raw_request_bytes, remaining_bytes_to_read);

    if (read_result < (ssize_t)remaining_bytes_to_read) {
      if (read_result == -1) {
        this->handler_exception("Failed to read complete request body");
        response = this->handle_error(
            500, "Internal Server Error: Failed to read request body.");

        this->safe_send(client_socket_fd, this->build_response_str(response));
        return;
      }

      this->handler_exception("Connection closed during body read; expected " +
                              std::to_string(remaining_bytes_to_read) +
                              ", got " + std::to_string(read_result) +
                              " more bytes");

      response =
          this->handle_error(400, "Bad Request: Incomplete request body.");

      this->safe_send(client_socket_fd, this->build_response_str(response));
      return;
    }

    request_body_str.append(raw_request_bytes.data() + total_received_bytes,
                            read_result);
  }

  if (request.headers.count("Content-Type")) {
    std::string content_type = request.headers["Content-Type"];

    if (content_type.rfind("multipart/form-data", 0) == 0) {
      std::smatch match;
      std::regex boundary_regex("boundary=([^;]+)");

      if (std::regex_search(content_type, match, boundary_regex)) {
        std::string boundary = match[1].str();
        this->parse_multipart_data(request_body_str, boundary, request);
      } else {
        this->handler_exception("Multipart form-data without boundary");
        response = this->handle_error(
            400,
            "Bad Request: Malformed multipart/form-data (missing boundary).");

        this->safe_send(client_socket_fd, this->build_response_str(response));
        return;
      }
    } else if (content_type.rfind("application/x-www-form-urlencoded", 0) ==
               0) {
      request.contents = request_body_str;
      this->parse_url_enc_data(request_body_str, request);
    } else
      request.contents = request_body_str;
  } else
    request.contents = request_body_str;

  this->safe_send(client_socket_fd,
                  this->build_response_str(this->route_request(request)));
}

Response Weblet::route_request(const Request &request) {
  for (const Purple::Net::Route &route : this->routes) {
    std::smatch match;

    if (std::regex_match(request.request_path, match, route.path_regex)) {
      std::map<std::string, std::string> parameters;

      for (size_t i = 0; i < route.path_names.size(); ++i) {
        if (i + 1 < match.size()) {
          std::string paramValue = match[i + 1].str();

          if (paramValue != "")
            parameters[route.path_names[i]] = paramValue;
        }
      }

      return route.handler(this->configuration, request, parameters);
    }
  }

  if (!this->public_dir.empty()) {
    std::string requested_path = request.request_path;
    if (requested_path == "/" || requested_path.empty())
      requested_path = "/index.html";

    std::string filepath = this->public_dir + requested_path;
    if (std::filesystem::exists(filepath) &&
        std::filesystem::is_regular_file(filepath))
      return this->serve_static(filepath);
    else if (this->spa) {
      std::string filename_part;
      size_t last_slash = requested_path.rfind('/');

      if (last_slash != std::string::npos)
        filename_part = requested_path.substr(last_slash + 1);
      else
        filename_part = requested_path;

      std::string spa_index_path = this->public_dir + "/index.html";
      bool is_asset_request = (filename_part.find('.') != std::string::npos);

      if (!is_asset_request && std::filesystem::exists(spa_index_path) &&
          std::filesystem::is_regular_file(spa_index_path))
        return this->serve_static(spa_index_path);
    }
  }

  return this->handle_error(404);
}

Response Weblet::serve_static(const std::string &filepath) {
  Response response;
  std::ifstream file(filepath, std::ios::binary);

  if (file.is_open()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    response.contents = content;
    response.status_code = 200;
    response.status_message = "OK";

    response.set_header("Content-Type", get_mime_type(filepath));
  } else
    return this->handle_error(500, "Could not read file: " + filepath);

  return response;
}

Response Weblet::handle_error(int error_code, const std::string &message) {
  Response response;
  response.status_code = error_code;
  response.status_message = "Not Found";

  if (error_code == 404)
    response.status_message = "Not Found";
  else if (error_code == 500)
    response.status_message = "Internal Server Error";

  if (this->error_handlers.count(error_code)) {
    std::string errorfilepath = this->error_handlers[error_code];
    std::ifstream file(errorfilepath);

    if (file.is_open()) {
      std::string content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

      response.contents = content;
      response.status_message = "Error Page";
      response.set_header("Content-Type", "text/html");
    } else {
      response.contents =
          "<h1>" + std::to_string(error_code) +
          " - Error</h1><p>Failed to load error page: " + errorfilepath +
          "</p>";

      if (!message.empty())
        response.contents += "<p>" + message + "</p>";
      response.set_header("Content-Type", "text/html");
    }
  } else {
    response.set_header("Content-Type", "text/plain");
    response.contents =
        "Error " + std::to_string(error_code) + ": " +
        (message.empty() ? "An unexpected error occurred." : message);
  }

  return response;
}

bool Weblet::is_spa() const { return this->spa; }

void Weblet::set_config(Purple::Format::DotEnv config) {
  this->configuration = config;
}

Purple::Format::DotEnv Weblet::get_config() const {
  return this->configuration;
}

} // namespace Purple::Net
