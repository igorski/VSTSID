###########################################
# CMake Project Template for VST Plug-ins #
###########################################

project(VSTSID)
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

# path to Steinberg VST3 SDK
set(VSTSDK_PATH "/Library/VST_SDK/VST3_SDK" CACHE PATH "Path to VST3 SDK")

# uncomment to build as VST2.4
#set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

# compiler flags
add_definitions(-DNDEBUG)

if(MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

###############
# Unix builds #
###############

if(UNIX)
    if(XCODE)
       set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
       set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    elseif(APPLE)
       set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
       link_libraries(c++)
    else()
        add_definitions( -D__cdecl= )
        set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar")
        link_libraries(stdc++fs pthread dl)
    endif()
endif()


#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${VSTSDK_PATH}/cmake/modules")

include(Global)
include(AddVST3Library)
include(Bundle)
include(ExportedSymbols)
include(PrefixHeader)
include(PlatformIOS)

###############
# VST sources #
###############

set(VSTSDK_INCLUDE_DIR ${VSTSDK_PATH})
set(VSTSDK_PLUGIN_SOURCE
  ${VSTSDK_PATH}/public.sdk/source/vst/vstaudioeffect.cpp
  ${VSTSDK_PATH}/public.sdk/source/vst/vstaudioprocessoralgo.h
  ${VSTSDK_PATH}/public.sdk/source/vst/vsteditcontroller.h
  ${VSTSDK_PATH}/pluginterfaces/base/ibstream.h
  ${VSTSDK_PATH}/pluginterfaces/base/ustring.h
  ${VSTSDK_PATH}/pluginterfaces/vst/ivstevents.h
  ${VSTSDK_PATH}/pluginterfaces/vst/ivstparameterchanges.h
  ${VSTSDK_PATH}/pluginterfaces/vst/vstpresetkeys.h
)

if (SMTG_CREATE_VST2_VERSION)
    set(vstsid_vst2_sources
        ${SDK_ROOT}/public.sdk/source/common/memorystream.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
        ${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
        ${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.h
        ${SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp
        ${SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp
        src/vstentry_vst2.cpp
    )
endif()

###################
# Project sources #
###################

set(vstsid_sources
    src/global.h
    src/miditable.h
    src/paramids.h
    src/synthesizer.h
    src/synthesizer.cpp
    src/vst.h
    src/vst.cpp
    src/vstentry.cpp
    src/ui/controller.h
    src/ui/controller.cpp
    src/ui/uimessagecontroller.h
    #resources/vstsid.rc
    #resources/vstplug.def
    ${VSTSDK_PLUGIN_SOURCE}
)

# VST2 is only defined for macOS and Windows
if(MAC OR WIN)
    set(vstsid_sources ${vstsid_sources} ${vstsid_vst2_sources})
endif()

set(target vstsid)
smtg_add_vst3plugin(${target} ${VSTSDK_PATH} ${vstsid_sources})

###########################
# STEINBERG VST LIBRARIES #
###########################

include_directories(${VSTSDK_INCLUDE_DIR})
target_link_libraries(${target} ${VSTSDK_LIBRARIES})
target_include_directories(${target} PUBLIC ${VSTSDK_PATH}/vstgui4)
target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libbase.a)
target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libsdk.a)
target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui_support.a)
target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui_uidescription.a)
target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui.a)

if (SMTG_CREATE_VST2_VERSION)
    message(STATUS "SMTG_CREATE_VST2_VERSION is set. A VST 2 version of the plug-in will be created (just rename the generated file from .vst3 to .vst).")
    if(XCODE)
        # fix missing VSTPluginMain symbol when also building VST 2 version
        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "")
    endif()
    if (WIN)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()
endif()

###############################
# PLATFORM SPECIFIC LIBRARIES #
###############################

IF (APPLE)
    if(XCODE)
        target_link_libraries(${target} PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework Accelerate" "-framework QuartzCore" "-framework Carbon")
    else()
        find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
        find_library(COCOA_FRAMEWORK Cocoa)
        find_library(OPENGL_FRAMEWORK OpenGL)
        find_library(ACCELERATE_FRAMEWORK Accelerate)
        find_library(QUARTZCORE_FRAMEWORK QuartzCore)
        find_library(CARBON_FRAMEWORK Carbon)
        find_library(EXPAT Expat)
        target_link_libraries(${target} PRIVATE ${COREFOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${ACCELERATE_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} ${CARBON_FRAMEWORK} ${EXPAT})
    endif()
ENDIF (APPLE)

#############
# Resources #
#############

smtg_add_vst3_resource(${target} "resources/graphics/vstsid.xml")
smtg_add_vst3_resource(${target} "resources/graphics/background.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_background.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_handle.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_handle_2.0x.png")
smtg_add_vst3_resource(${target} "resources/graphics/vu_on.png")
smtg_add_vst3_resource(${target} "resources/graphics/vu_off.png")

#########################
# Set bundle properties #
#########################

set_target_properties(vstsid PROPERTIES
  BUNDLE true
  BUNDLE_EXTENSION "vst3"
  XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/osx/Info.plist"
  MACOSX_BUNDLE_BUNDLE_NAME "VSTSID"
  MACOSX_BUNDLE_GUI_IDENTIFIER "nl.igorski.VSTSID"
  MACOSX_BUNDLE_ICON_FILE ""
  MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
  MACOSX_BUNDLE_COPYRIGHT "igorski.nl © 2018"
)

if(MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/osx/Info.plist" PREPROCESS)
elseif(WIN)
    target_sources(${target} PRIVATE resource/vstsid.rc)
endif()

####################
# Set Install Path #
####################

if(APPLE)
  install(TARGETS vstsid
    DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/VST"
  )
elseif(WIN32)
  install(TARGETS vstsid
    DESTINATION "/Program Files (x86)/Common Files/VST3/"
  )
elseif(UNIX AND NOT APPLE) #Linux
  install(TARGETS vstsid
    DESTINATION "/usr/lib/lxvst"
  )
endif()
