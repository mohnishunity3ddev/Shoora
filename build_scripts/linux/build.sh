#!/bin/bash

dirName=$(dirname "$0")
source $dirName/_variables.sh

cmake -S $PROJECT_ROOT_DIR_PATH -B $BIN_DIR_PATH -GNinja
ninja -C $BIN_DIR_PATH
