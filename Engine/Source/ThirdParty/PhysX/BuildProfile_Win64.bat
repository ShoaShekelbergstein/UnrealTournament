@echo off

pushd ..\..\..\Build\BatchFiles
call RunUAT.bat BuildPhysX -TargetPlatforms=Win64+Win32 -TargetConfigs=profile+checked+release -TargetWindowsCompilers=VisualStudio2015 -SkipSubmit
popd