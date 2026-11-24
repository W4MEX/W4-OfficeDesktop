#Requires -Version 5.1
param(
    [Parameter(Mandatory)][string]$BaseZip,
    [Parameter(Mandatory)][string]$GraphicsDll,
    [Parameter(Mandatory)][string]$OutputZip,
    [string]$SevenZip = '7z.exe'
)

$ErrorActionPreference = 'Stop'
$base = (Resolve-Path -LiteralPath $BaseZip).Path
$graphics = (Resolve-Path -LiteralPath $GraphicsDll).Path
$output = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputZip)
if ([IO.Path]::GetExtension($output) -ne '.zip' -or (Test-Path -LiteralPath $output)) {
    throw 'OutputZip must be a new ZIP file.'
}
$tool = (Get-Command $SevenZip -CommandType Application).Source
$work = Join-Path ([IO.Path]::GetTempPath()) ([guid]::NewGuid().ToString())
$entryName = 'W4 Office USB/App/converter/graphics.dll'
$entryPath = Join-Path $work $entryName
$candidate = Join-Path $work 'candidate.zip'
$graphicsHash = (Get-FileHash -LiteralPath $graphics).Hash
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Get-PackagedGraphicsHash([string]$ArchivePath) {
    $archive = [IO.Compression.ZipFile]::OpenRead($ArchivePath)
    try {
        $entries = @($archive.Entries | Where-Object { $_.FullName -eq $entryName })
        if ($entries.Count -ne 1 -or $entries[0].FullName -cne $entryName) {
            throw "Expected one exact archive member named $entryName."
        }
        $stream = $entries[0].Open()
        try { (Get-FileHash -InputStream $stream).Hash }
        finally { $stream.Dispose() }
    } finally { $archive.Dispose() }
}

$null = Get-PackagedGraphicsHash $base
New-Item -ItemType Directory -Path (Split-Path -Parent $entryPath) -Force | Out-Null
Copy-Item -LiteralPath $base -Destination $candidate
Copy-Item -LiteralPath $graphics -Destination $entryPath
Push-Location $work
try {
    & $tool a -tzip $candidate $entryName | Out-Host
    if ($LASTEXITCODE -ne 0) { throw '7-Zip could not update the package.' }
    & $tool t $candidate | Out-Host
    if ($LASTEXITCODE -ne 0) { throw 'The updated ZIP failed its integrity test.' }
    if ((Get-PackagedGraphicsHash $candidate) -ne $graphicsHash) {
        throw 'The packaged DLL hash differs from the input DLL.'
    }
    [IO.File]::Copy($candidate, $output, $false)
    Get-FileHash -LiteralPath $output
} finally {
    Pop-Location
    $temp = (Resolve-Path -LiteralPath ([IO.Path]::GetTempPath())).Path.TrimEnd('\')
    $resolved = (Resolve-Path -LiteralPath $work).Path
    if ([IO.Path]::GetDirectoryName($resolved) -eq $temp) {
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
}
