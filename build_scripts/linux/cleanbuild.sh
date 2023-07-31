#!/bin/bash

dirName=$(dirname "$0")
source $dirName/_variables.sh

dirName2="${BASH_SOURCE%/*}"
# echo "dirName=$dirName2"


echo "BIN_DIR_PATH=$BIN_DIR_PATH"
echo "PROJECT_ROOT_DIR_PATH=$PROJECT_ROOT_DIR_PATH"

rm -rf $BIN_DIR_PATH/.* $BIN_DIR_PATH/* 2>/dev/null

echo "Clean Successful.."