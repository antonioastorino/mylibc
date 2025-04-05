#!/bin/bash
APP_NAME="my-clib-test"
ARTIFACT_FOLDER="test/artifacts"
LOG_FILE_ERR="${ARTIFACT_FOLDER}/err.log"
COMMON_HEADER="include/common.h"
FLAGS="-Wall -Wextra -std=c2x -pedantic -g -fsanitize=address"
COMPILER="clang"
LIB=""
BUILD_DIR="build"
MAKE_FILE="Makefile"
MAIN="main-test"
SRC_EXTENSIONS=("c")
INC_EXTENSIONS=("h")

HEADER_PATHS=("include")
SRC_PATHS=("src")
if [ "$(uname -s)" = "Linux" ]; then
    FLAGS="${FLAGS} -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE"
fi
