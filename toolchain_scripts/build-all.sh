#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

"$SCRIPT_DIR"/binutils-i686-elf.sh
"$SCRIPT_DIR"/gcc-i686-elf.sh

"$SCRIPT_DIR"/binutils-x86_64-elf.sh
"$SCRIPT_DIR"/gcc-x86_64-elf.sh
"$SCRIPT_DIR"/libstdc++-x86_64-elf.sh

"$SCRIPT_DIR"/grub.sh
"$SCRIPT_DIR"/limine.sh
