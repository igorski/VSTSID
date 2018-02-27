#include <stdio.h>
#include <string.h>
#include "global.h"
#include "vst.h"

VSTProgram::VSTProgram ()
{
    fGain = 0.75;
    strcpy (name, "Init");
}


VST::VST( audioMasterCallback audioMaster )
    : AudioEffectX( audioMaster, 1, kNumParams )
{
    programs = new VSTProgram[ 1 ];
    fGain = 0.75;

    if ( programs ) {
        setProgram( 0 );
    }

    setNumInputs ( 2 );
    setNumOutputs( 2 );

    setUniqueID( Global::ID );

    resume();
}


VST::~VST ()
{
    if ( programs ) {
        delete[] programs;
    }
}


void VST::setProgram( VstInt32 program )
{
    VSTProgram* ap = &programs[program];
    curProgram = program;
    setParameter(kGain, ap->fGain);
}

void VST::setProgramName( char *name )
{
    strcpy( programs[ curProgram ].name, name );
}

void VST::getProgramName (char *name)
{
    if ( !strcmp( programs[ curProgram ].name, "Init" )) {
        sprintf( name, "%s %d", programs[ curProgram ].name, curProgram + 1 );
    }
    else {
        strcpy( name, programs[ curProgram ].name );
    }
}

bool VST::getProgramNameIndexed( VstInt32 category, VstInt32 index, char* text )
{
    if ( index < numPrograms ) {
        strcpy( text, programs[ index ].name );
        return true;
    }
    return false;
}

void VST::resume ()
{
    AudioEffectX::resume();
}

void VST::setParameter( VstInt32 index, float value )
{
    VSTProgram* ap = &programs[curProgram];

    switch ( index ) {
        case kGain:
            fGain = ap->fGain = value;
            break;
    }
}

float VST::getParameter (VstInt32 index)
{
    float v = 0;

    switch (index) {
        case kGain:
            v = fGain;
            break;
    }
    return v;
}

void VST::getParameterName (VstInt32 index, char *label)
{
    switch( index ) {
        case kGain:
            strcpy( label, "Gain" );
            break;
    }
}


void VST::getParameterDisplay( VstInt32 index, char *text )
{
    switch ( index ) {
        case kGain:
            float2string( fGain, text, kVstMaxParamStrLen );
            break;
    }
}


void VST::getParameterLabel (VstInt32 index, char *label)
{
    switch ( index ) {
        case kGain: strcpy( label, "" );
            break;
    }
}


bool VST::getEffectName( char* name )
{
    strcpy( name, Global::NAME );
    return true;
}


bool VST::getProductString( char* text )
{
    strcpy( text, Global::NAME );
    return true;
}


bool VST::getVendorString( char* text )
{
    strcpy( text, Global::VENDOR );
    return true;
}


void VST::processReplacing( float** inputs, float** outputs, VstInt32 sampleFrames )
{
    VstInt32 current = sampleFrames;
    while ( --current >= 0 ) {
        outputs[ 0 ][ current ] = inputs[ 0 ][ current ] * fGain;
        outputs[ 1 ][ current ] = inputs[ 1 ][ current ] * fGain;
    }
}
