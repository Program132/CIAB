@echo off
setlocal enabledelayedexpansion

echo ===================================
echo   CIAB - Compilation (Windows)
echo ===================================

:: 1. Verification de la presence de CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERREUR] CMake n'est pas detecte dans le PATH.
    echo Veuillez installer CMake ou l'ajouter a votre variable d'environnement PATH.
    pause
    exit /b 1
)

:: 2. Configuration avec CMake (detecte automatiquement le compilateur du systeme : MSVC, MinGW, Clang, Ninja, etc.)
echo [1/3] Configuration avec CMake...
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERREUR] Echec de la configuration CMake.
    pause
    exit /b %ERRORLEVEL%
)

:: 3. Compilation du projet
echo.
echo [2/3] Compilation du projet...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERREUR] Echec de la compilation.
    pause
    exit /b %ERRORLEVEL%
)

:: 4. Execution des tests via CTest (universel, cross-platform et multi-config)
echo.
echo [3/3] Execution des tests...
ctest --test-dir build --output-on-failure -C Release --verbose
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERREUR] Un ou plusieurs tests ont echoue.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ===================================
echo [SUCCES] Compilation et tests valides avec succes !
echo ===================================
