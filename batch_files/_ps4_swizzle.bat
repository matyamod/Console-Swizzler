@echo off
@if "%~1"=="" goto skip

@pushd %~dp0
swizzler-cli.exe swizzle %~1 %~1.new.dds ps4
@popd

pause
:skip
