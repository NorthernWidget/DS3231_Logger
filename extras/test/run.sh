#!/bin/sh
# Output-regression test for DS3231_Logger: compiles src/DS3231_Logger.cpp
# against the NW_Core stubs and prints every format of formatTime() for fixed
# register images. Usage: ./run.sh [--record]
cd "$(dirname "$0")" || exit 1
exec ../../../NW_Core/extras/test/run_library.sh ds3231_test "$@"
