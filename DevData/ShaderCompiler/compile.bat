ShaderCompiler.exe /Tvs_4_0 /FoToonVS.vso ToonVS.hlsl
ShaderCompiler.exe /Tps_4_0 /FoToonPS.pso ToonPS.hlsl

ShaderCompiler.exe /Tps_4_0 /FoOutlinePS.pso OutlinePS.hlsl
ShaderCompiler.exe /Tvs_4_0 /FoOutlineVS.vso OutlineVS.hlsl

ShaderCompiler.exe /Tps_4_0 /FoModelToonPS.pso ModelToonPS.hlsl

ShaderCompiler.exe /Tvs_4_0 /FoModelToonVS_4Frame.vso ModelToonVS_4Frame.hlsl
ShaderCompiler.exe /Tvs_4_0 /FoModelToonVS_NMap4Frame.vso ModelToonVS_NMap4Frame.hlsl

pause