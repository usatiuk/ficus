#!/bin/bash

if [ -z "$OS2_ROOT" ]; then
    echo OS2_ROOT is blank setting default!
    export OS2_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
fi
