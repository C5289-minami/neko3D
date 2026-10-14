param([switch]$SkipBuild)
$ErrorActionPreference = 'Stop'
$taskRepo = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $taskRepo
$taskBuildPath = $env:Path
Remove-Item Env:PATH -ErrorAction SilentlyContinue
Remove-Item Env:Path -ErrorAction SilentlyContinue
$env:Path = $taskBuildPath

$taskVswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$taskVs = & $taskVswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$taskVs) { throw 'Visual Studio C++ tools were not found.' }
if (!$SkipBuild) {
    & (Join-Path $taskVs 'MSBuild\Current\Bin\MSBuild.exe') 3dgp1.sln /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo /verbosity:minimal
    if ($LASTEXITCODE -ne 0) { throw 'Debug build failed.' }
}

$taskOutput = Join-Path $taskRepo 'obj\PlayerDamageTests'
[IO.Directory]::CreateDirectory($taskOutput) | Out-Null
$taskDxlib = if (Test-Path -LiteralPath 'C:\DxLib_VC\Dxlib\DxLib.h') { 'C:\DxLib_VC\Dxlib' } else { Join-Path $taskRepo 'DxLib' }
$taskEffekseer = Join-Path $taskRepo 'Effekseer\プロジェクトに追加すべきファイル_VC用'
$taskCompileArgs = @('/nologo', '/std:c++17', '/EHsc', '/MTd', '/D_DEBUG', '/DUNICODE', '/D_UNICODE', '/c')
foreach ($taskInclude in @($taskRepo, $taskDxlib, $taskEffekseer, (Join-Path $taskRepo 'imgui'), (Join-Path $taskRepo 'SSPlayer'))) {
    $taskCompileArgs += '/I"' + $taskInclude + '"'
}
$taskCompileArgs += @(('/Fo"' + (Join-Path $taskOutput 'PlayerDamageTests.obj') + '"'), ('"' + (Join-Path $PSScriptRoot 'PlayerDamageTests.cpp') + '"'))
[IO.File]::WriteAllLines((Join-Path $taskOutput 'compile.rsp'), $taskCompileArgs, [Text.Encoding]::Unicode)
[xml]$taskProject = Get-Content -LiteralPath (Join-Path $taskRepo '3dgp1.vcxproj')
$taskObjects = $taskProject.Project.ItemGroup.ClCompile | Where-Object { $_.Include -and $_.Include -ne 'main.cpp' } | ForEach-Object {
    '"' + (Join-Path $taskRepo ('obj\Debug\' + [IO.Path]::GetFileNameWithoutExtension($_.Include) + '.obj')) + '"'
}
$taskLinkArgs = @('/nologo', '/subsystem:console', '/machine:x64',
    ('/out:"' + (Join-Path $taskOutput 'PlayerDamageTests.exe') + '"'),
    ('/libpath:"' + $taskDxlib + '"'), ('/libpath:"' + $taskEffekseer + '"'),
    ('"' + (Join-Path $taskOutput 'PlayerDamageTests.obj') + '"')) + $taskObjects
[IO.File]::WriteAllLines((Join-Path $taskOutput 'link.rsp'), $taskLinkArgs, [Text.Encoding]::Unicode)
$taskBatch = @"
@echo off
call "$taskVs\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl.exe @"$taskOutput\compile.rsp"
if errorlevel 1 exit /b 1
link.exe @"$taskOutput\link.rsp"
if errorlevel 1 exit /b 1
"$taskOutput\PlayerDamageTests.exe"
exit /b %errorlevel%
"@
[IO.File]::WriteAllText((Join-Path $taskOutput 'run.cmd'), $taskBatch)
& cmd.exe /d /c (Join-Path $taskOutput 'run.cmd')
if ($LASTEXITCODE -ne 0) { throw 'Player damage tests failed.' }
