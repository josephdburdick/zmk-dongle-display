#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p build
cc -std=c11 -Wall -Wextra -Werror \
  tests/stage_fsm/test_stage_fsm.c \
  boards/shields/dongle_display/widgets/stage_fsm.c \
  -o build/test_stage_fsm
./build/test_stage_fsm
