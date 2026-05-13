# WIFT
WIFT is a client-server file transfer system built around epoll and event-driven architecture.
It is designed for fast, scalable, and efficient file exchange over TCP connections.

---

## Features

- Epoll-based event loop
- Asynchronous client-server architecture
- Efficient file streaming
- Minimal overhead networking
- Linux-oriented design

---

## Architecture

WIFT consist of two main components:

- **Server** - handles multiple concurrent connections using epoll
- **Client** - sends/receives files over a persistent TCP connection

Data flow is fully event-driven to avoid blocking operations.

---

## Goal

The goal of WIFT is to explore and implement a low-level, high-performance file transfer system using Linux networking primitives.

---

## Status

🚧 In development
