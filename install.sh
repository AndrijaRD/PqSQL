#!/bin/bash

set -e  # Exit on any error

echo "=== PqSQL Library Installation ==="
echo ""

# Check for sudo privileges
if [ "$EUID" -ne 0 ]; then
    echo "This script requires root privileges for system installation."
    echo "Please run with: sudo $0"
    exit 1
fi

# Define installation directories
LIB_NAME="libPqSQL.so"
LIB_DIR="/usr/local/lib"
INCLUDE_DIR="/usr/local/include"
PKGCONFIG_DIR="/usr/lib/pkgconfig"
PKGCONFIG_FILE="PqSQL.pc"

echo "Step 1: Compiling the library..."
make clean
if ! make; then
    echo "ERROR: Library compilation failed!"
    exit 1
fi

if [ ! -f "$LIB_NAME" ]; then
    echo "ERROR: Library file '$LIB_NAME' not found after compilation!"
    exit 1
fi

echo "✅ Compilation successful!"
echo ""

echo "Step 2: Installing library to $LIB_DIR..."
if [ -f "$LIB_DIR/$LIB_NAME" ]; then
    echo "Removing old version..."
    rm -f "$LIB_DIR/$LIB_NAME"
fi
cp "$LIB_NAME" "$LIB_DIR/"
echo "✅ Library installed."

echo "Step 3: Installing header to $INCLUDE_DIR..."
if [ -f "$INCLUDE_DIR/PqSQL.h" ]; then
    echo "Removing old header..."
    rm -f "$INCLUDE_DIR/PqSQL.h"
fi
cp "PqSQL.h" "$INCLUDE_DIR/"
echo "✅ Header installed."

echo "Step 4: Creating pkg-config file..."
if [ -f "$PKGCONFIG_DIR/$PKGCONFIG_FILE" ]; then
    echo "Removing old pkg-config file..."
    rm -f "$PKGCONFIG_DIR/$PKGCONFIG_FILE"
fi

cat > "$PKGCONFIG_DIR/$PKGCONFIG_FILE" << EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: PqSQL
Description: Lightweight PostgreSQL C++ wrapper
Version: 1.0.0
Requires: libpq
Libs: -L\${libdir} -lPqSQL
Cflags: -I\${includedir}
EOF
echo "✅ pkg-config file created."

echo "Step 5: Updating library cache..."
if command -v ldconfig >/dev/null 2>&1; then
    ldconfig
    echo "✅ Library cache updated."
else
    echo "⚠️  ldconfig not found, library cache not updated."
fi

echo ""
echo "=== Installation Complete! ==="