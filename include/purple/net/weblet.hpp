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

/**
 * @file weblet.hpp
 * @author Nathanne Isip <nathanneisip@gmail.com>
 * @brief Provides the Weblet lightweight web server framework for handling HTTP
 * requests, routing, static file serving, and dynamic module loading.
 *
 * This header defines the core Weblet API including request/response
 * structures, route handling, middleware-style request handlers, error
 * handling, and dynamic shared object module support. The Weblet server is
 * designed to be lightweight, asynchronous, and modular, leveraging Purple's
 * concurrent tasklet system.
 */
#ifndef PURPLE_NET_WEBLET_HPP
#define PURPLE_NET_WEBLET_HPP

#include <purple/concurrent/channel.hpp>
#include <purple/concurrent/tasklet.hpp>
#include <purple/format/dotenv.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace Purple::Net {

using namespace Purple::Concurrent;

/**
 * @struct UploadedFile
 * @brief Represents a file uploaded through a multipart/form-data HTTP request.
 *
 * Uploaded files are stored in memory, including metadata such as filename,
 * MIME content type, and raw binary data.
 */
struct UploadedFile {
  std::string filename;      ///< Original name of the uploaded file.
  std::string content_type;  ///< MIME type of the file.
  std::vector<uint8_t> data; ///< Raw binary file contents.

  /**
   * @brief Default constructor initializes empty file metadata.
   */
  UploadedFile() : filename(""), content_type(""), data() {}
};

/**
 * @struct Request
 * @brief Represents an HTTP request received by the Weblet server.
 *
 * Encapsulates request metadata (method, headers, cookies), request body,
 * form fields, and uploaded files. Both plain text and raw binary contents
 * are supported.
 */
struct Request {
  std::string full_url;     ///< Full URL of the request (path + query).
  std::string request_path; ///< Path of the request (e.g. `/users/123`).
  std::string method;       ///< HTTP method (GET, POST, etc.).

  std::map<std::string, std::string> headers; ///< Map of request headers.
  std::map<std::string, std::string>
      cookies; ///< Map of cookies parsed from the `Cookie` header.
  std::map<std::string, std::string>
      form_fields; ///< Parsed form fields for POST requests.

  std::string contents;                   ///< Request body as plain string.
  std::vector<uint8_t> contents_in_bytes; ///< Request body as raw binary data.

  std::map<std::string, UploadedFile>
      upload_files; ///< Uploaded files (for multipart forms).

  /**
   * @brief Default constructor initializes an empty HTTP request.
   */
  Request()
      : full_url(""), request_path(""), method(""), headers(), cookies(),
        form_fields(), contents(""), contents_in_bytes(), upload_files() {}
};

/**
 * @struct Response
 * @brief Represents an HTTP response sent back to the client.
 *
 * Includes status code, message, response body, headers, and cookies.
 */
struct Response {
  std::map<std::string, std::string>
      headers; ///< HTTP headers to send with the response.
  std::map<std::string, std::string>
      cookies; ///< Cookies to set in the response.

  std::string contents;       ///< Response body.
  int status_code;            ///< HTTP status code (e.g. 200, 404, 500).
  std::string status_message; ///< HTTP status message (e.g. "OK", "Not Found").

  /**
   * @brief Constructs a default 200 OK response with no body.
   */
  Response()
      : headers(), cookies(), contents(""), status_code(200),
        status_message("OK") {}

  /**
   * @brief Sets or replaces an HTTP response header.
   * @param key Header name.
   * @param value Header value.
   */
  void set_header(const std::string &key, const std::string &value);

  /**
   * @brief Sets a cookie in the HTTP response.
   * @param name Cookie name.
   * @param value Cookie value.
   * @param attributes Optional attributes (e.g. Path, Secure, HttpOnly).
   */
  void set_cookie(const std::string &name, const std::string &value,
                  const std::map<std::string, std::string> &attributes = {});
};

