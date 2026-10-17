#!/usr/bin/env bash

set -euo pipefail

usage() {
    echo "Usage: $0 --bin <json2ts> <JSON_FILES>..."
    exit 1
}

if [[ -z "$(ls Makefile 2> /dev/null)" ]]; then
    echo "error: not in project's root directory" >&2
    exit 1
fi

if [[ $# -lt 3 || "$1" != "--bin" ]]; then
    usage
fi

BIN="$2"
shift 2

if [[ ! -x "$BIN" ]]; then
    echo "error: executable not found: $BIN" >&2
    exit 1
fi

OUTPUT_DIR="test/expect"
mkdir -p "$OUTPUT_DIR"

for input in "$@"; do
    if [[ ! -f "$input" ]]; then
        echo "error: input file not found: $input" >&2
        exit 1
    fi

    filename="$(basename "$input" .json)"
    output="$OUTPUT_DIR/$filename.ts"

    echo -e ""$BIN" "$input" --output "$output" --ts"
    "$BIN" "$input" --output "$output" --ts
done
