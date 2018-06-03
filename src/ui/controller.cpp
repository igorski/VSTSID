/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - https://www.igorski.nl
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

    RangeParameter* attackParam = new RangeParameter(
        USTRING( "Attack time" ), kAttackId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( attackParam );

    RangeParameter* decayParam = new RangeParameter(
        USTRING( "Decay time" ), kDecayId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
     );
    parameters.addParameter( decayParam );

    RangeParameter* sustainParam = new RangeParameter(
        USTRING( "Sustain volume" ), kSustainId, USTRING( "0 - 1" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( sustainParam );

    RangeParameter* releaseParam = new RangeParameter(
        USTRING( "Release time" ), kReleaseId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( releaseParam );

    // filter controls

    RangeParameter* cutoffParam = new RangeParameter(
        USTRING( "Cutoff frequency" ), kCutoffId, USTRING( "Hz" ),
        Igorski::SID::FILTER_MIN_FREQ, Igorski::SID::FILTER_MAX_FREQ, Igorski::SID::FILTER_MIN_FREQ,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( cutoffParam );

    RangeParameter* resonanceParam = new RangeParameter(
        USTRING( "Resonance" ), kResonanceId, USTRING( "db" ),
        Igorski::SID::FILTER_MIN_RESONANCE, Igorski::SID::FILTER_MAX_RESONANCE, Igorski::SID::FILTER_MIN_RESONANCE,
        0, ParameterInfo::kCanAutomate, unitId
   );
    parameters.addParameter( resonanceParam );

    RangeParameter* lfoRateParam = new RangeParameter(
        USTRING( "LFO rate" ), kLFORateId, USTRING( "Hz" ),
        Igorski::SID::MIN_LFO_RATE(), Igorski::SID::MAX_LFO_RATE(), Igorski::SID::MIN_LFO_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( lfoRateParam );

    RangeParameter* lfoDepthParam = new RangeParameter(
        USTRING( "LFO depth" ), kLFODepthId, USTRING( "%" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( lfoDepthParam );

    // ring modulator

    RangeParameter* ringModRateParam = new RangeParameter(
        USTRING( "Ring modulator rate" ), kRingModRateId, USTRING( "Hz" ),
        Igorski::SID::MIN_RING_MOD_RATE(), Igorski::SID::MAX_RING_MOD_RATE(), Igorski::SID::MIN_RING_MOD_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( ringModRateParam );

    // initialization

    String str( "VST SID" );
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
    if ( state )
    {
        float savedAttack = 1.f;
        if ( state->read( &savedAttack, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedDecay = 1.f;
        if ( state->read( &savedDecay, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedSustain = 1.f;
        if ( state->read( &savedSustain, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedRelease = 1.f;
        if ( state->read( &savedRelease, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedCutoff = Igorski::SID::FILTER_MAX_FREQ;
        if ( state->read( &savedCutoff, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedResonance = Igorski::SID::FILTER_MAX_RESONANCE;
        if ( state->read( &savedResonance, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFORate = Igorski::SID::MIN_LFO_RATE();
        if ( state->read( &savedLFORate, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFODepth = 1.f;
        if ( state->read( &savedLFODepth, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedRingModRate = Igorski::SID::MIN_RING_MOD_RATE();
        if ( state->read( &savedRingModRate, sizeof( float )) != kResultOk )
            return kResultFalse;

#if BYTEORDER == kBigEndian
    SWAP_32( savedAttack )
    SWAP_32( savedDecay )
    SWAP_32( savedSustain )
    SWAP_32( savedRelease )
    SWAP_32( savedCutoff )
    SWAP_32( savedResonance )
    SWAP_32( savedLFORate )
    SWAP_32( savedLFODepth )
    SWAP_32( savedRingModRate )
#endif
        setParamNormalized( kAttackId,      savedAttack );
        setParamNormalized( kDecayId,       savedDecay );
        setParamNormalized( kSustainId,     savedSustain );
        setParamNormalized( kReleaseId,     savedRelease );
        setParamNormalized( kCutoffId,      savedCutoff );
        setParamNormalized( kResonanceId,   savedResonance );
        setParamNormalized( kLFORateId,     savedLFORate );
        setParamNormalized( kLFODepthId,    savedLFODepth );
        setParamNormalized( kRingModRateId, savedRingModRate );

        state->seek( sizeof ( float ), IBStream::kIBSeekCur );
    }
    return kResultOk;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API VSTSIDController::createView( const char* name )
{
    // create the visual editor
    if ( name && strcmp( name, "editor" ) == 0 )
    {
        VST3Editor* view = new VST3Editor( this, "view", "vstsid.uidesc" );
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
            if (( tag == kLFORateId || tag == kRingModRateId ) && valueNormalized == 0 )
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
tresult PLUGIN_API VSTSIDController::getMidiControllerAssignment( int32 busIndex, int16 /*midiChannel*/,
    CtrlNumber midiControllerNumber, ParamID& tag )
{
    // we support for the Gain parameter all MIDI Channel but only first bus (there is only one!)
/*
    if ( busIndex == 0 && midiControllerNumber == kCtrlVolume )
    {
        tag = kAttackId;
        return kResultTrue;
    }
*/
    return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
