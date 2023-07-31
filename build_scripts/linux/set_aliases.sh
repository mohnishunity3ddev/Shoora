#!/bin/bash

dirName=$(dirname "$0")
source $PWD/_variables.sh

alias clean="$BUILD_SCRIPTS_DIR_PATH/cleanbuild.sh"
alias build="$BUILD_SCRIPTS_DIR_PATH/build.sh"
alias run="$BUILD_SCRIPTS_DIR_PATH/run_project.sh"
alias create="$BUILD_SCRIPTS_DIR_PATH/create_project.sh"

