#!/bin/bash

dirName=$(dirname "$0")
source $dirName/_variables.sh

projNameArg=$1
chapterNumber=$2

foundFolder=0
folderPath=""
for d in $SRC_DIR_PATH/* ; do
    # echo "$d"
    folderPath=$d
    folderName=$(basename ${d}) # gets the base file at the end of the path
    # echo "Checking for chapter number $arg1 in $folderName"
    if [[ "$folderName" == *"$chapterNumber"* ]]; then
        foundFolder=1
        break
    fi
done

if [ $foundFolder -eq 1 ];
then
    mkdir $projNameArg
    mkdir $projNameArg/bin
    mkdir $projNameArg/src
    echo "#include <iostream>"$'\n'"int main() {"$'\n'"    std::cout << \"Hello, World! from $projNameArg\n\";"$'\n'"    return 0;"$'\n'"}" > $projNameArg/src/main.cpp
    echo -e "cmake_minimum_required(VERSION 3.22.0)\n\nproject($projNameArg)\n\ninclude(../../../cmake_macros/prac.cmake)\n\nSETUP_APP($projNameArg \"Chapter$chapterNumber\")" > $projNameArg/CMakeLists.txt

    mv $projNameArg $folderPath

    echo -e "\nadd_subdirectory(src/Chapter$chapterNumber/$projNameArg)" >> $PROJECT_ROOT_DIR_PATH/CMakeLists.txt
fi

