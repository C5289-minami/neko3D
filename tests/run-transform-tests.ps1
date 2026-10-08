$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $repo

# ビルド前にPathの重複を整理する
$taskBuildPath = $env:Path
Remove-Item Env:PATH -ErrorAction SilentlyContinue
Remove-Item Env:Path -ErrorAction SilentlyContinue
$env:Path = $taskBuildPath

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual Studio C++ tools were not found.' }
& (Join-Path $vs 'MSBuild\Current\Bin\MSBuild.exe') 3dgp1.sln /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo /verbosity:minimal
if ($LASTEXITCODE -ne 0) { throw 'Debug build failed.' }

$output = Join-Path $repo 'obj\TransformSettingsTests'
[IO.Directory]::CreateDirectory($output) | Out-Null
$dxlib = if (Test-Path -LiteralPath 'C:\DxLib_VC\Dxlib\DxLib.h') { 'C:\DxLib_VC\Dxlib' } else { Join-Path $repo 'DxLib' }
$compileArgs = @(
    '/nologo', '/std:c++17', '/EHsc', '/MTd', '/D_DEBUG', '/DUNICODE', '/D_UNICODE', '/c',
    ('/I"' + $repo + '"'), ('/I"' + $dxlib + '"'),
    ('/I"' + (Join-Path $repo 'imgui') + '"'), ('/I"' + (Join-Path $repo 'SSPlayer') + '"'),
    ('/Fo"' + (Join-Path $output 'TransformSettingsTests.obj') + '"'),
    ('"' + (Join-Path $PSScriptRoot 'TransformSettingsTests.cpp') + '"')
)
[IO.File]::WriteAllLines((Join-Path $output 'compile.rsp'), $compileArgs)
[xml]$project = Get-Content -LiteralPath (Join-Path $repo '3dgp1.vcxproj')
$objects = $project.Project.ItemGroup.ClCompile | Where-Object { $_.Include -and $_.Include -ne 'main.cpp' } | ForEach-Object {
    '"' + (Join-Path $repo ('obj\Debug\' + [IO.Path]::GetFileNameWithoutExtension($_.Include) + '.obj')) + '"'
}
$linkArgs = @('/nologo', '/subsystem:console', '/machine:x64',
    ('/out:"' + (Join-Path $output 'TransformSettingsTests.exe') + '"'),
    ('/libpath:"' + $dxlib + '"'),
    ('"' + (Join-Path $output 'TransformSettingsTests.obj') + '"')) + $objects
[IO.File]::WriteAllLines((Join-Path $output 'link.rsp'), $linkArgs)
$batch = @"
@echo off
call "$vs\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl.exe @"$output\compile.rsp"
if errorlevel 1 exit /b 1
link.exe @"$output\link.rsp"
if errorlevel 1 exit /b 1
"$output\TransformSettingsTests.exe" "$output\data"
if errorlevel 1 exit /b 1
cl.exe /nologo /std:c++17 /EHsc /c /I"$repo" /FI"$output\data\SavedTransformDefaults.h" /Fo"$output\CompileProbe.obj" "$PSScriptRoot\TransformDefaultsCompileProbe.cpp"
exit /b %errorlevel%
"@
[IO.File]::WriteAllText((Join-Path $output 'run.cmd'), $batch)
& cmd.exe /d /c (Join-Path $output 'run.cmd')
if ($LASTEXITCODE -ne 0) { throw 'Transform tests failed.' }
