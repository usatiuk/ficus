#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
    exit 1
fi

if [ -z "$PREFIX" ]; then
    echo "PREFIX" is blank
    exit 1
fi

if [ -z "$SCRIPT_DIR" ]; then
    echo "SCRIPT_DIR" is blank
    exit 1
fi

GCC_VERSION=13.3.0
BINUTILS_VERSION=2.41
AUTOMAKE_VERSION=1.15.1
AUTOCONF_VERSION=2.69


cd "$SCRIPT_DIR"

mkdir -p "$SCRIPT_DIR"/automake/source
mkdir -p "$SCRIPT_DIR"/automake/build
mkdir -p "$SCRIPT_DIR"/automake/prefix

curl -L "https://ftp.gnu.org/gnu/automake/automake-${AUTOMAKE_VERSION}.tar.gz" | tar xz --strip-components 1 -C "$SCRIPT_DIR"/automake/source
(
cd "$SCRIPT_DIR"/automake/build && \
../source/configure --prefix "$SCRIPT_DIR"/automake/prefix && \
make -j$BUILD_PARALLEL install
)

export PATH="$SCRIPT_DIR/automake/prefix/bin:$PATH"

mkdir -p "$SCRIPT_DIR"/autoconf/source
mkdir -p "$SCRIPT_DIR"/autoconf/build
mkdir -p "$SCRIPT_DIR"/autoconf/prefix

curl -L "https://ftp.gnu.org/gnu/autoconf/autoconf-${AUTOCONF_VERSION}.tar.gz" | tar xz --strip-components 1 -C "$SCRIPT_DIR"/autoconf/source
(
cd "$SCRIPT_DIR"/autoconf/build && \
../source/configure --prefix "$SCRIPT_DIR"/autoconf/prefix && \
make -j$BUILD_PARALLEL install
)

export PATH="$SCRIPT_DIR/autoconf/prefix/bin:$PATH"


mkdir -p "$SCRIPT_DIR"/gcc-x86_64-ficus/source
mkdir -p "$SCRIPT_DIR"/binutils-x86_64-ficus/source

curl -L "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz" | tar xz --strip-components 1 -C "$SCRIPT_DIR"/gcc-x86_64-ficus/source

(
cd gcc-x86_64-ficus/source && \
patch -p1 -u -i "$SCRIPT_DIR"/gcc-x86_64-ficus/0001-gcc-diff.patch
)

(
cd gcc-x86_64-ficus/source/libstdc++-v3 && \
automake
)

curl -L "https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz" | tar xz --strip-components 1 -C "$SCRIPT_DIR"/binutils-x86_64-ficus/source

(
cd binutils-x86_64-ficus/source && \
patch -p1 -u -i "$SCRIPT_DIR"/binutils-x86_64-ficus/0001-binutils-diff.patch
)

(
cd binutils-x86_64-ficus/source/ld && \
automake
)