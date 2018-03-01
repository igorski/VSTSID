# VSTSID

This is a VST3.0 plug-in version of the [WebSID](https://www.igorski.nl/experiment/websid) Commodore 64
synthesizer.

## Build instructions

The project uses [CMake](https://cmake.org) to generate the build system
after which you can use _make_ to build the application.

The project has been developed against the VST 3.6.8 Audio Plug-Ins SDK
on OS X. It should work completely via CLI without requiring XCode. As
such, Windows and Linux builds might require an update of the CMakeLists.txt
file (see below).

### Environment setup

Apart from having CMake and a _g++_ compiler, the only dependency required is
the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html).
Update _CMakeLists.txt_ to point to the root of the SDK's installation
location (see "VST Sources").

The Steinberg VST3 sources need to be built in a _/build_-subfolder of
the _/VST3_SDK_-folder, e.g.:

CLI from VST SDK root:

```
cd VST3_SDK
mkdir build
cd build
cmake ..
make
```

The result being that in _/STEINBERG_VST_SDK_ROOT/VST3_SDK/build/lib_ all
Steinberg VST libraries are prebuilt.

### Building the plugin

Update _CMakeLists.txt_ to point to your VST SDK installation folder. Run CMake to generate the
Makefiles for your environment, e.g.:

    cmake .

After which you can run:

    make

To compile the plugin.
