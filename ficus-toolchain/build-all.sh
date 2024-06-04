#!/bin/bash
export SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
cd "$SCRIPT_DIR"
source buildenv.sh

set -euxo pipefail

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit 1
fi

mkdir -p "$FICUS_ROOT/sysroot"

case "$1" in
  "newlib")
    "$SCRIPT_DIR"/build-newlib.sh noconf
    ;;
  "newlib-conf")
    "$SCRIPT_DIR"/build-newlib.sh conf
    ;;
  "s2only")
    "$SCRIPT_DIR"/build-newlib.sh conf
    cd "$SCRIPT_DIR"
    "$SCRIPT_DIR"/build-s2.sh
    cd "$SCRIPT_DIR"
    ;;
  "s1only")
    "$SCRIPT_DIR"/build-s1.sh
    cd "$SCRIPT_DIR"
    ;;
  "all")
    "$SCRIPT_DIR"/build-s1.sh
    cd "$SCRIPT_DIR"
    "$SCRIPT_DIR"/build-newlib.sh conf
    cd "$SCRIPT_DIR"
    "$SCRIPT_DIR"/build-s2.sh
    cd "$SCRIPT_DIR"
    ;;
  *)
    echo "Unknown command"
    exit 1
    ;;
esac
