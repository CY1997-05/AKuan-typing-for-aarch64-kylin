@echo off
rem ============================================================
rem  HZTest Qt 5.5.1 (MSVC 2010 32bit) build script
rem  Run: build.bat
rem ============================================================
setlocal

set VCVARS="D:\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
set QTBIN=D:\Qt\Qt5.5.1\5.5\msvc2010\bin

call %VCVARS% x86
if errorlevel 1 (
    echo [ERROR] vcvarsall.bat failed - check MSVC 2010 installation
    exit /b 1
)

set PATH=%QTBIN%;%PATH%
cd /d "%~dp0"

rem ------------------------------------------------------------
rem link.exe writes the exe into $(DESTDIR)=bin\ but never creates
rem that directory itself; if bin\ is missing the link step fails with
rem   LNK1104: cannot open file bin\HZTest.exe
rem (that is the OUTPUT file, not an input). Also make sure no stale
rem instance is holding the exe open.
rem ------------------------------------------------------------
if not exist bin mkdir bin
if not exist dist mkdir dist
taskkill /F /IM HZTest.exe >nul 2>&1

qmake HZTest.pro
if errorlevel 1 (
    echo [ERROR] qmake failed
    exit /b 1
)

rem ------------------------------------------------------------
rem nmake batch inference rules ("::") put every .cpp into one cl
rem invocation, which can crash cl (0xc0000005) on some machines.
rem Convert them to per-file rules to avoid that.
rem ------------------------------------------------------------
powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-ChildItem -Path . -Filter 'Makefile*' -File | ForEach-Object { $p = $_.FullName; $t = [IO.File]::ReadAllText($p); $t = [Text.RegularExpressions.Regex]::Replace($t, '(?m)^(\{[^\r\n]*?\}\.obj)::', '$1:'); [IO.File]::WriteAllText($p, $t) }"

nmake /NOLOGO
if errorlevel 1 (
    echo [ERROR] nmake failed
    exit /b 1
)

echo.
echo [OK] build finished: bin\HZTest.exe
echo      Make sure Qt DLLs are reachable (add %QTBIN% to PATH or run windeployqt).
endlocal
