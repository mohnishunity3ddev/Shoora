# all variables used in ps1 files
Set-Variable -Name PROJECT_NAME -Value "Shoora"
Set-Variable -Name EXE_NAME -Value "ShuraEngine_Debug.exe"

Set-Variable -Name BUILD_SCRIPTS_DIR_PATH -Value $PSScriptRoot
Set-Variable -Name PROJECT_ROOT_PATH_RELATIVE_FROM_SCRIPT -Value ..\..
Set-Variable -Name BUILD_FOLDER_NAME -Value bin
Set-Variable -Name SRC_FOLDER_NAME -Value engine
Set-Variable -Name BUILD_SCRIPTS_FOLDER_NAME -Value build_scripts
Set-Variable -Name ASSETS_FOLDER_NAME -Value assets
Set-Variable -Name ASSETS_FROM_BIN_RELATIVE_PATH -Value ..\bin\engine

Set-Variable -Name PROJECT_ROOT_DIR_PATH -Value $BUILD_SCRIPTS_DIR_PATH\$PROJECT_ROOT_PATH_RELATIVE_FROM_SCRIPT
Set-Variable -Name BIN_DIR_PATH -Value $PROJECT_ROOT_DIR_PATH\$BUILD_FOLDER_NAME
Set-Variable -Name SRC_DIR_PATH -Value $PROJECT_ROOT_DIR_PATH\$SRC_FOLDER_NAME
Set-Variable -Name DATA_DIR_PATH -Value $PROJECT_ROOT_DIR_PATH\$ASSETS_FOLDER_NAME
Set-Variable -Name BUILD_SCRIPTS_DIR_PATH -Value $PROJECT_ROOT_DIR_PATH\$BUILD_SCRIPTS_FOLDER_NAME

# functions
function GetFullPath{
    param (
        [ValidateNotNullOrEmpty()]
        [string]$relativePath
    )

    $absPath = (Get-Item $relativePath).FullName
    return $absPath
}

# Allowed Colors...
# Black, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkMagenta,
# DarkYellow, Gray, DarkGray, Blue, Green, Cyan, Red, Magenta,
# Yellow, White
function LogOutput {
    param (
        [ValidateNotNullOrEmpty()]
        [string]$msg,
        [System.ConsoleColor]$fgcolor,
        [System.ConsoleColor]$backcolor
    )

    if($backcolor -eq $null) {
        $backcolor = [System.ConsoleColor]"Black"
    }

    if($fgcolor -eq $null) {
        $fgcolor = [System.ConsoleColor]"Gray"
    }

    $msg | write-host -fore $fgcolor -back $backcolor;
}

function Write-Success {
    param (
        [ValidateNotNullOrEmpty()]
        [string]$msg
    )

    LogOutput $msg Green
}

function Write-Alert {
    param (
        [ValidateNotNullOrEmpty()]
        [string]$msg
    )

    LogOutput $msg Black DarkRed
}

function Write-Yellow {
    param (
        [ValidateNotNullOrEmpty()]
        [string]$msg
    )

    LogOutput $msg Yellow Black
}