/**
 * @typedef RequestHandler
 * @brief Function signature for request handlers.
 *
 * A request handler receives a Request object and a map of URL path parameters
 * with included configuration environment and returns a Response.
 */
using RequestHandler = std::function<Response(
    Purple::Format::DotEnv, Request, std::map<std::string, std::string>)>;

/**
 * @typedef RequestHandlerException
 * @brief Callback type for reporting handler or server errors.
 */
using RequestHandlerException = std::function<void(std::string)>;

/**
 * @struct Route
 * @brief Represents a registered route with regex pattern matching.
 *
 * Each route maps a regular expression path pattern and extracted path
 * parameters to a request handler.
 */
struct Route {
  std::regex path_regex; ///< Compiled regex for matching request paths.
  std::vector<std::string>
      path_names; ///< Parameter names extracted from the path.

  RequestHandler handler; ///< Handler function for the route.
};

/**
 * @struct SocketCloser
 * @brief RAII wrapper for a socket file descriptor.
 *
 * Ensures that sockets are closed automatically when the object goes out of
 * scope.
 */
struct SocketCloser {
  int fd; ///< Socket file descriptor.

  /**
   * @brief Constructs a SocketCloser for the given descriptor.
   * @param descriptor File descriptor of the socket.
   */
  SocketCloser(int descriptor) : fd(descriptor) {}

  /**
   * @brief Destructor closes the socket if valid.
   */
  ~SocketCloser();

  /**
   * @brief Implicit conversion operator to int (socket file descriptor).
   */
  operator int() const;
};

/**
 * @class WebletException
 * @brief Exception class for Weblet server errors.
 *
 * Used to signal socket, binding, listening, or other internal failures.
 */
class WebletException : public std::exception {
private:
  std::string message; ///< Error message.

public:
  /**
   * @brief Constructs a WebletException with an error message.
   * @param message The error message string.
   */
  explicit WebletException(const std::string &message) : message(message) {}

  /**
   * @brief Returns the error message.
   */
  const char *what() const noexcept override { return message.c_str(); }
};

/**
 * @class Weblet
 * @brief A lightweight, asynchronous HTTP server with routing and static file
 * support.
 *
 * Weblet provides:
 * - HTTP request parsing and response handling
 * - Route registration with path parameter extraction
 * - Static file serving and SPA (Single Page Application) support
 * - Dynamic shared object (DSO) module loading for handlers
 * - Custom error page handling
 * - Tasklet-based concurrency
 */
class Weblet {
public:
  /**
   * @brief Constructs a Weblet server instance.
   * @param host Hostname or IP to bind (e.g. "127.0.0.1").
   * @param port TCP port to listen on.
   * @param spa Enable Single Page Application (SPA) mode.
   * @param num_threads Number of tasklet threads to spawn.
   * @param handler_exception_fn Callback for reporting exceptions.
   */
  Weblet(const std::string &host, int port, bool spa, size_t num_threads,
         RequestHandlerException handler_exception_fn)
      : port(port), server_desc(-1), spa(spa), hostname(host), public_dir(),
        routes(), error_handlers(), next_mod_id(1), loaded_mods(),
        handler_exception(handler_exception_fn),
        tasklet_manager(TaskletManager(num_threads)), configuration() {}

  /**
   * @brief Destructor stops the server and unloads modules.
   */
  ~Weblet();

  /**
   * @brief Registers a request handler for a given path pattern.
   * @param path_pattern Regex-style path pattern (supports `{param}` syntax).
   * @param handler Handler function to process requests.
   */
  void handle(const std::string &path_pattern, RequestHandler handler);

  /**
   * @brief Registers a public directory for serving static files.
   * @param public_dir Filesystem path to the public directory.
   */
  void handle_public(const std::string &public_dir);

