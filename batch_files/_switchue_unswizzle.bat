@echo off
rem Unswizzling for UE games. (They use 8 as the GOBs height.)
@if "%~1"=="" goto skip

set FILE=%1

@pushd %~dp0
swizzler-cli.exe unswizzle %FILE% %FILE:~0,-4%.new.dds switch 8
@popd

pause
:skip
