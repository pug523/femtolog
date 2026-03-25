#!/bin/bash

scripts_dir="$(dirname "$0")"
root_dir=$(cd "$scripts_dir"/.. && pwd)

xmake f -c -m release --tests=y --toolchain=llvm --stdlib=libc++

xmake run tests [benchmark][logger] -r json -o "$root_dir"/tests/benchmarks/results/benchmark_results.json
# TODO: implement format results.py
# uv run python "$scripts_dir"/format_results.py \
#     -i="$root_dir"/tests/benchmarks/results/benchmark_results.json \
#     -o="$root_dir"/tests/benchmarks/results/benchmark_results.png \
#     --format
#     --plot=
