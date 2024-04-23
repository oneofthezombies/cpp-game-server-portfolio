#!/usr/bin/env bash

set +x
set +e

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
