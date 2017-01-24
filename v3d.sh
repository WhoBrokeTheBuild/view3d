#!/bin/sh

FILE="$1"
if [[ "$FILE" != /* ]]; then
    FILE="$(pwd)/$FILE"
fi

SCRIPT=$(readlink -f "$0")
cd $(dirname $SCRIPT)

bin/v3d "$FILE"
