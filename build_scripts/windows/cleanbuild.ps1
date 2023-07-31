# import all variables from _variables.ps1
. $PSScriptRoot/_variables.ps1

Write-Alert "Cleaning ..."

# Remove all contents of root bin directory
Remove-Item "$BIN_DIR_PATH\*" -Recurse -Force

Write-Success "Cleaning Done!"