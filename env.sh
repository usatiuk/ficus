#!/bin/bash

if [ -z "$FICUS_ROOT" ]; then
    echo FICUS_ROOT is blank setting default!
    export FICUS_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
fi
