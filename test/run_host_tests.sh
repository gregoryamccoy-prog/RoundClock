#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$project_dir/test-output"
cd "$project_dir"

g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$project_dir/include" \
  "$project_dir/src/core_logic.cpp" \
  "$project_dir/src/face_renderer.cpp" \
  "$project_dir/test/host_tests.cpp" \
  -o "$project_dir/test-output/host_tests"

ASAN_OPTIONS=detect_leaks=0 "$project_dir/test-output/host_tests"

if command -v ffmpeg >/dev/null 2>&1; then
  ffmpeg -hide_banner -loglevel error -y \
    -i "$project_dir/test-output/clock-preview.ppm" \
    "$project_dir/test-output/clock-preview.png"
  ffmpeg -hide_banner -loglevel error -y \
    -i "$project_dir/test-output/clock-preview.png" \
    -vf scale=960:960:flags=neighbor \
    "$project_dir/test-output/clock-preview-review.png"
fi

