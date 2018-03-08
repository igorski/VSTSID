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
// GainParameter Declaration
// example of custom parameter (overwriting to and fromString)
//------------------------------------------------------------------------
class GainParameter : public Parameter
{
    public:
        GainParameter( int32 flags, int32 id, const char* title, const char* units );

        void toString( ParamValue normValue, String128 string ) const SMTG_OVERRIDE;
        bool fromString( const TChar* string, ParamValue& normValue ) const SMTG_OVERRIDE;
};

//------------------------------------------------------------------------
// GainParameter Implementation
//------------------------------------------------------------------------
GainParameter::GainParameter( int32 flags, int32 id, const char* title, const char* units )
{
    Steinberg::UString( info.title, USTRINGSIZE( info.title )).assign( USTRING( title ));
    Steinberg::UString( info.units, USTRINGSIZE( info.units )).assign( USTRING( units ));

    info.flags = flags;
    info.id = id;
    info.stepCount = 0;
    info.defaultNormalizedValue = 0.5f;
    info.unitId = kRootUnitId;

    setNormalized (1.f);
}

//------------------------------------------------------------------------
void GainParameter::toString( ParamValue normValue, String128 string ) const
{
    char text[32];
    sprintf( text, "%.2f", ( float ) normValue );
    Steinberg::UString( string, 128 ).fromAscii( text );
}

//------------------------------------------------------------------------
bool GainParameter::fromString (const TChar* string, ParamValue& normValue) const
{
    String wrapper(( TChar* ) string ); // don't know buffer size here!
    double tmp = 0.0;
    if ( wrapper.scanFloat( tmp ))
    {
        // allow only values between 0 and 1
        if ( tmp < 0.0 )
            tmp = 0.0;
        else if ( tmp > 1.0 )
            tmp = 1.0;

        normValue = tmp;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
// VSTSIDController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::initialize( FUnknown* context )
{
    tresult result = EditControllerEx1::initialize( context );
    if (result != kResultOk)
    {
        return result;
    }

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

    // create a unit1 for the ADSR
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId;    // attached to the root unit

    Steinberg::UString( unitInfo.name, USTRINGSIZE( unitInfo.name )).assign( USTRING( "ADSR" ));

    unitInfo.programListId = kNoProgramListId;

    unit = new Unit( unitInfo );
    addUnit( unit );

    //---Create ADSR Parameters------------

    GainParameter* attackParam = new GainParameter( ParameterInfo::kCanAutomate, kAttackId, "AttackTime", "seconds" );
    parameters.addParameter( attackParam );
    attackParam->setUnitID( 1 );

    GainParameter* decayParam = new GainParameter( ParameterInfo::kCanAutomate, kDecayId, "DecayTime", "seconds" );
    parameters.addParameter( decayParam );
    decayParam->setUnitID( 1 );

    GainParameter* sustainParam = new GainParameter( ParameterInfo::kCanAutomate, kSustainId, "SustainVolume", "0 - 1" );
    parameters.addParameter( sustainParam );
    sustainParam->setUnitID( 1 );

    GainParameter* releaseParam = new GainParameter( ParameterInfo::kCanAutomate, kReleaseId, "ReleaseTime", "seconds" );
    parameters.addParameter( releaseParam );
    releaseParam->setUnitID( 1 );

    GainParameter* cutoffParam = new GainParameter( ParameterInfo::kCanAutomate, kCutoffId, "Cutoff frequency", "Hz" );
    parameters.addParameter( cutoffParam );
    cutoffParam->setUnitID( 1 );

    GainParameter* resonanceParam = new GainParameter( ParameterInfo::kCanAutomate, kResonanceId, "Resonance", "dB" );
    parameters.addParameter( resonanceParam );
    resonanceParam->setUnitID( 1 );

    //---Custom state init------------

    String str( "VST SID" );
    str.copyTo16( defaultMessageText, 0, 127 );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::terminate  ()
{
    return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::setComponentState( IBStream* state )
{
    // we receive the current state of the component (processor part)
    // we read only the ADSR envelope values
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

        float savedCutoff = 1.f;
        if ( state->read( &savedCutoff, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedResonance = 1.f;
        if ( state->read( &savedResonance, sizeof( float )) != kResultOk )
            return kResultFalse;

#if BYTEORDER == kBigEndian
    SWAP_32( savedAttack )
    SWAP_32( savedDecay )
    SWAP_32( savedSustain )
    SWAP_32( savedRelease )
    SWAP_32( savedCutoff )
    SWAP_32( savedResonance )
#endif
        setParamNormalized( kAttackId,    savedAttack );
        setParamNormalized( kDecayId,     savedDecay );
        setParamNormalized( kSustainId,   savedSustain );
        setParamNormalized( kReleaseId,   savedRelease );
        setParamNormalized( kCutoffId,    savedCutoff );
        setParamNormalized( kResonanceId, savedResonance );

        // jump the GainReduction
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
        VST3Editor* view = new VST3Editor( this, "view", "vstsid.xml" );
        return view;
    }
    return 0;
}

//------------------------------------------------------------------------
IController* VSTSIDController::createSubController (UTF8StringPtr name,
                                                   const IUIDescription* /*description*/,
                                                   VST3Editor* /*editor*/)
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
tresult PLUGIN_API VSTSIDController::setState (IBStream* state)
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
        for (int32 i = 0; i < 128; i++)
            SWAP_16( defaultMessageText[ i ])
    }

    // update our editors
    for ( UIMessageControllerList::iterator it = uiMessageControllers.begin (), end = uiMessageControllers.end (); it != end; ++it )
        ( *it )->setMessageText( defaultMessageText );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSIDController::getState (IBStream* state)
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
    if (text)
    {
        fprintf (stderr, "[VSTSIDController] received: ");
        fprintf (stderr, "%s", text);
        fprintf (stderr, "\n");
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
        // ADSR envelopes are floating point values in 0 - 1 range

        case kAttackId:
        case kDecayId:
        case kSustainId:
        case kReleaseId:
        {
            char text[32];
            sprintf( text, "%.2f", ( float ) valueNormalized );
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
    /* example, but better to use a custom Parameter as seen in GainParameter
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
    CtrlNumber midiControllerNumber, ParamID& tag)
{
    // we support for the Gain parameter all MIDI Channel but only first bus (there is only one!)
    if ( busIndex == 0 && midiControllerNumber == kCtrlVolume )
    {
        tag = kAttackId;
        return kResultTrue;
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
