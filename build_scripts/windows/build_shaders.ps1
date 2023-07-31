. $PSScriptRoot/_variables.ps1

Write-Host "Building Shaders..."
$shaders = Get-ChildItem -Path $PSScriptRoot\..\..\assets\shaders -Filter "*.glsl" -Recurse

foreach($shader in $shaders)
{
    $shaderStage=""
    $buildShader=$true
    if ($shader.Name.EndsWith(".frag.glsl"))
    {
        $shaderStage="fragment"
    }
    elseif ($shader.Name.EndsWith(".vert.glsl"))
    {
        $shaderStage="vertex"
    }
    else
    {
        $buildShader=$false
        Write-Host "Unknown shader type: $($shader.FullName)"
    }

    if($buildShader)
    {
        $glslangValidatorPath="$env:VULKAN_SDK\bin\glslc.exe"
        $output="$($shader.Directory.FullName)\spirv\$($shader.BaseName).spv"
        $arguments="-fshader-stage=$($shaderStage) $shader -o $output"
        Start-Process -FilePath $glslangValidatorPath -ArgumentList $arguments -NoNewWindow -Wait
    }
}