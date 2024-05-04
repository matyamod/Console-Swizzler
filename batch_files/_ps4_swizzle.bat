@echo off
@if "%~1"=="" goto skip

set FILE=%1

@pushd %~dp0
swizzler-cli.exe swizzle %FILE% %FILE:~0,-4%.new.dds ps4
@popd

pause
:skip
