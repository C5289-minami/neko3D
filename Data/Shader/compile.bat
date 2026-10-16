@echo off
setlocal
pushd "%~dp0"
if errorlevel 1 exit /b 1
if not exist "Bin" mkdir "Bin"

rem --- 頂点シェーダー ---
ShaderCompiler.exe /Tvs_4_0 /FoBin\ModelVS.vso Src\ModelVS.hlsl
if errorlevel 1 goto :error

ShaderCompiler.exe /Tvs_4_0 /FoBin\ModelVS_Bone.vso Src\ModelVS_Bone.hlsl
if errorlevel 1 goto :error

rem --- ピクセルシェーダー ---
ShaderCompiler.exe /Tps_4_0 /FoBin\ModelToonPS.pso Src\ModelToonPS.hlsl
if errorlevel 1 goto :error

ShaderCompiler.exe /Tps_4_0 /FoBin\DepthPS.pso Src\DepthPS.hlsl
if errorlevel 1 goto :error

ShaderCompiler.exe /Tps_4_0 /FoBin\PostProcessPS.pso Src\PostProcessPS.hlsl
if errorlevel 1 goto :error

ShaderCompiler.exe /Tps_4_0 /FoBin\FullScreenPS.pso Src\FullScreenPS.hlsl
if errorlevel 1 goto :error

echo.
echo Shader Compilation Completed!
popd
exit /b 0

:error
echo.
echo Shader Compilation Failed!
popd
exit /b 1
