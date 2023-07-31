. $PSScriptRoot\_variables.ps1

$arg = $args[0]
$arg2 = $args[1]

Invoke-Expression "& cleanall"

if([bool]$arg) {
    if(($arg -eq "ms") -or ($arg -eq "msvc")) {
        if([bool]$arg2 -and (($arg2 -eq "start") -or ($arg2 -eq "s") -or ($arg2 -eq "1") )) {
            Invoke-Expression "& msvc build start"
        } else {
            Invoke-Expression "& msvc build"
        }
    } else {
        Write-Alert "Argument Provided. Pass ms / msvc to build msvc build. Skipping for now ..."
        return
    }
} else {
    Write-Alert "Building Ninja Files ..."
    Invoke-Expression "& build"
}
