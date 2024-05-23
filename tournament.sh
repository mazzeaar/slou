#!/bin/bash

cutechess="/Users/aaronmazzetta/dev/cpp/cutechess/build/cutechess-cli"

ENGINE1_NAME="v1"
ENGINE2_NAME="current"

ENGINE1_PATH="bin/slou_v1"  # Ensure this points to the executable file
ENGINE2_PATH="bin/slou_v1"  # Ensure this points to the executable file

OUTPUT_PATH="./results.pgn"

$cutechess -engine name=$ENGINE1_NAME cmd=$ENGINE1_PATH proto='uci' -engine name=$ENGINE2_NAME cmd=$ENGINE2_PATH proto='uci' -each tc=40/60+0.1 -games 10 -repeat -concurrency 10