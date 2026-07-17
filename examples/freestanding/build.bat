@echo off
rem Freestanding build demo: produce a runtime-less object exposing kmain/fib
rem unmangled, with zero ox_* symbols. Inspect with llvm-nm (or dumpbin).
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "PATH=C:\Program Files\LLVM\bin;%PATH%"
set "ROOT=%~dp0..\.."
set "OX=%ROOT%\build\oxide.exe"
cd /d "%~dp0"

"%OX%" build --freestanding --entry kmain kmain.ox -o k.o
echo BUILD_RC=%errorlevel%

echo === runtime ox_* symbols (expect none) ===
llvm-nm k.o | findstr /R "ox_ oxide_main" || echo   none
echo === exported functions (expect kmain, fib, entry_check) ===
llvm-nm k.o | findstr " T "

del k.o 2>nul
