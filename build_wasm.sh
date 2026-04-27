#!/bin/bash
# build_wasm.sh — Compile nth_prime_wasm.cpp to WebAssembly
#
# Prerequisites:
#   Install Emscripten: https://emscripten.org/docs/getting_started/downloads.html
#   Activate: source emsdk_env.sh
#
# Usage:
#   ./build_wasm.sh

set -e

echo "Building WASM module..."

emcc -O3 \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_init_table","_query_nth_prime","_get_table_max_n_lo","_get_table_max_n_hi","_get_table_max_x_lo","_get_table_max_x_hi","_get_stride","_get_num_records","_get_result_lo","_get_result_hi","_get_time_us","_get_error","_malloc","_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=67108864 \
    -s ENVIRONMENT=web \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='NthPrimeModule' \
    -s NO_EXIT_RUNTIME=1 \
    --no-entry \
    -o docs/nth_prime.js \
    nth_prime_wasm.cpp

echo ""
echo "Done! Output:"
echo "  docs/nth_prime.js   (WASM glue)"
echo "  docs/nth_prime.wasm (WASM binary)"
echo ""
echo "Test locally:"
echo "  cd docs && python3 -m http.server 8080"
