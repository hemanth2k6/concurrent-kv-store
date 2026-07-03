# Concurrent KV Store

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-064F8C.svg?style=flat&logo=cmake)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg?style=flat&logo=linux)](https://www.kernel.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg?style=flat)](LICENSE)

A multithreaded, network-enabled Key-Value database written in modern C++20, built as a lightweight, protocol-compatible replacement for Redis. It implements a subset of the RESP wire protocol from scratch, using POSIX sockets, `std::thread`, and `std::shared_mutex` to serve concurrent reads and writes with disk-backed durability via a Write-Ahead Log.

## Architecture Overview

- **RESP Protocol Parser** — Implements the REdis Serialization Protocol (RESP) at the byte level, enabling the server to communicate natively with standard Redis clients such as `redis-cli`.
- **Concurrent Connection Handling** — Accepts and services multiple simultaneous client connections using POSIX sockets, with each connection managed on a dedicated `std::thread`.
- **Reader-Writer Concurrency Control** — Uses `std::shared_mutex` to allow concurrent, lock-free reads across threads while serializing write access, maximizing throughput under read-heavy workloads.
- **Write-Ahead Log (WAL)** — Persists every write operation to disk before acknowledging the client, ensuring the in-memory store can be fully reconstructed after a crash or restart.

## Getting Started

### Prerequisites

- A C++20-compliant compiler (GCC 10+ or Clang 12+)
- CMake 3.16 or higher
- A POSIX-compliant OS (Linux or macOS)

### Build

```bash
# Clone the repository
git clone https://github.com/hemanth2k6/concurrent-kv-store.git
cd concurrent-kv-store

# Configure and build with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Run the Server

```bash
# From the build directory
./kv_store_server --port 6379
```

The server will start listening on the specified port and begin accepting client connections.

## Usage

Once the server is running, open a second terminal and connect using `redis-cli` (or any RESP-compatible client):

```bash
redis-cli -p 6379
```

```
127.0.0.1:6379> SET username hemanth
OK
127.0.0.1:6379> GET username
"hemanth"
127.0.0.1:6379> SET session_token abc123 EX 60
OK
127.0.0.1:6379> GET session_token
"abc123"
```

All writes are durably logged to the WAL before the client receives an acknowledgment, so data survives a server restart.

## Future Roadmap

- [ ] **Thread Pool Implementation** — Replace the current thread-per-connection model with a fixed-size worker pool to reduce context-switching overhead under high connection counts.
- [ ] **LRU Cache Eviction** — Introduce a configurable memory ceiling with Least Recently Used (LRU) eviction to bound resident memory usage.
- [ ] **AOF/Snapshot Compaction** — Periodic WAL compaction into point-in-time snapshots to reduce log replay time on startup.
- [ ] **Expanded RESP Command Set** — Support for additional data types (Lists, Hashes, Sets) beyond basic string key-value operations.
- [ ] **Replication Support** — Primary-replica architecture for read scaling and fault tolerance.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
