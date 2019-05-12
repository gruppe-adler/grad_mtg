New-Item $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\addons -type directory | Out-Null
New-Item $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept -type directory | Out-Null

Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\Debug\${env:APPVEYOR_PROJECT_NAME}.dll $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\Debug\${env:APPVEYOR_PROJECT_NAME}.pdb $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win32\Debug\${env:APPVEYOR_PROJECT_NAME}.ilk $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept

Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\Debug\${env:APPVEYOR_PROJECT_NAME}_x64.dll $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\Debug\${env:APPVEYOR_PROJECT_NAME}_x64.pdb $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept
Copy-Item $env:APPVEYOR_BUILD_FOLDER\build\win64\Debug\${env:APPVEYOR_PROJECT_NAME}_x64.ilk $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\intercept

Copy-Item $env:APPVEYOR_BUILD_FOLDER\addons\$env:APPVEYOR_PROJECT_NAME.pbo $env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME\addons

7z a "$env:APPVEYOR_BUILD_FOLDER\release\${env:APPVEYOR_PROJECT_NAME}_${env:APPVEYOR_REPO_COMMIT}.zip" "$env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME" | Out-Null
Write-Host "Zipped files"
Push-AppveyorArtifact "$env:APPVEYOR_BUILD_FOLDER\release\@$env:APPVEYOR_PROJECT_NAME" 