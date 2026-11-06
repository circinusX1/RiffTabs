@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Deploying Qt Dependencies for RiffTabs.exe
echo ===================================================

:: 1. Define Paths
:: Set current directory as working directory
set "APP_DIR=%~dp0"
set "EXE_NAME=RiffTabs.exe"
set "TARGET_EXE=%APP_DIR%%EXE_NAME%"

:: Provide fallback locations for Qt tools if QT_DIR isn't already set
if "%QT_DIR%"=="" (
    set "QT_DIR=C:\Qt\6.12.0\msvc2022_64"
)

:: 2. Verify File Locations
if not exist "%TARGET_EXE%" (
    echo [ERROR] %EXE_NAME% was not found in the current folder:
    echo %APP_DIR%
    echo.
    echo Please place this batch file in the same directory as %EXE_NAME%.
    pause
    exit /b 1
)

if not exist "%QT_DIR%\bin\windeployqt.exe" (
    echo [ERROR] windeployqt.exe was not found at: %QT_DIR%\bin\windeployqt.exe
    echo Please update QT_DIR in this script to point to your Qt installation path.
    pause
    exit /b 1
)

:: 3. Run windeployqt in current folder
echo.
echo Running windeployqt in current directory...
"%QT_DIR%\bin\windeployqt.exe" --release --compiler-runtime --dir "%APP_DIR:~0,-1%" "%TARGET_EXE%"

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] windeployqt failed with exit code %ERRORLEVEL%.
    pause
    exit /b %ERRORLEVEL%
)

:: 4. Verify Plugins directory
if not exist "%APP_DIR%plugins" (
    echo.
    echo [NOTE] Ensuring plugins folder exists...
    mkdir "%APP_DIR%plugins"
)

echo.
echo ===================================================
echo Deployment successful!
echo All required Qt DLLs and plugins have been deployed to:
echo %APP_DIR%
echo ===================================================
pause
