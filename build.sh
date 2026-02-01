#!/bin/bash

set -xe 

CFLAGS="-Wall -Wextra -g `pkg-config --cflags --libs raylib` -lm"
SOURCES="../main.c ../game.c"
TARGET="main"

mkdir -p build
cd build
gcc -o $TARGET $SOURCES $CFLAGS
cd ..
