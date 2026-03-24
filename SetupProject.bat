@echo off
setlocal EnableDelayedExpansion
echo ===================================================
echo Automated C++ and Raylib Environment Setup Tool
echo ===================================================

echo.
echo [1/6] Verifying Windows Package Manager (winget) availability...
where winget >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [Status] winget is missing. Bootstrapping via Microsoft.WinGet.Client module...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Write-Host 'Configuring Environment...'; Install-PackageProvider -Name NuGet -Force; Set-PSRepository -Name PSGallery -InstallationPolicy Trusted; Write-Host 'Installing Client Module...'; Install-Module -Name Microsoft.WinGet.Client -Force -Repository PSGallery; Write-Host 'Executing Installation...'; Repair-WinGetPackageManager"
    set "PATH=%LocalAppData%\Microsoft\WindowsApps;%PATH%"
) else (
    echo [Status] winget is already installed. Proceeding...
)

echo.
echo [2/6] Optimizing winget extraction parameters for virtualized environments...
:: Establish the target directory for winget settings
set "WINGET_SETTINGS_DIR=%LOCALAPPDATA%\Packages\Microsoft.DesktopAppInstaller_8wekyb3d8bbwe\LocalState"
if not exist "%WINGET_SETTINGS_DIR%" mkdir "%WINGET_SETTINGS_DIR%"
set "WINGET_SETTINGS_FILE=%WINGET_SETTINGS_DIR%\settings.json"

:: Programmatically construct the JSON configuration
echo { > "%WINGET_SETTINGS_FILE%"
echo     "$schema": "https://aka.ms/winget-settings.schema.json", >> "%WINGET_SETTINGS_FILE%"
echo     "installBehavior": { >> "%WINGET_SETTINGS_FILE%"
echo         "archiveExtractionMethod": "tar" >> "%WINGET_SETTINGS_FILE%"
echo     } >> "%WINGET_SETTINGS_FILE%"
echo } >> "%WINGET_SETTINGS_FILE%"
echo [Status] Configuration applied successfully.

echo.
echo [3/6] Installing dependencies and Visual Studio Code...
winget install WinLibs cmake Git.Git ezwinports.make Microsoft.VisualStudioCode --accept-package-agreements --accept-source-agreements

echo.
echo [4/6] Refreshing environment variables for the current session...
:: Interrogate the Windows Registry for updated PATH variables
for /f "tokens=2*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul') do set "SYS_PATH=%%B"
for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul') do set "USR_PATH=%%B"
:: Reconstruct the active session PATH
set "PATH=%SYS_PATH%;%USR_PATH%;%PATH%"

:: Fallback explicit path injection for Git to ensure execution redundancy
if not exist "git.exe" set "PATH=%ProgramFiles%\Git\cmd;%PATH%"

echo.
echo [5/6] Cloning the repository...
set REPO_URL="https://github.com/lhldev/late-to-class"
git clone %REPO_URL%

echo.
echo [6/6] Installing Visual Studio Code Extensions...
set "VSCODE_BIN=%LocalAppData%\Programs\Microsoft VS Code\bin\code.cmd"
if not exist "%VSCODE_BIN%" set "VSCODE_BIN=%ProgramFiles%\Microsoft VS Code\bin\code.cmd"
if not exist "%VSCODE_BIN%" set "VSCODE_BIN=%ProgramFiles(x86)%\Microsoft VS Code\bin\code.cmd"

if exist "%VSCODE_BIN%" (
    call "%VSCODE_BIN%" --install-extension ms-vscode.cpptools-extension-pack --force
) else (
    echo WARNING: Could not locate the VS Code executable.
)

echo.
echo Setup complete! You may now open the cloned folder in Visual Studio Code.
pause
