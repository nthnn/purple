<p align="center">
    <img src="assets/purple-logo.png" width="120" />
</p>
<h1 align="center">Purple Framework</h1>

Purple is a comprehensive library designed to empower developers in building robust, scalable, and high-performance systems that fully leverage the capabilities of the Linux operating system. Inspired by modern design principles and focusing on efficiency, Purple provides a rich set of tools and components to streamline the development of complex, concurrent, and distributed applications.

Moreover, Purple is built with the following principles in mind:

- **🐧 Linux Native** — Designed to maximize performance and leverage unique Linux system calls and features.
- **📈 Scalability** — Components are built to handle high concurrency and large data volumes, making it suitable for demanding applications.
- **🧩 Modularity** — Purple's features are designed as independent modules, allowing developers to pick and choose the components they need without unnecessary bloat.
- **🤝 Developer Friendly** — Clear APIs, comprehensive documentation (forthcoming), and a focus on ease of use.
- **⚡ Performance** — Optimized for speed and resource efficiency, crucial for full-time running systems. A Linux-Native Framework for Scalable Systems

> [!NOTE]
> Purple is still underdevelopment. Do not use in production environment.

## 🛠️ Features

Purple offers a wide array of features, meticulously crafted to address common challenges in system development:

- **🔄 Concurrency Management**
- **⏰ Cron (Job Scheduling)**
- **⚙️ Environment and Configuration**
- **🌐 Web and Networking Utilities**
- **✅ Data Validation and Generation**
- **🔧 System Utilities**

## 🌐 Example Webserver

This program demonstrates how to use the Purple framework to create a lightweight HTTP server (`Weblet`) with environment configuration support (`DotEnv`), request/response handling, and lifecycle management (start, run, stop).

It defines a handshake handler function, configures the server, attaches a route, starts serving requests, keeps the server alive for a period of time, and then stops gracefully.

```cpp
#include <iostream>

#include <purple/cron/schedule.hpp>
#include <purple/format/dotenv.hpp>
#include <purple/net/weblet.hpp>

using namespace Purple::Cron;
using namespace Purple::Format;
using namespace Purple::Net;
using namespace std;

Response hello(DotEnv env, Request request,
                   std::map<std::string, std::string> parameters) {
  Response response;
  response.set_header("Content-Type", "text/plain");
  response.contents = "Hello, world!";

  return response;
}

int main() {
  Weblet server("0.0.0.0", 8080, false, 4, [](std::string message) {
    std::cout << "Error: " << message << std::endl;
  });

  server.handle("/", hello);
  std::cout << "Server is up!" << std::endl;
  server.start();

  std::this_thread::sleep_for(Purple::Cron::CronSeconds(30));

  server.stop();
  return 0;
}

```
