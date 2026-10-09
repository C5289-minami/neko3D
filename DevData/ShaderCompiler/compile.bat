@echo off

if not exist "Bin" mkdir "Bin"

rem --- 頂点シェーダー ---
ShaderCompiler.exe /Tvs_4_0 /FoBin\ToonVS.vso Src\ToonVS.hlsl
ShaderCompiler.exe /Tvs_4_0 /FoBin\OutlineVS.vso Src\OutlineVS.hlsl
ShaderCompiler.exe /Tvs_4_0 /FoBin\ModelToonVS_4Frame.vso Src\ModelToonVS_4Frame.hlsl
ShaderCompiler.exe /Tvs_4_0 /FoBin\ModelToonVS_NMap4Frame.vso Src\ModelToonVS_NMap4Frame.hlsl

rem --- ピクセルシェーダー ---
ShaderCompiler.exe /Tps_4_0 /FoBin\ToonPS.pso Src\ToonPS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoBin\OutlinePS.pso Src\OutlinePS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoBin\ModelToonPS.pso Src\ModelToonPS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoBin\DepthPS.pso Src\DepthPS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoBin\DepthViewPS.pso Src\DepthViewPS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoBin\PostProcessPS.pso Src\PostProcessPS.hlsl

rem --- スクリーンシェーダー --- 
ShaderCompiler.exe /Tps_4_0 /FoBin\FullScreenPS.pso Src\FullScreenPS.hlsl

echo Shader Compilation Completed!
pause