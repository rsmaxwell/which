@echo off
setlocal

SET scriptpath=%~dp0

set ROOT=%scriptpath:~0,-1%
set SOURCE=%ROOT%\src\main\c
set OUTPUT=%ROOT%\target\output
set DIST=%ROOT%\target\dist
set BUILD_TYPE=static
set buildLabel=${SNAPSHOT}

if NOT exist %OUTPUT% mkdir %OUTPUT%
if NOT exist %DIST%   mkdir %DIST%

cd %OUTPUT%

echo on
make -f %ROOT%\src\main\make\x86_64-Windows-vs2017.makefile %*


