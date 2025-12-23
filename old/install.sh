#!/bin/bash

set -e  # Exit on any error

# Ensure the script is run as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

# Define installation directories
LIB_NAME="libPqSQL.so"
LIB_DIR="/usr/local/lib"
INCLUDE_DIR="/usr/local/include/PqSQL"
PKGCONFIG_DIR="/usr/lib/pkgconfig"
PKGCONFIG_FILE="PqSQL.pc"

echo "Compiling the PqSQL library..."

# Compile the library
make clean 2>/dev/null || true  # Clean previous builds if Makefile supports it
make

echo "Installing the library..."

# Create directories if they don't exist
sudo mkdir -p "$LIB_DIR"
sudo mkdir -p "$INCLUDE_DIR"
sudo mkdir -p "$PKGCONFIG_DIR"

# Copy the shared library
cp -r "$LIB_NAME" "$LIB_DIR"

# Copy header files
cp -r lib/* "$INCLUDE_DIR"
# cp -r lib/*/*.h "$INCLUDE_DIR"

# Create PqSQL.pc file
echo "Creating $PKGCONFIG_FILE..."
cat <<EOF | sudo tee "$PKGCONFIG_DIR/$PKGCONFIG_FILE" > /dev/null
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include/PqSQL

Name: PqSQL
Description: A custom library for PostgresSQL
Version: 1.0.0
Libs: -L\${libdir} -lPqSQL
Cflags: -I\${includedir}
EOF

# Update library cache
echo "Updating linker cache..."
ldconfig

echo "PqSQL library installed/updated successfully."