  /**
   * @brief Adds a custom error handler for a specific status code.
   * @param error_code HTTP error code (e.g. 404, 500).
   * @param filepath Path to HTML file to serve for this error.
   */
  void add_error_handler(int error_code, const std::string &filepath);

  /**
   * @brief Dynamically loads a shared object module.
   * @param shared_obj Path to the `.so` file.
   * @return Module ID if successful, 0 otherwise.
   */
  int add_module(std::string shared_obj);

  /**
   * @brief Loads a handler function from a dynamic module.
   * @param shared_mods Module ID returned by add_module().
   * @param response_name Name of the exported response function.
   * @return A RequestHandler bound to the loaded function.
   */
  RequestHandler load_response(int shared_mods, std::string response_name);

  /**
   * @brief Starts the Weblet server in asynchronous mode.
   */
  void start();

  /**
   * @brief Stops the Weblet server gracefully.
   */
  void stop();

  /**
   * @brief Checks if the server is currently running.
   * @return true if running, false otherwise.
   */
  bool is_running();

  /**
   * @brief Checks if Single Page Application (SPA) mode is enabled.
   * @return true if SPA mode is enabled.
   */
  bool is_spa() const;

  /**
   * @brief Attaches a configuration object to the current Weblet instance.
   *
   * This allows a Weblet to be parameterized using environment-style
   * configuration values provided via a `DotEnv` instance. Settings such
   * as port numbers, hostnames, SSL options, logging, and feature flags
   * can be injected into the Weblet without modifying code.
   *
   * @param config A `DotEnv` object containing configuration values.
   * The object is copied into the Weblet’s internal state.
   */
  void set_config(Purple::Format::DotEnv config);

  /**
   * @brief Retrieves the current configuration associated with this Weblet.
   *
   * Provides access to the internal `DotEnv` object that was previously
   * attached using `set_config()`. This can be used to query runtime
   * configuration values, e.g. for debugging, logging, or conditional
   * behavior.
   *
   * @return A copy of the `DotEnv` configuration object currently bound
   * to this Weblet instance.
   */
  Purple::Format::DotEnv get_config() const;

private:
  int port;             ///< TCP port number.
  int server_desc;      ///< Server socket descriptor.
  bool spa;             ///< SPA mode enabled flag.
  std::string hostname; ///< Hostname or IP to bind.

  std::string public_dir;    ///< Directory for serving static files.
  std::vector<Route> routes; ///< Registered routes.
  std::map<int, std::string> error_handlers; ///< Error handlers by code.

  int next_mod_id;                           ///< Next available module ID.
  std::map<int, void *> loaded_mods;         ///< Loaded dynamic modules.
  RequestHandlerException handler_exception; ///< Exception reporting callback.
  TaskletManager tasklet_manager;       ///< Tasklet manager for concurrency.
  Purple::Format::DotEnv configuration; ///< Configuration dot environment.

  ssize_t safe_send(int sock_desc, const std::string &data, int flags = 0);

  ssize_t safe_recv_to_vec(int sock_desc, std::vector<char> &buffer,
                           size_t len_to_read, int flags = 0);

  void parse_req_headers(std::istringstream &headers_stream, Request &request);
  void parse_url_enc_data(const std::string &body, Request &request);

  void parse_multipart_data(const std::string &body,
                            const std::string &boundary, Request &request);

  std::string build_response_str(const Response &response);
  void handle_client(int client_socket_fd);

  Response route_request(const Request &request);
  Response serve_static(const std::string &filepath);
  Response handle_error(int error_code, const std::string &message = "");
};

/**
 * @def WebletDynamicHandler
 * @brief Macro for declaring dynamic request handlers in shared modules.
 *
 * Example:
 * @code
 * WebletDynamicHandler my_handler(Request req,
 * std::map<std::string,std::string> params) { Response res; res.contents =
 * "Dynamic Response!"; return res;
 * }
 * @endcode
 */
#define WebletDynamicHandler extern "C" struct Response

} // namespace Purple::Net

#endif
