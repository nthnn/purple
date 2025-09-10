# 🧪 Netlet

Netlet is a comprehensive library designed to empower developers in building robust, scalable, and high-performance systems that fully leverage the capabilities of the Linux operating system. Inspired by modern design principles and focusing on efficiency, Netlet provides a rich set of tools and components to streamline the development of complex, concurrent, and distributed applications.

Moreover, Netlet is built with the following principles in mind:

- **🐧 Linux Native** — Designed to maximize performance and leverage unique Linux system calls and features.
- **📈 Scalability** — Components are built to handle high concurrency and large data volumes, making it suitable for demanding applications.
- **🧩 Modularity** — Netlet's features are designed as independent modules, allowing developers to pick and choose the components they need without unnecessary bloat.
- **🤝 Developer Friendly** — Clear APIs, comprehensive documentation (forthcoming), and a focus on ease of use.
- **⚡ Performance** — Optimized for speed and resource efficiency, crucial for full-time running systems. A Linux-Native Framework for Scalable Systems

> [!NOTE]
> Netlet is still underdevelopment. Do not use in production environment.

## 🛠️ Features

Netlet offers a wide array of features, meticulously crafted to address common challenges in system development:

- **🔄 Concurrency Management**
    - **Channels**
        Facilitate safe and efficient communication between concurrent processes, inspired by CSP (Communicating Sequential Processes) and similar to Golang's channels.
    - **Tasklets**
        Lightweight, user-space execution units akin to Golang's goroutines, enabling highly concurrent operations with minimal overhead.

- **⏰ Cron (Job Scheduling)**
    - **Scheduler**
        A powerful and flexible job scheduler for executing tasks at specified intervals or times.
    - **Timepoint Management**
        Define precise execution times and recurring schedules for automated processes.

- **⚙️ Environment and Configuration**
    - **Dot File Environment I/O**
        Seamlessly read and write configuration from and to dot files, a common Linux convention.
    - **JSON Handling**
        Robust parsing, building, and manipulation of JSON data for configuration, API communication, and data storage.

- **🌐 Web and Networking Utilities**
    - **Robots.txt I/O and Generation**
        Easily manage and generate robots.txt files for web crawler control.
    - **URL Parser, Builder, and Generator**
        Comprehensive utilities for handling URLs, including parsing components, building new URLs, and generating valid URL strings.
    - **Weblet (Web Server)**
    A lightweight and efficient web server component, providing the foundation for building web services and APIs directly within your Netlet application.

- **✅ Data Validation and Generation**
    - **Credit/Debit Card Validator**
        Functions for validating credit and debit card numbers (e.g., Luhn algorithm checks).
    - **Email Message and Address Builder and Generator**
        Tools for constructing and validating email addresses and full email messages.
    - **UUID String Generator**
        Generate universally unique identifiers for various purposes.
    - **Validator Functions**
        A collection of general-purpose validation functions for common data types.

- **🔧 System Utilities**
    - **Cache Management System**
        Efficiently manage in-memory caches to improve application performance.
    - **MIME Type Detection Function**
        Accurately detect MIME types of files based on their content or extension.
    - **Timestamps**
        Utilities for precise timestamp generation and manipulation.

- **🚦 Finite State Machine (FSM) Management**
    - **Transitions**
        Define and manage state transitions within your application logic.
    - **Callbacks**
        Associate custom functions with state entries, exits, and transitions, enabling powerful event-driven architectures.
