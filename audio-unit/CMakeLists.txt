
cmake_minimum_required(VERSION 3.15.0)

project(smtg-vst3-vstsid
    VERSION 1.0.3.1
    DESCRIPTION "igorski.nl VSTSID"
)

if(NOT SMTG_ADD_VSTGUI)
    return()
endif()

set(vstsid_sources
    src/global.h
    src/miditable.h
    src/paramids.h
    src/lfo.h
    src/lfo.cpp
    src/filter.h
    src/filter.cpp
    src/ringmod.h
    src/ringmod.cpp
    src/synth.h
    src/synth.cpp
    src/vst.h
    src/vst.cpp
    src/vstentry.cpp
    src/version.h
    src/ui/controller.h
    src/ui/controller.cpp
    src/ui/uimessagecontroller.h
    resource/plugin.uidesc
)
set(target vstsid)

smtg_add_vst3plugin(${target} ${vstsid_sources})
smtg_target_configure_version_file(${target})

#smtg_add_vst3plugin(${target} PACKAGE_NAME "vstsid" SOURCES_LIST ${vstsid_sources})
set_target_properties(${target}
    PROPERTIES
        ${SDK_IDE_PLUGIN_EXAMPLES_FOLDER}
)

target_compile_features(${target}
    PUBLIC
        cxx_std_17
)

target_link_libraries(${target}
    PRIVATE
        sdk
        vstgui_support
)
smtg_target_add_plugin_resources(${target}
    RESOURCES
        resource/plugin.uidesc
        resource/background.png
        resource/slider_background.png
        resource/slider_handle_2.0x.png
        resource/slider_handle.png
        resource/version.png
)

#smtg_target_add_plugin_snapshots (${target}
#    RESOURCES
#        resource/84E8DE5F92554F5396FAE4133C935A18_snapshot.png
#        resource/84E8DE5F92554F5396FAE4133C935A18_snapshot_2.0x.png
#        resource/41347FD6FED64094AFBB12B7DBA1D441_snapshot.png
#        resource/41347FD6FED64094AFBB12B7DBA1D441_snapshot_2.0x.png
#)

if(SMTG_MAC)
    smtg_target_set_bundle(${target}
        BUNDLE_IDENTIFIER "nl.igorski.vst.${target}"
        COMPANY_NAME "igorski.nl"
    )
elseif(SMTG_WIN)
    target_sources(${target}
        PRIVATE
            resource/plugin.rc
    )
endif(SMTG_MAC)

# Add an AUv2 target
if (SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
	include(SMTG_AddVST3AuV2)
	smtg_target_add_auv2(vstsid-au
		BUNDLE_NAME vstsid
		BUNDLE_IDENTIFIER nl.igorski.vst.${target}.audiounit
		INFO_PLIST_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/audio-unit/Info.plist
		VST3_PLUGIN_TARGET vstsid
    )
endif(SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
