#!/bin/bash
set -e
echo "Building DeepStream Application..."
mkdir -p build && cd build
cmake ..
make -j$(nproc)
echo "All tasks built successfully!"
