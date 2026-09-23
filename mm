#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -G Ninja

ninja -C "$BUILD_DIR"

ln -sfn "$BUILD_DIR/compile_commands.json" "$ROOT_DIR/compile_commands.json"

valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all \
  "$BUILD_DIR/tests/unit_tests/aima_unit_tests"















