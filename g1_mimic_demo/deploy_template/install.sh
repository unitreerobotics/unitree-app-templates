#!/bin/bash
set -e  # Exit immediately on error

# Clean up old files
rm -rf build out

# Configure CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j

# Install to the out directory
cmake --install build --prefix out

echo "✔ Build & Install finished. Output in ./out"
