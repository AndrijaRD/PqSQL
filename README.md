# PqSQL - Lightweight PostgreSQL C++ Library

A simple, efficient C++ wrapper for PostgreSQL using libpq.

## Features

- Simple API with only 3 main functions
- Prepared statements for performance
- Automatic parameter counting
- Thread-safe (connection pool coming soon)
- No external dependencies besides libpq

## Installation

### Prerequisites

- PostgreSQL client library: `libpq-dev`
- C++20 compatible compiler

### From Source

```bash
git clone https://github.com/AndrijaRD/PqSQL.git
cd PqSQL
make
sudo ./install.sh
```
