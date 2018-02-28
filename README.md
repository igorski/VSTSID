# VSTSID

This is VST3.0 plug-in version of the [WebSID](https://www.igorski.nl/experiment/websid) Commodore 64
synthesizer.

## Build instructions

### Environment setup

The project is built using [CMake](https://cmake.org). The only dependency you need is to get
the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html). This project has been developed against the VST 3.6.8 Audio Plug-Ins SDK.

### Building the plugin

Update _CMakeLists.txt_ to point to your VST SDK installation folder. Run CMake to generate the
Makefiles for your environment, e.g.:

    cmake .

After which you can run:

    make

To compile the plugin.
