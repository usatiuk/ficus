#!/bin/bash

if [ -z "$FICUS_ROOT" ]; then
  echo "$FICUS_ROOT" is blank
fi

export PREFIX="$FICUS_ROOT/toolchain/gcc-x86_64-ficus-prefix/"
export TARGET=x86_64-ficus
export PATH="$PREFIX/bin:$PATH"

if [ -z "${BUILD_PARALLEL}" ]; then
  export BUILD_PARALLEL=$(nproc)
fi
