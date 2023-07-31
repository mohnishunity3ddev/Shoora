. $PSScriptRoot\_variables.ps1

# Allowed Colors...
# Black, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkMagenta,
# DarkYellow, Gray, DarkGray, Blue, Green, Cyan, Red, Magenta,
# Yellow, White

Write-Alert "#####Configuring MSVC ..."
$arg = $args[0]
$build = 0
if([bool]$arg) {
    if(($arg -eq "b") -or ($arg -eq "build") -or ($arg -eq "force") -or ($arg -eq "f")) {
        $build = 1
    }
}

Invoke-Expression "& cmake -S '$PROJECT_ROOT_DIR_PATH' -B '$BIN_DIR_PATH'"

if($?) {
    if($build -eq 1) {
        Write-Alert "#####Building All MSVC Projects ..."
        Invoke-Expression "& MSBuild.exe '$BIN_DIR_PATH\$PROJECT_NAME.sln' "

        if($?) {
            Write-Output "#####Build Successful."
            $arg2 = $args[1]
            if([bool]$arg2 -and (($arg2 -eq "start") -or ($arg2 -eq "s") -or ($arg2 -eq "1") )) {
                Invoke-Expression "& '$BIN_DIR_PATH\$PROJECT_NAME.sln'"
            }
        } else {
            Write-Error "#####Build Failed."
        }
    } elseif([bool]$arg) {
        Write-Warning "Argument provided. Pass 'b' / 'build' / 'force' / 'f' to build all vs projects. Skipping for now ..."
    }
} else {
    Write-Error "#####Configuration failed."
}




