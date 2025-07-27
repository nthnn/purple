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

#ifndef AETHERIUM_NET_WEBLET_HPP
#define AETHERIUM_NET_WEBLET_HPP

#include <aetherium/concurrent/channel.hpp>
#include <aetherium/concurrent/tasklet.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace Aetherium::Net {

using namespace Aetherium::Concurrent;

struct UploadedFile {
    std::string filename;
    std::string content_type;
    std::vector<uint8_t> data;

    UploadedFile() :
        filename(""),
        content_type(""),
        data() { }
};

struct Request {
    std::string full_url;
    std::string request_path;
    std::string method;

    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string> form_fields;

    std::string contents;
    std::vector<uint8_t> contents_in_bytes;

    std::map<std::string, UploadedFile> upload_files;

    Request() :
        full_url(""),
        request_path(""),
        method(""),
        headers(),
        cookies(),
        form_fields(),
        contents(""),
        contents_in_bytes(),
        upload_files() { }
};

struct Response {
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> cookies;

    std::string contents;
    int status_code;
    std::string status_message;

    Response() :
        headers(),
        cookies(),
        contents(""),
        status_code(200),
        status_message("OK") { }

    void set_header(
        const std::string& key,
        const std::string& value
    );

    void set_cookie(
        const std::string& name,
        const std::string& value,
        const std::map<std::string, std::string>& attributes = {}
    );
};

using RequestHandler = std::function<Response(
    Request,
    std::map<std::string, std::string>
)>;

using RequestHandlerException = std::function<void(
    std::string
)>;

struct Route {
    std::regex path_regex;
    std::vector<std::string> path_names;

    RequestHandler handler;
};

struct SocketCloser {
    int fd;

    SocketCloser(int descriptor) :
        fd(descriptor) { }

    ~SocketCloser();
    operator int() const;
};

class WebletException : public std::exception {
private:
    std::string message;

public:
    explicit WebletException(
        const std::string& message
    ) : message(message) { }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Weblet {
public:
    Weblet(
        const std::string& host,
        int port,
        bool spa,
        size_t num_threads,
        RequestHandlerException handler_exception_fn
    ) : port(port),
        server_desc(-1),
        spa(spa),
        hostname(host),
        public_dir(),
        routes(),
        error_handlers(),
        next_mod_id(1),
        loaded_mods(),
        handler_exception(handler_exception_fn),
        tasklet_manager(TaskletManager(num_threads)) { }

    ~Weblet();

    void handle(
        const std::string& path_pattern,
        RequestHandler handler
    );
    void handle_public(const std::string& public_dir);

    void add_error_handler(
        int error_code,
        const std::string& filepath
    );

    int add_module(std::string shared_obj);
    RequestHandler load_response(
        int shared_mods,
        std::string response_name
    );

    void start();
    void stop();

    bool is_running();
    bool is_spa() const;

private:
    int port;
    int server_desc;
    bool spa;
    std::string hostname;

    std::string public_dir;
    std::vector<Route> routes;
    std::map<int, std::string> error_handlers;

    int next_mod_id;
    std::map<int, void*> loaded_mods;
    RequestHandlerException handler_exception;
    TaskletManager tasklet_manager;

    ssize_t safe_send(
        int sock_desc,
        const std::string& data,
        int flags = 0
    );

    ssize_t safe_recv_to_vec(
        int sock_desc,
        std::vector<char>& buffer,
        size_t len_to_read,
        int flags = 0
    );

    void parse_req_headers(
        std::istringstream& headers_stream,
        Request& request
    );
    void parse_url_enc_data(
        const std::string& body,
        Request& request
    );

    void parse_multipart_data(
        const std::string& body,
        const std::string& boundary,
        Request& request
    );

    std::string build_response_str(const Response& response);
    void handle_client(int client_socket_fd);

    Response route_request(const Request& request);
    Response serve_static(const std::string& filepath);
    Response handle_error(
        int error_code,
        const std::string& message = ""
    );
};

#define WebletDynamicHandler extern "C" struct Response

}

#endif
