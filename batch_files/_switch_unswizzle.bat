@echo off
@if "%~1"=="" goto skip

set FILE=%1

@pushd %~dp0
swizzler-cli.exe unswizzle %FILE% %FILE:~0,-4%.new.dds switch
@popd

pause
:skip
