@echo off
rmdir /Q /S build
mkdir build
cd build

IF /I "%1"=="vst2" (
    set SMTG_CREATE_VST2_VERSION="-DSMTG_CREATE_VST2_VERSION=ON"
) ELSE (
    set SMTG_CREATE_VST2_VERSION=""
)

cmake.exe -G"Visual Studio 16 2019" %SMTG_CREATE_VST2_VERSION% -DVST3_SDK_ROOT=%VST3_SDK_ROOT% ..
cmake --build . --config Release

IF /I "%1"=="vst2" (
    rename VST3\Release\vstsid.vst3\Contents\x86_64-win\vstsid.vst3 vstsid.dll
)

cd ..
