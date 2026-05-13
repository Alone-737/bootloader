#!/bin/bash
export PREFIX="/usr/local/gcc-cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
make all
