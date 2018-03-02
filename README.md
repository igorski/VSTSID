# VSTSID

This is a VST3.0 plug-in version of the [WebSID](https://www.igorski.nl/experiment/websid) Commodore 64
synthesizer.

## On compatibility

VST3.0 is great and all, but support is patchy (shoutout to Bitwig Studio for being awesome).
You can however build this plugin as a VST2.4 plugin and enjoy it on a wider range of host platforms.
Simply uncomment the following line in _CMakeLists.txt_:

    set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

Depending on plugin your host software having 32-bit or 64-bit support, you can best compile for a
wider range of architectures, e.g. on OS X:

    cmake "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ..

## Build instructions

The project uses [CMake](https://cmake.org) to generate the build system
after which you can use _make_ to build the application.

The project has been developed against the VST 3.6.8 Audio Plug-Ins SDK
on OS X (see dependencies listed below). It should work completely via
CLI without requiring XCode. As such, Windows and Linux builds might
require an update of the CMakeLists.txt file (see below).

### Environment setup

Apart from having _CMake_ and a _g++_ compiler, the only dependency required is
the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html).
Update _CMakeLists.txt_ to point to the root of the SDK's installation
location (see "VST Sources").

The Steinberg VST sources need to be built in a _/build_-subfolder of
the _/VST3_SDK_-folder, e.g.:

CLI from VST SDK root:

    ./copy_vst2_to_vst3_sdk.sh
    cd VST3_SDK
    mkdir build
    cd build
    cmake ..
    make

The result being that in _/STEINBERG_VST_SDK_ROOT/VST3_SDK/build/lib_ all
Steinberg VST libraries are prebuilt.

### Building the plugin

Run CMake to generate the Makefiles for your environment, e.g.:

    cmake .

After which you can run:

    make

To compile the plugin.
