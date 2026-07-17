@echo off
rem Separate-compilation demo: build lib.ox and main.ox to objects, link them
rem against the (exact) oxide C runtime, and run the resulting executable.
call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "PATH=%ProgramFiles%\LLVM\bin;%PATH%"
set "ROOT=%~dp0..\.."
set "OX=%ROOT%\build\oxide.exe"
cd /d "%~dp0"

"%OX%" rt -o oxide_rt.c
"%OX%" build lib.ox -o lib.o
"%OX%" build main.ox -o main.o

rem link: the two oxide objects + the single shared runtime -> executable
clang -O2 main.o lib.o oxide_rt.c -o demo.exe
echo LINK_RC=%errorlevel%
.\demo.exe
echo EXIT=%errorlevel%

del lib.o main.o oxide_rt.c demo.exe 2>nul
