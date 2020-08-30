#define PLATFORM_64 true

#include "version.h"

// Check https://developer.apple.com/library/archive/documentation/General/Conceptual/ExtensibilityPG/AudioUnit.html for various types

/* Bundle Identifier */
#define kAudioUnitBundleIdentifier	nl.igorski.vst.transformant.audiounit

/* Version Number (needs to be in hex) */
#define kAudioUnitVersion			0x00010000

/* Company Name + Effect Name */
#define kAUPluginName 				igorski: Transformant

/* A product name for the audio unit, such as TremoloUnit */
#define kAUPluginDescription 		Transformant

/*
  The specific variant of the Audio Unit. The four possible types and their values are:
  Effect (aufx), Generator (augn), Instrument (aumu), and Music Effect (aufm).
 */
#define kAUPluginType 				aumu

/* A subtype code for the audio unit, such as tmlo. This value must be exactly 4 alphanumeric characters. */
#define kAUPluginSubType 			synt

/* A manufacturer code for the audio unit, such as Aaud. This value must be exactly 4 alphanumeric characters.
 * Manufacturer OSType should have at least one non-lower case character */
#define kAUPluginManufacturer 		IGOR

// Definitions for the resource file
#define kAudioUnitName				"igorski: Transformant" // same as kAUPluginName
#define kAudioUnitDescription	    "Transformant" // same as kAUPluginDescription
#define kAudioUnitType				'aumu' // same as kAUPluginType
#define kAudioUnitComponentSubType	'synt' // same as kAUPluginSubType
#define kAudioUnitComponentManuf    'IGOR' // same as kAUPluginManufacturer

#define kAudioUnitCarbonView		1		// if 0 no Carbon view support will be added
