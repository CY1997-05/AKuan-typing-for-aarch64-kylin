@echo off
rem ============================================================
rem  Build the single-file portable HZTest:
rem    dist\HZTestPortable.exe  (embeds app + Qt runtime)
rem ============================================================
setlocal

set VCVARS="D:\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
set QTBIN=D:\Qt\Qt5.5.1\5.5\msvc2010\bin

call %VCVARS% x86
if errorlevel 1 (
    echo [ERROR] vcvarsall.bat failed
    exit /b 1
)
set PATH=%QTBIN%;%PATH%
cd /d "%~dp0"

if not exist bin\HZTest.exe (
    echo [1/5] building application ...
    call build.bat
    if errorlevel 1 exit /b 1
)

echo [2/5] staging runtime files ...
if exist dist\stage rmdir /s /q dist\stage
mkdir dist\stage
copy /y bin\HZTest.exe dist\stage\ >nul
copy /y bin\*.dll dist\stage\ >nul
if exist bin\platforms (
    mkdir dist\stage\platforms
    copy /y bin\platforms\*.dll dist\stage\platforms\ >nul
)
rem keep the payload small: no svg/image plugins, no software OpenGL fallback
del /q dist\stage\opengl32sw.dll 2>nul
del /q dist\stage\Qt5Svg.dll 2>nul

rem qt.conf: plugins live next to the exe
> dist\stage\qt.conf echo [Paths]
>> dist\stage\qt.conf echo Plugins=.

rem user data files must not be embedded
del /q dist\stage\hztest.ini dist\stage\candidates.dat dist\stage\records.dat 2>nul
del /q dist\stage\practice.dat dist\stage\server.log 2>nul
del /q dist\stage\HZTest.exe.embed.manifest 2>nul

echo [3/5] packing payload ...
pushd portable
cl /nologo /O2 /MT /EHsc /c packer.cpp
if errorlevel 1 (popd & exit /b 1)
link /NOLOGO /SUBSYSTEM:CONSOLE /OUT:packer.exe packer.obj
if errorlevel 1 (popd & exit /b 1)
packer.exe ..\dist\stage payload.bin
if errorlevel 1 (popd & exit /b 1)

echo [4/5] compiling launcher ...
rc /nologo /fo portable.res portable.rc
if errorlevel 1 (popd & exit /b 1)
cl /nologo /O2 /MT /EHsc /DUNICODE /D_UNICODE /c launcher.cpp
if errorlevel 1 (popd & exit /b 1)
link /NOLOGO /SUBSYSTEM:WINDOWS /OUT:..\dist\HZTestPortable.exe launcher.obj portable.res
if errorlevel 1 (popd & exit /b 1)
popd

echo [5/5] done.
dir /b dist\HZTestPortable.exe
echo.
echo Portable build: dist\HZTestPortable.exe
endlocal
