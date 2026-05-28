@echo off
echo Building lab2 with MSVC...

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
    echo Run from "x64 Native Tools Command Prompt for VS" or fix path in build.bat
    pause
    exit /b 1
)

call "%VCVARS%" x64 >nul 2>&1
if %ERRORLEVEL% neq 0 call "%VCVARS%" x86_amd64 >nul 2>&1

cd /d "%~dp0"
rc /nologo /folab1.res lab1.rc
if %ERRORLEVEL% neq 0 exit /b 1

cl /nologo /EHsc /Fe:ShapesLab.exe main.cpp ShapeList.cpp ShapeRenderer.cpp ShapeFactory.cpp Solid3D.cpp ^
   LineShape.cpp RectangleShape.cpp EllipseShape.cpp CircleShape.cpp TriangleShape.cpp PointShape.cpp ^
   Cube3DShape.cpp Sphere3DShape.cpp Tetra3DShape.cpp PolygonShape.cpp ^
   lab1.res user32.lib gdi32.lib comdlg32.lib

if %ERRORLEVEL% equ 0 (
    echo OK: ShapesLab.exe
) else (
    echo Build failed.
)
pause