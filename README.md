## Webserv - 42 School Project

### Overview

Webserv is a fully functional HTTP web server implemented in C++ as part of the 42 school curriculum.
It demonstrates a deep understanding of network programming, system calls, and server architecture. The project focuses on creating a lightweight, high-performance server that can handle multiple clients concurrently.

### Key Features

- HTTP/1.1 Compliance: Supports GET, POST, and DELETE methods.
- Multi-client Handling: Uses epoll for efficient concurrent connections.
- Custom Error Pages: Handles standard HTTP errors with user-friendly pages.
- Configurable Server: Supports multiple virtual hosts and custom settings.
- Static & Dynamic Content: Serves static files and handles CGI scripts.
- Robust & Secure: Includes proper request validation and resource management.

### Technical Highlights

- Deep use of system calls: socket, bind, listen, accept, read, write, close.
- Poll-based event loop for non-blocking I/O.
- C++ STL used for data structures and string manipulation.
- File and directory handling with precise resource cleanup.

### Learning Outcomes

- Understanding of the HTTP protocol and client-server interaction.
- Mastery of network programming and socket management.
- Experience building a production-like server from scratch.
- Strong debugging and problem-solving skills in low-level C++ projects.
