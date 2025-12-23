# PqSQL - Lightweight PostgreSQL C++ Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![PostgreSQL](https://img.shields.io/badge/PostgreSQL-15+-336791.svg)](https://www.postgresql.org)
[![Arch Linux](https://img.shields.io/badge/Arch_Linux-1793D1?logo=arch-linux&logoColor=white)](https://archlinux.org)

A simple, high-performance C++ wrapper for PostgreSQL using `libpq`. Designed specifically for Arch Linux systems.

## Features

- **Simple API** - Only 3 main functions to learn
- **Prepared Statements** - Automatic preparation for maximum performance
- **Zero Dependencies** - Just `libpq` (PostgreSQL client library)
- **Modern C++20** - Clean, type-safe interfaces
- **Arch Linux Optimized** - Built and tested on Arch Linux

## Installation (Arch Linux)

### Prerequisites
```bash
sudo pacman -S postgresql-libs base-devel
git clone https://github.com/AndrijaRD/PqSQL.git
cd PqSQL
sudo ./install.sh
```



### Quick Start
```c++
#include <PqSQL.h>
#include <iostream>

int main() {
    // Initialize connection
    if (DB::init("mydatabase") != 0) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }
    
    // Prepare statement
    std::string sql = "SELECT * FROM users WHERE id = $1";
    int stmtId = DB::prepareStatement(sql);
    
    // Execute with parameters
    DBResult result;
    if (DB::exec(stmtId, {"42"}, result) == 0) {
        std::cout << "Rows: " << result.rowCount() << std::endl;
    }
    
    return 0;
}
```

### Compile
```bash
g++ program.cpp -lPqSQL -lpq -std=c++20 -o program
```


## API

### Connection
```c++
DB::init("database", "user", "password", "host", port);
```

### Statements
```c++
int id = DB::prepareStatement("SELECT * FROM table WHERE id = $1");
DB::exec(id, {"42"}, result);
```

### Results
```c++
result.rowCount();      // Number of rows
result.columnCount();   // Number of columns  
result.getValue(0, 0);  // Get cell value
```



## BUILDING PROJECT

### Makefile Example
```makefile
CXX = g++
CXXFLAGS = -std=c++20 -Wall
LDFLAGS = -lPqSQL -lpq

app: app.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
```

### pkg-config
```bash
g++ app.cpp $(pkg-config --cflags --libs PqSQL) -o app
```

