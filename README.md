# VSTSID

This is a VST3.0 plug-in version of the [WebSID](https://www.igorski.nl/experiment/websid) Commodore 64
synthesizer.

## On compatibility

VST3.0 is great and all, but support across DAW's is poor (shout out to Bitwig Studio for being awesome).
You can however build this plugin as a VST2.4 plugin and enjoy it on a wider range of host platforms.
Simply uncomment the following line in _CMakeLists.txt_:

    set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

And rename the plugin extension from _.vst3_ to _.vst_.

Depending on your host software having 32-bit or 64-bit support, you can best compile for a
wider range of architectures, e.g. by replacing all _cmake_ invocations listed here like so:

    cmake "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ..

## Build instructions

The project uses [CMake](https://cmake.org) to generate the build system
after which you can use _make_ to build the application.

The project has been developed against the VST 3.6.9 Audio Plug-Ins SDK
on OS X (see dependencies listed below). It should work completely via
CLI without requiring XCode. Windows and Linux builds are provided, but are as
yet untested (see _CMakeLists.txt_).

### Environment setup

Apart from requiring _CMake_ and a _g++_ compiler, the only other dependency is
the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html).
Update _CMakeLists.txt_ to point to the root of the Steinberg SDK installation
location (update "_VSTSDK_PATH_").

Additionally, the Steinberg VST sources need to be built as well. Following
Steinbergs guidelines, the target is a _/build_-subfolder of the _/VST3_SDK_-folder, e.g.:

Execute from the Steinberg VST SDK root:

    ./copy_vst2_to_vst3_sdk.sh
    cd VST3_SDK
    mkdir build
    cd build
    cmake ..
    make

The result being that in _{VSTSDK_PATH}/VST3_SDK/build/lib_ all
Steinberg VST libraries are prebuilt.

### Building the VSTSID plugin

Run CMake to generate VSTSID's Makefile for your environment, e.g.:

    cmake .

After which you can run:

    make

To compile the plugin. The build output will be stored in _./build/VST3/vstsid.vst_
as well as copied to your systems VST3 plugin folder.

## Running the plugin

You can copy the build output into your system VST(3) folder and run it directly in a
VST host / DAW of your choice.

When debugging, you can also choose to run the plugin against Steinbergs validator
and editor host utilities:

    {VSTSDK_PATH}/build/bin/validator build/VST3/vstsid.vst
    {VSTSDK_PATH}/build/bin/editorhost build/VST3/vstsid.vst
