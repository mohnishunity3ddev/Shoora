. $PSScriptRoot/_variables.ps1

$ExeFolderPath = "$BIN_DIR_PATH\$SRC_FOLDER_NAME"
$DataFolderPath = "$DATA_DIR_PATH"
Write-Output "ExeFolderPath: $ExeFolderPath"

Set-Location $DataFolderPath
# $exe = Get-ChildItem $ExeFolderPath -Recurse -Include *.exe
# Write-Output "Exe: $exe"
Invoke-Expression "& `"$ASSETS_FROM_BIN_RELATIVE_PATH\Debug\$EXE_NAME`""

goto src

# Write-Output "Chapter [$chapterArg] Project [$projectArg] FOUND!"
