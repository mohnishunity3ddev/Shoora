. $PSScriptRoot/_variables.ps1

$firstArg = $args[0]
$secondArg = $args[1]

if(![bool]$firstArg -and ![bool]$secondArg) {
    Write-Error "Atleast one argument required."
    return
}

if($firstArg -eq "scripts") {
    Set-Location $BUILD_SCRIPTS_DIR_PATH
    return
}

if($firstArg -eq "root") {
    Set-Location $PROJECT_ROOT_DIR_PATH
    return
}

if($firstArg -eq "bin") {
    Set-Location $BIN_DIR_PATH
    return
}

if($firstArg -eq "extern") {
    Set-Location $PROJECT_ROOT_DIR_PATH\external
    return
}

if($firstArg -eq "src") {
    Set-Location $PROJECT_ROOT_DIR_PATH\$SRC_FOLDER_NAME
    return
}

# Write-Output "Chapter [$chapterArg] Project [$projectArg] FOUND!"
