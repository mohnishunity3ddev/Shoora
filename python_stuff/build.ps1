Write-Output("Building Python Integration stuff..")

cd $PSScriptRoot/bin
cmake ..
msbuild.exe shoora_python.sln
Debug\python_example.exe
cd ..
