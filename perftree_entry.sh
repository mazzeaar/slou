#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"

DEPTH=$1
shift
REST=$@

"${SCRIPT_DIR}/bin/slou" -debug "$DEPTH" "$REST"