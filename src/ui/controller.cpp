/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2023 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "../global.h"
#include "controller.h"
#include "uimessagecontroller.h"
#include "../paramids.h"
#include "../filter.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "base/source/fstring.h"
#include "base/source/fstreamer.h"

#include "vstgui/uidescription/delegationcontroller.h"

#include <stdio.h>
#include <math.h>

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// VSTSIDController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::initialize( FUnknown* context )
{
    tresult result = EditControllerEx1::initialize( context );

    if ( result != kResultOk )
        return result;

    //--- Create Units-------------
    UnitInfo unitInfo;
    Unit* unit;

    // create root only if you want to use the programListId
    /*	unitInfo.id = kRootUnitId;	// always for Root Unit
    unitInfo.parentUnitId = kNoParentUnitId;	// always for Root Unit
    Steinberg::UString (unitInfo.name, USTRINGSIZE (unitInfo.name)).assign (USTRING ("Root"));
    unitInfo.programListId = kNoProgramListId;

    unit = new Unit (unitInfo);
    addUnitInfo (unit);*/

    // create a unit1
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId;    // attached to the root unit

    Steinberg::UString( unitInfo.name, USTRINGSIZE( unitInfo.name )).assign( USTRING( "ADSR" ));

    unitInfo.programListId = kNoProgramListId;

    unit = new Unit( unitInfo );
    addUnit( unit );
    int32 unitId = 1;

    // ADSR controls

    parameters.addParameter( new RangeParameter(
        USTRING( "Attack time" ), kAttackId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "Decay time" ), kDecayId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "Sustain volume" ), kSustainId, USTRING( "0 - 1" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "Release time" ), kReleaseId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    // filter controls

    parameters.addParameter( new RangeParameter(
        USTRING( "Cutoff frequency" ), kCutoffId, USTRING( "Hz" ),
        Igorski::VST::FILTER_MIN_FREQ, Igorski::VST::FILTER_MAX_FREQ, Igorski::VST::FILTER_MIN_FREQ,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "Resonance" ), kResonanceId, USTRING( "db" ),
        Igorski::VST::FILTER_MIN_RESONANCE, Igorski::VST::FILTER_MAX_RESONANCE, Igorski::VST::FILTER_MIN_RESONANCE,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "LFO rate" ), kLFORateId, USTRING( "Hz" ),
        Igorski::VST::MIN_LFO_RATE(), Igorski::VST::MAX_LFO_RATE(), Igorski::VST::MIN_LFO_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    ));

    parameters.addParameter( new RangeParameter(
        USTRING( "LFO depth" ), kLFODepthId, USTRING( "%" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    // ring modulator

    parameters.addParameter( new RangeParameter(
        USTRING( "Ring modulator rate" ), kRingModRateId, USTRING( "Hz" ),
        Igorski::VST::MIN_RING_MOD_RATE(), Igorski::VST::MAX_RING_MOD_RATE(), Igorski::VST::MIN_RING_MOD_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    ));

    // Bypass
	parameters.addParameter(
        STR16( "Bypass" ), nullptr, 1, 0, ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass, kBypassId
    );

    // Pitch bend
    auto param = new RangeParameter( USTRING( "Pitch Bend" ), kMasterTuningId, USTRING( "cent" ), -200, 200, 0 );
	param->setPrecision( 0 );
	parameters.addParameter( param );

    // Portamento
    parameters.addParameter( new RangeParameter(
        STR16( "Portamento" ), kPortamentoId, USTRING( "ms" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    ));

    // Init Default MIDI-CC Map
	std::for_each( midiCCMapping.begin (), midiCCMapping.end (), [] ( ParamID& pid ) {
        pid = InvalidParamID;
    });
	midiCCMapping[ ControllerNumbers::kPitchBend ] = kMasterTuningId;

    // initialization

    String str( "VSTSID" );
    str.copyTo16( defaultMessageText, 0, 127 );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::terminate()
{
    return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::setComponentState( IBStream* state )
{
    // we receive the current state of the component (processor part)
    if ( !state )
        return kResultFalse;

    IBStreamer streamer( state, kLittleEndian );

    float savedAttack = 1.f;
    if ( streamer.readFloat( savedAttack ) == false )
        return kResultFalse;
    setParamNormalized( kAttackId, savedAttack );

    float savedDecay = 1.f;
    if ( streamer.readFloat( savedDecay ) == false )
        return kResultFalse;
    setParamNormalized( kDecayId, savedDecay );

    float savedSustain = 1.f;
    if ( streamer.readFloat( savedSustain ) == false )
        return kResultFalse;
    setParamNormalized( kSustainId, savedSustain );

    float savedRelease = 1.f;
    if ( streamer.readFloat( savedRelease ) == false )
        return kResultFalse;
    setParamNormalized( kReleaseId, savedRelease );

    float savedCutoff = Igorski::VST::FILTER_MAX_FREQ;
    if ( streamer.readFloat( savedCutoff ) == false )
        return kResultFalse;
    setParamNormalized( kCutoffId, savedCutoff );

    float savedResonance = Igorski::VST::FILTER_MAX_RESONANCE;
    if ( streamer.readFloat( savedResonance ) == false )
        return kResultFalse;
    setParamNormalized( kResonanceId, savedResonance );

    float savedLFORate = Igorski::VST::MIN_LFO_RATE();
    if ( streamer.readFloat( savedLFORate ) == false )
        return kResultFalse;
    setParamNormalized( kLFORateId, savedLFORate );

    float savedLFODepth = 1.f;
    if ( streamer.readFloat( savedLFODepth ) == false )
        return kResultFalse;
    setParamNormalized( kLFODepthId, savedLFODepth );

    float savedRingModRate = Igorski::VST::MIN_RING_MOD_RATE();
    if ( streamer.readFloat( savedRingModRate ) == false )
        return kResultFalse;
    setParamNormalized( kRingModRateId, savedRingModRate );

    // the following properties are allowed to fail (no return) as these were added in later versions

    int32 savedBypass = 0; // added in version 1.0.3
    if ( streamer.readInt32( savedBypass ) != false ) {
        setParamNormalized( kBypassId, savedBypass ? 1 : 0 );
    }

    float savedTuning = 0.f; // added in version 1.1.0
    if ( streamer.readFloat( savedTuning ) != false ) {
        setParamNormalized( kMasterTuningId, ( savedTuning + 1 ) / 2.f );
    }

    int32 savedPortamento = 0; // added in version 1.1.0
    if ( streamer.readInt32( savedPortamento ) != false ) {
        setParamNormalized( kPortamentoId, savedPortamento ? 1 : 0 );
    }

    return kResultOk;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API VSTSIDController::createView( const char* name )
{
    // create the visual editor
    if ( name && strcmp( name, "editor" ) == 0 )
    {
        VST3Editor* view = new VST3Editor( this, "view", "plugin.uidesc" );
        return view;
    }
    return 0;
}

//------------------------------------------------------------------------
IController* VSTSIDController::createSubController( UTF8StringPtr name,
                                                    const IUIDescription* /*description*/,
                                                    VST3Editor* /*editor*/ )
{
    if ( UTF8StringView( name ) == "MessageController" )
    {
        UIMessageController* controller = new UIMessageController( this );
        addUIMessageController( controller );
        return controller;
    }
    return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::setState( IBStream* state )
{
    tresult result = kResultFalse;

    int8 byteOrder;
    if (( result = state->read( &byteOrder, sizeof( int8 ))) != kResultTrue )
        return result;

    if (( result = state->read( defaultMessageText, 128 * sizeof( TChar ))) != kResultTrue )
        return result;

    // if the byteorder doesn't match, byte swap the text array ...
    if ( byteOrder != BYTEORDER )
    {
        for ( int32 i = 0; i < 128; i++ )
            SWAP_16( defaultMessageText[ i ])
    }

    // update our editors
    for ( UIMessageControllerList::iterator it = uiMessageControllers.begin (), end = uiMessageControllers.end (); it != end; ++it )
        ( *it )->setMessageText( defaultMessageText );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::getState( IBStream* state )
{
    // here we can save UI settings for example

    // as we save a Unicode string, we must know the byteorder when setState is called
    int8 byteOrder = BYTEORDER;
    if ( state->write( &byteOrder, sizeof( int8 )) == kResultTrue )
    {
        return state->write( defaultMessageText, 128 * sizeof( TChar ));
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult VSTSIDController::receiveText( const char* text )
{
    // received from Component
    if ( text )
    {
        fprintf( stderr, "[VSTSIDController] received: " );
        fprintf( stderr, "%s", text );
        fprintf( stderr, "\n" );
    }
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::setParamNormalized( ParamID tag, ParamValue value )
{
    // called from host to update our parameters state
    tresult result = EditControllerEx1::setParamNormalized( tag, value );
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::getParamStringByValue( ParamID tag, ParamValue valueNormalized, String128 string )
{
    switch ( tag )
    {
        // ADSR envelopes and LFO depth are floating point values in 0 - 1 range, we can
        // simply read the normalized value which is in the same range

        case kAttackId:
        case kDecayId:
        case kSustainId:
        case kReleaseId:
        case kLFODepthId:
        {
            char text[32];
            sprintf( text, "%.2f", ( float ) valueNormalized );
            Steinberg::UString( string, 128 ).fromAscii( text );

            return kResultTrue;
        }

        // Filter and ring modulator settings are also floating point but in a custom range
        // request the plain value from the normalized value

        case kCutoffId:
        case kResonanceId:
        case kLFORateId:
        case kRingModRateId:
        {
            char text[32];
            if (( tag == kLFORateId || tag == kRingModRateId || tag == kPortamentoId ) && valueNormalized == 0 )
                sprintf( text, "%s", "Off" );
            else
                sprintf( text, "%.2f", normalizedParamToPlain( tag, valueNormalized ));
            
            Steinberg::UString( string, 128 ).fromAscii( text );

            return kResultTrue;
        }

        // everything else
        default:
            return EditControllerEx1::getParamStringByValue( tag, valueNormalized, string );
    }
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::getParamValueByString( ParamID tag, TChar* string, ParamValue& valueNormalized )
{
    /* example, but better to use a custom Parameter as seen in RangeParameter
    switch (tag)
    {
        case kAttackId:
        {
            Steinberg::UString wrapper ((TChar*)string, -1); // don't know buffer size here!
            double tmp = 0.0;
            if (wrapper.scanFloat (tmp))
            {
                valueNormalized = expf (logf (10.f) * (float)tmp / 20.f);
                return kResultTrue;
            }
            return kResultFalse;
        }
    }*/
    return EditControllerEx1::getParamValueByString( tag, string, valueNormalized );
}

//------------------------------------------------------------------------
void VSTSIDController::addUIMessageController( UIMessageController* controller )
{
    uiMessageControllers.push_back( controller );
}

//------------------------------------------------------------------------
void VSTSIDController::removeUIMessageController( UIMessageController* controller )
{
    UIMessageControllerList::const_iterator it = std::find(
        uiMessageControllers.begin(), uiMessageControllers.end (), controller
    );
    if ( it != uiMessageControllers.end())
        uiMessageControllers.erase( it );
}

//------------------------------------------------------------------------
void VSTSIDController::setDefaultMessageText( String128 text )
{
    String tmp( text );
    tmp.copyTo16( defaultMessageText, 0, 127 );
}

//------------------------------------------------------------------------
TChar* VSTSIDController::getDefaultMessageText()
{
    return defaultMessageText;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::queryInterface( const char* iid, void** obj )
{
    QUERY_INTERFACE( iid, obj, IMidiMapping::iid, IMidiMapping );
    return EditControllerEx1::queryInterface( iid, obj );
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::getMidiControllerAssignment( int32 busIndex, int16 channel,
    CtrlNumber midiControllerNumber, ParamID& id /*out*/)
{
    if ( busIndex == 0 && channel == 0 && midiControllerNumber < kCountCtrlNumber ) {
		if ( midiCCMapping[ midiControllerNumber ] != InvalidParamID ) {
			id = midiCCMapping[ midiControllerNumber ];
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
