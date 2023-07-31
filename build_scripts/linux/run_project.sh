#!/bin/bash

dirName=$(dirname "$0")
source $dirName/_variables.sh

arg1=$1
arg2=$2

# echo "Source Directory=$SRC_DIR_PATH"

foundFolder=0
folderName=""
foundProject=0
projectFolderName=""
for d in $SRC_DIR_PATH/* ; do
    # echo "$d"
    folderName=$(basename ${d}) # gets the base file at the end of the path
    # echo "Checking for chapter number $arg1 in $folderName"
    if [[ "$folderName" == *"$arg1"* ]]; then
        foundFolder=1
        break
    fi
done

if [ $foundFolder -eq 0 ]; 
then
    echo "Could not find folder containing $arg1"
else
    # echo "Found Chapter Folder $SRC_DIR_PATH/$folderName"
    for d in $SRC_DIR_PATH/$folderName/* ; do
        # echo "$d"
        projectFolderName=$(basename ${d}) # gets the base file at the end of the path
        # echo "Checking for project number $arg2 in $projectFolderName"
        if [[ "$projectFolderName" == *"$arg2"* ]]; then
            foundProject=1
            break
        fi
    done
fi

if [ $foundFolder -eq 1 ];
then
    if [ $foundProject -eq 1 ];
    then
        projectPath="$SRC_DIR_PATH/$folderName/$projectFolderName/$CHAPTER_SUBFOLDERS_BUILD_FOLDER_NAME"
        # echo "Found Project $projectFolderName in $projectPath"
        executable=""
        foundExec=0
        for entry in "$projectPath"/* ; do
            # echo "Found a file $entry"
            foundExec=1
            executable=$entry
            break
        done
        if [ $foundExec -eq 1 ];
        then
            $executable
        fi
    else
        echo "Could not find Project Number $arg2 in $SRC_DIR_PATH/$folderName"
    fi
fi


