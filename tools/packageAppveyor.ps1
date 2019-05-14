New-Item $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\addons -type directory | Out-Null
New-Item $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept -type directory | Out-Null

Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}.dll $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}.pdb $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
if(${env:CONFIGURATION} -eq 'Debug') {
    Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}.ilk $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
}

Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}_x64.dll $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}_x64.pdb $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
if(${env:CONFIGURATION} -eq 'Debug') {
    Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\${env:CONFIGURATION}\${env:APPVEYOR_PROJECT_NAME}_x64.ilk $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
}

Copy-Item $env:APPVEYOR_BUILD_FOLDER\addons\$env:APPVEYOR_PROJECT_NAME.pbo $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\addons

$tagToAdd = ""
if(![string]::IsNullOrEmpty($env:APPVEYOR_REPO_TAG_NAME)) {
    Write-Host "Using Tag: $env:APPVEYOR_REPO_TAG_NAME"
    $tagToAdd = "$env:APPVEYOR_REPO_TAG_NAME"
} else {
    $tagToAdd = "${env:APPVEYOR_REPO_COMMIT}"
    Write-Host "Tag not found, using commit hash"
}

7z a "$env:APPVEYOR_BUILD_FOLDER\release\${env:APPVEYOR_PROJECT_NAME}_$tagToAdd.zip" "$env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME" | Out-Null
Write-Host "Zipped mod folder"
Push-AppveyorArtifact "$env:APPVEYOR_BUILD_FOLDER\release\${env:APPVEYOR_PROJECT_NAME}_$tagToAdd.zip" -DeploymentName zip