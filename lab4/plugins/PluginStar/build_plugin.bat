@echo off
set "VCVARS="
set "PATHS_TO_CHECK="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat""

for %%p in (%PATHS_TO_CHECK%) do (
    if exist %%p (
        set "VCVARS=%%~p"
        goto :found
    )
)
:found
if not defined VCVARS (
    echo Run from VS Native Tools or fix VCVARS path.
    exit /b 1
)
call "%VCVARS%" x64 >nul 2>&1
if %ERRORLEVEL% neq 0 call "%VCVARS%" x86_amd64 >nul 2>&1

cd /d "%~dp0"
if not exist "..\..\" (
    echo Unexpected folder layout.
    exit /b 1
)

cl /nologo /EHsc /MD /LD /I..\.. PluginStar.cpp /Fe:..\..\plugins\PluginStar.dll /link user32.lib gdi32.lib
if %ERRORLEVEL% neq 0 exit /b 1
echo OK: ..\..\plugins\PluginStar.dll
exit /b 0
