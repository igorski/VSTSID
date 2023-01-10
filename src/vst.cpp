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
#include "global.h"
#include "calc.h"
#include "vst.h"
#include "synth.h"
#include "filter.h"
#include "paramids.h"

#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vstpresetkeys.h"

#include "base/source/fstreamer.h"

#include <stdio.h>

float Igorski::VST::SAMPLE_RATE = 44100.f; // updated in setupProcessing()

namespace Igorski {

//------------------------------------------------------------------------
// VSTSID Implementation
//------------------------------------------------------------------------
VSTSID::VSTSID ()
: fAttack( 0.f )
, fDecay( 1.f )
, fSustain( .5f )
, fRelease( 0.f )
, fCutoff( .5f )
, fResonance( 0.f )
, fLFORate( 0.f )
, fLFODepth( 1.f )
, fRingModRate( 0.f )
, fPitchBendRange( 1.f )
, fPortamento( 0.f )
, currentProcessMode( -1 ) // -1 means not initialized
{
    // register its editor class (the same as used in entry.cpp)
    setControllerClass( Igorski::VST::ControllerUID );

    // should be created on setupProcessing, this however doesn't fire for Audio Unit using auval?
    initPlugin( Igorski::VST::SAMPLE_RATE );
}

//------------------------------------------------------------------------
VSTSID::~VSTSID ()
{
    // free all allocated resources
    delete synth;
    delete filter;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::initialize( FUnknown* context )
{
    tresult result = AudioEffect::initialize( context );

    if ( result != kResultOk )
        return result;

    //---create Audio In/Out buses------
    addAudioInput ( STR16( "Stereo In" ),  SpeakerArr::kStereo );
    addAudioOutput( STR16( "Stereo Out" ), SpeakerArr::kStereo );

    //---create Event In/Out buses (1 bus with only 1 channel)------
    addEventInput( STR16( "Event In" ), 1 );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::terminate()
{
    // nothing to do here yet...except calling our parent terminate
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::setActive (TBool state)
{
    if (state)
        sendTextMessage( "VSTSID::setActive (true)" );
    else
        sendTextMessage( "VSTSID::setActive (false)" );

    // call our parent setActive
    return AudioEffect::setActive( state );
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::process( ProcessData& data )
{
    // In this example there are 4 steps:
    // 1) Read inputs parameters coming from host (in order to adapt our model values)
    // 2) Read inputs events coming from host (note on/off events)
    // 3) Process the gain of the input buffer to the output buffer

    //---1) Read input parameter changes-----------
    IParameterChanges* paramChanges = data.inputParameterChanges;
    if ( paramChanges )
    {
        int32 numParamsChanged = paramChanges->getParameterCount();
        // for each parameter which are some changes in this audio block:
        for ( int32 i = 0; i < numParamsChanged; i++ )
        {
            IParamValueQueue* paramQueue = paramChanges->getParameterData( i );
            if ( paramQueue )
            {
                ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();

                if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) != kResultTrue ) {
                    continue;
                }

                switch ( paramQueue->getParameterId())
                {
                    // we use in this example only the last point of the queue.
                    // in some wanted case for specific kind of parameter it makes sense to retrieve all points
                    // and process the whole audio block in small blocks.

                    case kAttackId:
                        fAttack = ( float ) value;
                        break;

                    case kDecayId:
                        fDecay = ( float ) value;
                        break;

                    case kSustainId:
                        fSustain = ( float ) value;
                        break;

                    case kReleaseId:
                        fRelease = ( float ) value;
                        break;

                    case kCutoffId:
                        fCutoff = ( float ) value;
                        break;

                    case kResonanceId:
                        fResonance = ( float ) value;
                        break;

                    case kLFORateId:
                        fLFORate = ( float ) value;
                        break;

                    case kLFODepthId:
                        fLFODepth = ( float ) value;
                        break;

                    case kRingModRateId:
                        fRingModRate = ( float ) value;
                        break;

                    case kBypassId:
                        _bypass = ( value > 0.5f );
                        break;

                    case kMasterTuningId:
						fMasterTuning = 2 * ( value - 0.5f ); // -1 to +1 range
                        scaleTuning();
						break;

                    case kPitchBendRangeId:
                        fPitchBendRange = ( float ) value;
                        scaleTuning();
                        break;

                    case kPortamentoId:
                        fPortamento = ( float ) value;
                        break;
                }
                syncModel();
            }
        }
    }

    // according to docs: processing context (optional, but most welcome)

    if ( data.processContext != nullptr && synth->TEMPO != data.processContext->tempo ) {
        synth->init(( int ) data.processContext->sampleRate, data.processContext->tempo );
    }

    //---2) Read input events-------------
    IEventList* eventList = data.inputEvents;
    if ( eventList )
    {
        int32 numEvent = eventList->getEventCount();
        for ( int32 i = 0; i < numEvent; i++ )
        {
            Event event;
            if ( eventList->getEvent( i, event ) == kResultOk )
            {
                switch ( event.type )
                {
                    //----------------------
                    case Event::kNoteOnEvent:
                        // event has properties: channel, pitch, velocity, length, tuning, noteId
                        synth->noteOn( event.noteOn.pitch, event.noteOn.velocity, event.noteOn.tuning );
                        break;

                    //----------------------
                    case Event::kNoteOffEvent:
                        // noteOff reset the reduction
                        synth->noteOff( event.noteOff.pitch );
                        break;
                }
            }
        }
    }

    //-------------------------------------
    //---3) Process Audio---------------------
    //-------------------------------------

    if ( data.numOutputs == 0 ) {
        // nothing to do
        return kResultOk;
    }

    int32 numChannels = data.outputs[ 0 ].numChannels;

    // --- get audio buffers----------------
    uint32 sampleFramesSize = getSampleFramesSizeInBytes( processSetup, data.numSamples );
//    void** in  = getChannelBuffersPointer( processSetup, data.inputs [ 0 ] );
    void** out = getChannelBuffersPointer( processSetup, data.outputs[ 0 ] );

    bool hasContent = false;

    if ( !_bypass )
    {
        // synthesize !
        hasContent = synth->synthesize(
            ( float** ) out, numChannels, data.numSamples, sampleFramesSize
        );

        if ( hasContent ) {
            filter->process(( float** ) out, numChannels, data.numSamples );
        }
    }

    // mark our outputs as not silent if content had been synthesized
    data.outputs[ 0 ].silenceFlags = !hasContent;

    return kResultOk;
}

//------------------------------------------------------------------------
tresult VSTSID::receiveText( const char* text )
{
    // received from Controller
    fprintf( stderr, "[VSTSID] received: " );
    fprintf( stderr, "%s", text );
    fprintf( stderr, "\n" );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::setState( IBStream* state )
{
    IBStreamer streamer( state, kLittleEndian );

    // called when we load a preset, the model has to be reloaded

    float savedAttack = 0.f;
    if ( streamer.readFloat( savedAttack ) == false )
        return kResultFalse;

    float savedDecay = 0.f;
    if ( streamer.readFloat( savedDecay ) == false )
        return kResultFalse;

    float savedSustain = 0.f;
    if ( streamer.readFloat( savedSustain ) == false )
        return kResultFalse;

    float savedRelease = 0.f;
    if ( streamer.readFloat( savedRelease ) == false )
        return kResultFalse;

    float savedCutoff = 0.f;
    if ( streamer.readFloat( savedCutoff ) == false )
        return kResultFalse;

    float savedResonance = 0.f;
    if ( streamer.readFloat( savedResonance ) == false )
        return kResultFalse;

    float savedLFORate = 0.f;
    if ( streamer.readFloat( savedLFORate ) == false )
        return kResultFalse;

    float savedLFODepth = 1.f;
    if ( streamer.readFloat( savedLFODepth ) == false )
        return kResultFalse;

    float savedRingModRate = 0.f;
    if ( streamer.readFloat( savedRingModRate ) == false )
        return kResultFalse;

    // may fail as this was only added in version 1.0.3
    int32 savedBypass = 0;
    if ( streamer.readInt32( savedBypass ) != false ) {
        _bypass = savedBypass > 0;
    }

    // may fail as these were only added in version 1.1.0
    float savedPBrange = 0.f;
    if ( streamer.readFloat( savedPBrange ) != false ) {
        fPitchBendRange = savedPBrange;
    }

    float savedPortamento = 0;
    if ( streamer.readFloat( savedPortamento ) != false ) {
        fPortamento = savedPortamento;
    }

    fAttack      = savedAttack;
    fDecay       = savedDecay;
    fSustain     = savedSustain;
    fRelease     = savedRelease;
    fCutoff      = savedCutoff;
    fResonance   = savedResonance;
    fLFORate     = savedLFORate;
    fLFODepth    = savedLFODepth;
    fRingModRate = savedRingModRate;

    syncModel();

    // Example of using the IStreamAttributes interface
    FUnknownPtr<IStreamAttributes> stream (state);
    if ( stream )
    {
        IAttributeList* list = stream->getAttributes ();
        if ( list )
        {
            // get the current type (project/Default..) of this state
            String128 string = {0};
            if ( list->getString( PresetAttributes::kStateType, string, 128 * sizeof( TChar )) == kResultTrue )
            {
                UString128 tmp( string );
                char ascii[128];
                tmp.toAscii( ascii, 128 );
                if ( !strncmp( ascii, StateType::kProject, strlen( StateType::kProject )))
                {
                    // we are in project loading context...
                }
            }

            // get the full file path of this state
            TChar fullPath[1024];
            memset( fullPath, 0, 1024 * sizeof( TChar ));
            if ( list->getString( PresetAttributes::kFilePathStringType,
                 fullPath, 1024 * sizeof( TChar )) == kResultTrue )
            {
                // here we have the full path ...
            }
        }
    }
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::getState( IBStream* state )
{
    // here we save the model values

    IBStreamer streamer( state, kLittleEndian );

    streamer.writeFloat( fAttack );
    streamer.writeFloat( fDecay );
    streamer.writeFloat( fSustain );
    streamer.writeFloat( fRelease );
    streamer.writeFloat( fCutoff );
    streamer.writeFloat( fResonance );
    streamer.writeFloat( fLFORate );
    streamer.writeFloat( fLFODepth );
    streamer.writeFloat( fRingModRate );
    streamer.writeInt32( _bypass ? 1 : 0 );
    streamer.writeFloat( fPitchBendRange );
    streamer.writeFloat( fPortamento );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::setupProcessing( ProcessSetup& newSetup )
{
    // called before the process call, always in a disabled state (not active)

    // here we keep a trace of the processing mode (offline,...) for example.
    currentProcessMode = newSetup.processMode;

    Igorski::VST::SAMPLE_RATE = newSetup.sampleRate;

    initPlugin( newSetup.sampleRate );

    return AudioEffect::setupProcessing( newSetup );
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::setBusArrangements( SpeakerArrangement* inputs,  int32 numIns,
                                               SpeakerArrangement* outputs, int32 numOuts )
{
    if ( numIns == 1 && numOuts == 1 )
    {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if ( SpeakerArr::getChannelCount( inputs[ 0 ])  == 1 &&
             SpeakerArr::getChannelCount( outputs[ 0 ]) == 1 )
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at( 0 ));
            if ( bus )
            {
                // check if we are Mono => Mono, if not we need to recreate the buses
                if ( bus->getArrangement () != inputs[ 0 ])
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Mono In" ),  inputs[ 0 ] );
                    addAudioOutput( STR16( "Mono Out" ), inputs[ 0 ] );
                }
                return kResultOk;
            }
        }
        // the host wants something else than Mono => Mono, in this case we are always Stereo => Stereo
        else
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at( 0 ));
            if ( bus )
            {
                tresult result = kResultFalse;

                // the host wants 2->2 (could be LsRs -> LsRs)
                if ( SpeakerArr::getChannelCount (inputs[ 0 ]) == 2 && SpeakerArr::getChannelCount( outputs[ 0 ]) == 2 )
                {
                    removeAudioBusses();
                    addAudioInput  ( STR16( "Stereo In" ),  inputs[ 0 ] );
                    addAudioOutput ( STR16( "Stereo Out" ), outputs[ 0 ]);
                    result = kResultTrue;
                }
                // the host want something different than 1->1 or 2->2 : in this case we want stereo
                else if ( bus->getArrangement () != SpeakerArr::kStereo )
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Stereo In" ),  SpeakerArr::kStereo );
                    addAudioOutput( STR16( "Stereo Out" ), SpeakerArr::kStereo );
                    result = kResultFalse;
                }

                return result;
            }
        }
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::canProcessSampleSize( int32 symbolicSampleSize )
{
    if ( symbolicSampleSize == kSample32 )
        return kResultTrue;

    // we support double processing
    if ( symbolicSampleSize == kSample64 )
        return kResultTrue;

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VSTSID::notify( IMessage* message )
{
    if ( !message )
        return kInvalidArgument;

    if ( !strcmp( message->getMessageID(), "BinaryMessage" ))
    {
        const void* data;
        uint32 size;
        if ( message->getAttributes ()->getBinary( "MyData", data, size ) == kResultOk )
        {
            // we are in UI thread
            // size should be 100
            if ( size == 100 && (( char* )data )[ 1 ] == 1 ) // yeah...
            {
                fprintf( stderr, "[VSTSID] received the binary message!\n" );
            }
            return kResultOk;
        }
    }

    return AudioEffect::notify( message );
}

void VSTSID::initPlugin( float sampleRate )
{
    if ( synth != nullptr ) {
        delete synth;
        delete filter;
    }

    synth = new Igorski::Synthesizer();
    synth->init( sampleRate, 120.f );

    filter = new Igorski::Filter( sampleRate );

    syncModel();
}

void VSTSID::scaleTuning()
{
    _scaledTuning = Calc::pitchShiftFactor( fMasterTuning * round( fPitchBendRange * VST::MAX_PITCH_BEND ));
}

void VSTSID::syncModel()
{
    synth->updateProperties( fAttack, fDecay, fSustain, fRelease, fRingModRate, _scaledTuning, fPortamento );
    filter->updateProperties( fCutoff, Calc::inverseNormalize( fResonance ), fLFORate, fLFODepth );
}

//------------------------------------------------------------------------
} // namespace Igorski
