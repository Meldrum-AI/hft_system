# Meldrum-HFT / hft_system
⚡ **Low-Latency High-Frequency Trading System** ⚡

[![GitHub stars](https://img.shields.io/github/stars/Meldrum-AI/hft_system?style=flat-square)](https://github.com/Meldrum-AI/hft_system/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/Meldrum-AI/hft_system?style=flat-square)](https://github.com/Meldrum-AI/hft_system/network/members)
[![C++](https://img.shields.io/badge/C++-20-blue.svg?style=flat-square)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square)](LICENSE)

A production-grade high-frequency trading (HFT) system written in modern C++20, designed for ultra-low latency order processing, matching, and cross-process communication.

---

## ✨ Core Features

### 🔒 Lock-Free Communication Primitives
- **SPSC/MPSC/SPMC/MsgQueue**: Zero-lock, cache-aligned queues optimized for thread-to-thread communication
- **Spinlock + Atomic Operations**: Avoid context switches and kernel overhead
- **Shared Memory Containers**: Cross-process data sharing with fixed-size hash tables and slot allocation

### 📦 Memory Management
- **Slab Memory Pool**: Pre-allocated object pool to eliminate malloc/free in hot paths
- **FixedHashTable**: Open-addressing hash table with lazy deletion, optimized for string keys
- **Page-Aligned Shared Memory**: POSIX-compliant shm management with automatic mount/unmount

### 🧾 Order Book & Matching Engine
- **Price-Time Priority Matching**: Supports limit/market/IOC/FOK orders
- **Multi-Level Book Structure**: Fast bid/ask level access with `std::map` and per-level queues
- **TSC Timestamping**: Hardware cycle counters for precise latency measurement

### 📊 Market Data & Execution
- **Multi-Source Market Gateway**: SPMC broadcast queues for tick distribution
- **Low-Latency Order Gateway**: MPSC queue for strategy-to-execution order flow
- **Extensible FIX/CTP Adapter**: Ready to integrate with real-world trading venues

### 📈 Monitoring & Risk
- **System Message Bus**: Cross-module communication via `SystemMessageQueue`
- **Basic Risk Checks**: Position limits and order validation hooks
- **Structured Logging**: Spdlog integration with compile-time log levels

---

## 🛠️ Build & Run

### Prerequisites
- C++20 compatible compiler (GCC 11+ / Clang 14+)
- CMake 3.20+
- POSIX-compliant OS (Linux / WSL2 recommended)
- `spdlog` (optional, for logging)

### Build Steps
```bash
# Clone the repo
git clone https://github.com/Meldrum-AI/hft_system.git
cd hft_system

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./hft
