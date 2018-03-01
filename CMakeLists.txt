###########################################
# CMake Project Template for VST Plug-ins #
###########################################

project(AGain)
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

# compiler flags
add_definitions(-DNDEBUG)

if(MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

############################
# API fix for Linux builds #
############################

if(UNIX AND NOT APPLE)
  add_definitions( -D__cdecl= )
endif()

########################
# OS X Specific builds #
########################

IF (APPLE)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation )
   find_library(COREGRAPHICS_LIBRARY   CoreGraphics )
   find_library(IMAGE_IO_LIBRARY       ImageIO )
   find_library(OPENGL_LIBRARY         OpenGL )
   find_library(CORE_TEXT_LIBRARY      CoreText )
   find_library(APPLICATION_SERVICES   ApplicationServices )
   find_library(CORE_APP_LIBRARY       AppKit )
   find_library(QUARTZ_CORE_LIBRARY    QuartzCore )
   find_library(EXPAT_LIBRARY          Expat )
   find_library(ACCELERATE_FRAMEWORK   Accelerate )
ENDIF (APPLE)

###############
# VST sources #
###############

set(VSTSDK_PATH "/Library/VST_SDK/VST3_SDK" CACHE PATH "Path to VST3 SDK")
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
    set(again_vst2_sources
        ${SDK_ROOT}/public.sdk/source/common/memorystream.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
        ${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
        ${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.h
        ${SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp
        ${SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp
        source/main_vst2.cpp
    )
endif()

###################
# Project sources #
###################

set(again_sources
    src/vst.cpp
    src/vst.h
    src/ui/controller.cpp
    src/ui/controller.h
    src/main_vst3.cpp
    src/ui/uimessagecontroller.h
    #resources/again.rc
    #resources/vstplug.def
    ${VSTSDK_PLUGIN_SOURCE}
)

# VST2 is only defined for macOS and Windows
if(MAC OR WIN)
    set(again_sources ${again_sources} ${again_vst2_sources})
endif()

set(target again)
add_library(${target} MODULE ${again_sources})

###############################
# PLATFORM SPECIFIC LIBRARIES #
###############################

IF (APPLE)
   target_link_libraries(${target} ${COREFOUNDATION_LIBRARY})
   target_link_libraries(${target} ${COREGRAPHICS_LIBRARY})
   target_link_libraries(${target} ${IMAGE_IO_LIBRARY})
   target_link_libraries(${target} ${OPENGL_LIBRARY})
   target_link_libraries(${target} ${CORE_TEXT_LIBRARY})
   target_link_libraries(${target} ${APPLICATION_SERVICES})
   target_link_libraries(${target} ${CORE_APP_LIBRARY})
   target_link_libraries(${target} ${QUARTZ_CORE_LIBRARY})
   target_link_libraries(${target} ${EXPAT_LIBRARY})
   target_link_libraries(${target} ${ACCELERATE_FRAMEWORK})
ENDIF (APPLE)

###########################
# STEINBERG VST LIBRARIES #
###########################

include_directories(${VSTSDK_INCLUDE_DIR})
target_link_libraries(${target} ${VSTSDK_LIBRARIES})
target_include_directories(${target} PUBLIC ${VSTSDK_PATH}/vstgui4)
target_link_libraries(${target} /Library/VST_SDK/VST3_SDK/build/lib/libbase.a)
target_link_libraries(${target} /Library/VST_SDK/VST3_SDK/build/lib/libsdk.a)
target_link_libraries(${target} /Library/VST_SDK/VST3_SDK/build/lib/libvstgui_support.a)
target_link_libraries(${target} /Library/VST_SDK/VST3_SDK/build/lib/libvstgui_uidescription.a)
target_link_libraries(${target} /Library/VST_SDK/VST3_SDK/build/lib/libvstgui.a)

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


######################################
# Set OSX-specific bundle properties #
######################################

set_target_properties(again PROPERTIES 
  BUNDLE true
  BUNDLE_EXTENSION "vst"
  XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst"
  MACOSX_BUNDLE_INFO_PLIST "resources/osx/Info.plist"
  MACOSX_BUNDLE_BUNDLE_NAME "VSTSID"
  MACOSX_BUNDLE_GUI_IDENTIFIER "nl.igorski.VSTSID"
  MACOSX_BUNDLE_ICON_FILE ""
  MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
  MACOSX_BUNDLE_COPYRIGHT "igorski.nl © 2018"
)

####################
# Set Install Path #
####################

if(APPLE)
  install(TARGETS again
    DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/VST"
  )
elseif(WIN32)
  install(TARGETS again
    DESTINATION "/Program Files (x86)/Common Files/VST3/"
  )
elseif(UNIX AND NOT APPLE) #Linux
  install(TARGETS again
    DESTINATION "/usr/lib/lxvst"
  )
endif()
