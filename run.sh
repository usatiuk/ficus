#!/bin/bash


POSITIONAL_ARGS=()

QEMU_OPTS=" -no-reboot "

while [[ $# -gt 0 ]]; do
  case $1 in
    -c)
      CLEAN=true
      shift
      ;;
    -r)
      MODE="relwithdebinfo"
      shift # past argument
      ;;
    -d)
      MODE="debug"
      shift
      ;;
    -gw)
      # wait for gdb
      QEMU_OPTS="$QEMU_OPTS -S"
      shift
      ;;
    -s)
      if [ $TERM_USED ]; then
        echo "Conflicting options!"; # (todo: there must be a way to use both...)
        exit 1
      fi
      TERM_USED=true
      # serial
      QEMU_OPTS="$QEMU_OPTS -serial stdio"
      shift
      ;;
    -mon)
      if [ $TERM_USED ]; then
        echo "Conflicting options!"; # (todo: there must be a way to use both...)
        exit 1
      fi
      TERM_USED=true
      # serial
      QEMU_OPTS="$QEMU_OPTS -monitor stdio"
      shift
      ;;
    -int)
      if [ $TERM_USED ]; then
        echo "Conflicting options!"; # (todo: there must be a way to use both...)
        exit 1
      fi
      TERM_USED=true
      # serial
      QEMU_OPTS="$QEMU_OPTS -d int"
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

set -exo pipefail

if [[ $BASH_SOURCE = */* ]]; then
  cd -- "${BASH_SOURCE%/*}/" || exit
fi

if [ $CLEAN ]; then
  cmake --build cmake-build-$MODE --target clean
fi

cmake --build cmake-build-$MODE --target iso

qemu-system-x86_64 -s $QEMU_OPTS -cdrom cmake-build-$MODE/src/iso/os2.iso

