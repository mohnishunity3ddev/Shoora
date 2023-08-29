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
        # Write-Host "Building Shader $shader for stage $shaderStage"
        $glslangValidatorPath="$env:VULKAN_SDK\bin\glslc.exe"
        $output="$($shader.Directory.FullName)\spirv\$($shader.BaseName).spv"
        $arguments="-fshader-stage=$($shaderStage) $shader -o $output"
        Start-Process -FilePath $glslangValidatorPath -ArgumentList $arguments -NoNewWindow
        # Write-Host "Done!"
    }
}


# Set-Location $PSScriptRoot\..\..\assets\shaders
# glslc -fshader-stage=vertex triangle.vert.glsl -o spirv/triangle.vert.spv
# glslc -fshader-stage=fragment triangle.frag.glsl -o spirv/triangle.frag.spv

# glslc -fshader-stage=vertex imgui.ui.vert.glsl -o spirv/imgui.ui.vert.spv
# glslc -fshader-stage=fragment imgui.ui.frag.glsl -o spirv/imgui.ui.frag.spv

# glslc -fshader-stage=vertex wireframe.vert.glsl -o spirv/wireframe.vert.spv
# glslc -fshader-stage=fragment wireframe.frag.glsl -o spirv/wireframe.frag.spv

Write-Host "Done!"