$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$msbuild = Join-Path $installation 'MSBuild/Current/Bin/MSBuild.exe'
$toolset = Get-ChildItem (Join-Path $installation 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$compilerBin = Join-Path $toolset.FullName 'bin/Hostx64/x64'
$savedPath = $env:PATH
$savedCompilerOptions = $env:_CL_
Push-Location $repoRoot
try {
    # Use the matching DLLs and embed compiler symbols to avoid PDB-server version conflicts.
    $env:PATH = "$compilerBin;$savedPath"
    $env:_CL_ = "$savedCompilerOptions /Z7"
    $buildOptions = @('/p:Configuration=Debug', '/p:Platform=x64', '/p:PreferredToolArchitecture=x64', '/nologo', '/v:quiet')
    # Keep the playable executable available even while the game is running.
    $gameOutput = Join-Path $repoRoot 'obj/CollisionBuild/'
    & $msbuild 3dgp1.vcxproj @buildOptions "/p:OutDir=$gameOutput"
    if ($LASTEXITCODE -ne 0) { throw 'Game build failed.' }
    & $msbuild tests/ModelCollisionTests.vcxproj @buildOptions
    if ($LASTEXITCODE -ne 0) { throw 'Collision test build failed.' }
    & './obj/CollisionTests/ModelCollisionTests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Collision tests failed.' }
}
finally {
    $env:PATH = $savedPath
    $env:_CL_ = $savedCompilerOptions
    Pop-Location
}
