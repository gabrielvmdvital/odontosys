$ErrorActionPreference = 'Stop'
$projectDir = "C:\Users\Victor\Documents\UPE\porgramacao imperativa\projeto final\odontosys"
$distBaseDir = "$projectDir\dist"
$distDir = "$distBaseDir\OdontoSys"

Write-Host "Creating dist directory..."
if (Test-Path $distBaseDir) { Remove-Item -Recurse -Force $distBaseDir }
New-Item -ItemType Directory -Path $distDir | Out-Null

Write-Host "Copying executable..."
Copy-Item "$projectDir\bin\programa.exe" -Destination "$distDir\odontosys.exe"

Write-Host "Finding DLLs..."
$env:PATH="C:\msys64\ucrt64\bin;" + $env:PATH
$lddOutput = C:\msys64\usr\bin\ldd.exe "$projectDir\bin\programa.exe"

$dlls = @{}
foreach ($line in $lddOutput) {
    if ($line -match "=> (/[^ ]+) ") {
        $path = $matches[1]
        if ($path -match "^/ucrt64/(.*)") {
            $realPath = "C:\msys64\ucrt64\" + $matches[1]
            $dlls[$realPath] = $true
        }
    }
}

Write-Host "Copying $($dlls.Count) DLLs..."
foreach ($dll in $dlls.Keys) {
    if (Test-Path $dll) {
        Copy-Item $dll -Destination $distDir
    }
}

Write-Host "Copying schemas..."
$schemasDir = "C:\msys64\ucrt64\share\glib-2.0\schemas"
if (Test-Path $schemasDir) {
    New-Item -ItemType Directory -Path "$distDir\share\glib-2.0" -Force | Out-Null
    Copy-Item $schemasDir -Destination "$distDir\share\glib-2.0\schemas" -Recurse
}

Write-Host "Copying icons..."
$iconsDir = "C:\msys64\ucrt64\share\icons\hicolor"
if (Test-Path $iconsDir) {
    New-Item -ItemType Directory -Path "$distDir\share\icons" -Force | Out-Null
    Copy-Item $iconsDir -Destination "$distDir\share\icons\hicolor" -Recurse
}

Write-Host "Done!"
