#!/bin/bash

# Exit immediately
set -e

git clone -b mbedtls-3.6 --single-branch https://github.com/Mbed-TLS/mbedtls.git

cd mbedtls
git submodule update --init --recursive

pip3 install jinja2     # May be required depending on your OS
pip3 install jsonschema # May be required depending on your OS

mkdir build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_TESTING=OFF \
  -DENABLE_PROGRAMS=OFF \
  -DUSE_STATIC_MBEDTLS_LIBRARY=ON \
  -DUSE_SHARED_MBEDTLS_LIBRARY=OFF \
  -DGEN_FILES=ON \
  -D CMAKE_OSX_ARCHITECTURES=arm64

make

# ======================================================= Next Steps =======================================================
# Create directories /lib/mbedtls inside the Xcode project root
# Copy mbedtls/include to the new subdirectory (files too, not just references if copying to Xcode)
# Copy the mbedtls/library/mbedtls... ".a" files into the new subdirectory

# Navigate to the Xcode project "Build Settings"
# Find "Header Search Paths" and add "$(PROJECT_DIR)/fetch-json/lib/mbedtls/include"; Xcode should display the absolute path

# Navigate to the Xcode project "Build Phases"
# Find "Link Binary With Libraries," and add the ".a" files in the following order:
# 1. libmbedtls.a
# 2. libmbedx509.a
# 3. libmbedcrypto.a 

# If the files are already listed, ensure the above order to avoid "Undefined symbol" compilation errors