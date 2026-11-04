#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
RESULTS_DIR="$ROOT_DIR/bench-results"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

usage() {
  echo "usage: $0 [test|bench|all]" >&2
  echo "  test  (default) Debug build + unit tests in $BUILD_DIR" >&2
  echo "  bench           Release O2/O3 benchmarks in $BUILD_DIR/bench-O{2,3}" >&2
  echo "  all             run test, then bench" >&2
}

run_tests() {
  mkdir -p "$BUILD_DIR"

  find "$BUILD_DIR" -name "*.pcm" -delete 2>/dev/null || true

  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -G Ninja

  ninja -C "$BUILD_DIR"

  ln -sfn "$BUILD_DIR/compile_commands.json" "$ROOT_DIR/compile_commands.json"

  #valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all \
  "$BUILD_DIR/tests/unit_tests/aima_unit_tests"
}

run_bench() {
  mkdir -p "$RESULTS_DIR"

  local opt dir
  for opt in O2 O3; do
    dir="$BUILD_DIR/bench-$opt"
    mkdir -p "$dir"

    cmake -S "$ROOT_DIR" -B "$dir" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-$opt -DNDEBUG" \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DBUILD_BENCHMARKS=ON \
      -G Ninja

    ninja -C "$dir" xy_bench

    "$dir/benchmarks/xy_bench" \
      --benchmark_repetitions=7 \
      --benchmark_report_aggregates_only=true \
      --benchmark_out_format=json \
      --benchmark_out="$RESULTS_DIR/bench-$opt.json"
  done
}

case "${1:-test}" in
  test)  run_tests ;;
  bench) run_bench ;;
  all)   run_tests; run_bench ;;
  -h|--help) usage ;;
  *) echo "error: unknown mode '$1'" >&2; usage; exit 1 ;;
esac
