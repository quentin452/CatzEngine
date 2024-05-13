#!/bin/bash
$architectures = @("1) 64 bit", "2) 32 bit", "3) ARM", "4) Web", "5) Nintendo Switch", "6) Android")
$configurations = @("Debug DX11", "Debug GL", "Debug Universal DX11", "Release DX11", "Release GL", "Release Universal DX11")

Write-Host "Please enter architecture :"
for ($i=0; $i -lt $architectures.Length; $i++) {
    Write-Host "$($i+1): $($architectures[$i])"
}
$architectureIndex = Read-Host -Prompt 'Enter the number corresponding to your choice'
$architecture = $architectures[$architectureIndex-1]

Write-Host "Please enter the build configuration :"
for ($i=0; $i -lt $configurations.Length; $i++) {
    Write-Host "$($i+1): $($configurations[$i])"
}
$configurationIndex = Read-Host -Prompt 'Enter the number corresponding to your choice'
$configuration = $configurations[$configurationIndex-1]

msbuild /p:Configuration=$configuration /p:Platform=$architecture Titan.sln
