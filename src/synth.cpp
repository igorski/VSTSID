/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2024 Igor Zinken - https://www.igorski.nl
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
#include "synth.h"
#include "calc.h"
#include "miditable.h"
#include <algorithm>
#include <math.h>
#include <climits>

using namespace Steinberg;

namespace Igorski {

Synthesizer::Synthesizer()
{
    TEMPO = 120.f;

    TWO_PI_OVER_SR       = TWO_PI / 44100.f;
    SAMPLE_RATE          = 44100;
    BUFFER_SIZE          = 256;
    MAX_ENVELOPE_SAMPLES = 44100;
    ARPEGGIO_DURATION    = 16;

    ringModulator = new Steinberg::Vst::mda::RingModulator();

    note_ids = 0;
}

Synthesizer::~Synthesizer() {
    reset();

    delete ringModulator;
    ringModulator = nullptr;
}

void Synthesizer::init( int sampleRate, double tempo )
{
    SAMPLE_RATE    = sampleRate;
    TWO_PI_OVER_SR = TWO_PI / ( float ) SAMPLE_RATE;

    // max envelope length is the desired envelope time in seconds, translated to buffer samples
    MAX_ENVELOPE_SAMPLES = ( int ) round( 1.f * SAMPLE_RATE );

    TEMPO = tempo;
}

void Synthesizer::noteOn( int16 pitch, float normalizedVelocity, float tuning )
{
    // do not allow a noteOn for the same pitch twice
    if ( getExistingNote( pitch ) != nullptr )
        removeNote( getExistingNote( pitch ));

    Note* note = nullptr;

    if ( doGlide() ) {

        // when glide/portamento is enabled, get the previous sounding note
        // that currently isn't gliding

        for ( size_t i = 0, l = notes.size(); i < l; ++i ) {
            auto compareNote = notes.at( i );
            if ( /*!compareNote->portamento.enabled &&*/ !compareNote->released ) {
                // store the last/original pitch that the note is synthesizing in the orgPitches list
                // so we can return to the pitch (if it hasn't been noteOff'ed yet) on noteOff
                auto list = compareNote->portamento.orgPitches;
                if ( std::find( std::begin( list ), std::end( list ), compareNote->pitch ) == std::end( list )) {
                    compareNote->portamento.orgPitches.push_back( compareNote->pitch );
                }
                note = compareNote;
                note->pitch = pitch; // update "ownership" of note by adjusting pitch
                break;
            }
        }

        if ( note != nullptr ) {
            float targetFrequency = MIDITable::frequencies[ pitch ];

            note->portamento.enabled   = true;
            note->portamento.steps     = Igorski::Calc::millisecondsToBuffer( 1000.f * props.glide );
            note->portamento.increment = ( targetFrequency - note->frequency ) / note->portamento.steps;

            return;
        }
    }

    float tuningDelta = 1.f;

    if ( tuning != 0.f ) {
        // given tuning defines a detuning in cents (e.g. 1.f = +1 cent, -1.f = -1 cent)
        tuningDelta = Calc::pitchShiftFactor( tuning / 100.f );
    }

    note = new Note();

    note->id             = generateNextNoteId();
    note->pitch          = pitch;
    note->volume         = normalizedVelocity;
    note->released       = false;
    note->muted          = false;
    note->baseFrequency  = MIDITable::frequencies[ pitch ] * tuningDelta;
    note->frequency      = note->baseFrequency;
    note->phase          = 0.f;
    note->pwm            = 0.f;
    note->arpIndex       = 0;
    note->arpOffset      = 0;

    notes.push_back( note );
    handleNoteAmountChange();

    if ( doArpeggiate ) {
        int fullMeasure   = round((( float ) SAMPLE_RATE * 60.f ) / TEMPO );
        ARPEGGIO_DURATION = fullMeasure / getArpeggiatorSpeedByTempo( TEMPO );
    }

    // set Note ADSR properties from current model

    note->adsr.attack          = props.attack;
    note->adsr.attackValue     = 0.f;
    note->adsr.attackDuration  = ( float ) MAX_ENVELOPE_SAMPLES * note->adsr.attack;
    note->adsr.attackIncrement = 1.0f / std::max( 1.0f, note->adsr.attackDuration );

    note->adsr.sustain = props.sustain;

    note->adsr.decay          = props.decay;
    note->adsr.decayValue     = 0.f;
    note->adsr.decayDuration  = ( float ) MAX_ENVELOPE_SAMPLES * note->adsr.decay;
    note->adsr.decayIncrement = ( 1.0f - note->adsr.sustain ) / std::max( 1.0f, note->adsr.decayDuration );

    // release is only set on noteOff (so the last known release is used if it is updated during Note playback)
}

void Synthesizer::noteOff( int16 pitch )
{
    Note* note = getExistingNote( pitch );

    if ( doGlide() ) {
        for ( Note* compareNote : notes ) {
            auto pitchList = &compareNote->portamento.orgPitches;
            // remove pitch from other playing notes pitch history
            if ( std::find( pitchList->begin(), pitchList->end(), pitch ) != pitchList->end()) {
                pitchList->erase( std::find( pitchList->begin(), pitchList->end(), pitch ));
            }
        }

        if ( note != nullptr && restorePitchOnRelease( note )) {
            return; // pitch is restored on existing note, keep note in list
        }
    }

    if ( note == nullptr ) {
        return; // likely first note in a portamento sequence (Note has changed "pitch ownership")
    }

    // instant removal when release is at 0 or note's sustain level is at 0

    if ( props.release == 0 || note->adsr.sustain == 0 ) {
        removeNote( note );
        return;
    }

    // apply release envelope to this note (will be disposed in render loop)

    if ( !note->released ) {
        note->adsr.release          = props.release;
        note->adsr.releaseValue     = 0.f;
        note->adsr.releaseDuration  = ( float ) MAX_ENVELOPE_SAMPLES * note->adsr.release;
        note->adsr.releaseIncrement = note->adsr.sustain / std::max( 1.f, note->adsr.releaseDuration );

        note->released = true;

        // if the SID was arpeggiating, update the note amount
        // accordingly to toggle the proper arpeggiation state

        if ( doArpeggiate ) {
            arpeggiatedNotes.erase( std::find( arpeggiatedNotes.begin(), arpeggiatedNotes.end(), note->id ));
            handleNoteAmountChange();
        }
    }
}

Note* Synthesizer::getExistingNote( int16 pitch )
{
    for ( int32 i = 0; i < notes.size(); ++i ) {
        if ( notes.at( i )->pitch == pitch ) {
            return notes.at( i );
        }
    }
    return nullptr;
}

Note* Synthesizer::getNoteById( int32 id )
{
    for ( int32 i = 0; i < notes.size(); ++i ) {
        if ( notes.at( i )->id == id )
            return notes.at( i );
    }
    return nullptr;
}

bool Synthesizer::removeNote( Note* note )
{
    bool removed = false;

    // first remove from arpeggiated notes vector

    if ( isArpeggiatedNote( note )) {
       arpeggiatedNotes.erase( std::find( arpeggiatedNotes.begin(), arpeggiatedNotes.end(), note->id ));
    }

    // and lastly, remove from notes vector and free memory

    if ( std::find( notes.begin(), notes.end(), note ) != notes.end()) {
        notes.erase( std::find( notes.begin(), notes.end(), note ));
        handleNoteAmountChange();
        delete note;
        removed = true;
    }
    return removed;
}

void Synthesizer::reset()
{
    while ( notes.size() > 0 ) {
        removeNote( notes.at( 0 ));
    }
    arpeggiatedNotes.clear();
    note_ids = 0;
}

void Synthesizer::handleNoteAmountChange()
{
    int amountOfNotes = notes.size();
    int activeNotes   = 0;

    int32 i;

    for ( i = 0; i < amountOfNotes; ++i )
    {
        if ( !notes.at( i )->released )
            ++activeNotes;
    }
    // we only arpeggiate when the current amount of unreleased
    // notes meets or exceeds the arpeggiator threshold
    // (released notes should not arpeggiate)

    doArpeggiate = activeNotes >= ARPEGGIATOR_THRESHOLD;

    for ( i = 0; i < amountOfNotes; ++i )
    {
        Note* note = notes.at( i );

        // when arpeggiated, we impose a limit on audible notes as we will use
        // a pitch cycle in the render function to play the muted notes frequency
        // once the arpeggiator step shifts to the next step

        bool audible = !doArpeggiate;
        note->muted  = !audible;

        // when arpeggiating, add note to arpeggiated notes vector

        if ( doArpeggiate && !isArpeggiatedNote( note )) {
            arpeggiatedNotes.push_back( note->id );
        }
    }

    if ( !doArpeggiate )
        return;

    for ( i = 0; i < amountOfNotes; ++i ) {

        // when arpeggiating we'd like the first
        // unreleased note to be unmuted as it will
        // be the note to cycle through all frequencies

        if ( !notes.at( i )->released ) {
            notes.at( i )->muted = false;
            break;
        }
    }
}

void Synthesizer::updateProperties( float attack, float decay, float sustain, float release, float ringModRate, float pitchBend, float portamento )
{
    props.attack    = attack;
    props.decay     = decay;
    props.sustain   = sustain;
    props.release   = release;
    props.pitchBend = pitchBend;
    props.glide     = portamento;

    ringModulator->setRate( ringModRate );
}

bool Synthesizer::restorePitchOnRelease( Note* note )
{
    if ( !note->portamento.enabled || note->portamento.orgPitches.size() == 0 ) {
        return false;
    }
    auto lastPitch = note->portamento.orgPitches.back();
    float targetFrequency = MIDITable::frequencies[ lastPitch ];

    note->pitch = lastPitch;
    note->portamento.orgPitches.pop_back();

    note->portamento.steps     = Igorski::Calc::millisecondsToBuffer( 1000.f * props.glide );
    note->portamento.increment = ( targetFrequency - note->frequency ) / note->portamento.steps;

    return true;
}

float Synthesizer::getArpeggiatorFrequency( int index )
{
    if ( arpeggiatedNotes.size() == 0 ) {
        return 0.f;
    }
    index = std::min( index, ( int ) ( arpeggiatedNotes.size() - 1 ));

    for ( int32 i = index; i < arpeggiatedNotes.size(); ++i ) {
        Note* note = getNoteById( arpeggiatedNotes.at( i ));
        if ( note != nullptr ) {
            return note->baseFrequency;
        }
    }
    return getNoteById( arpeggiatedNotes.at( 0 ))->baseFrequency;
}

int Synthesizer::getArpeggiatorSpeedByTempo( float tempo )
{
    // at what note subdivision should the arpeggios move ?
    // e.g. 16th notes, 32nd notes, 64th notes, etc.
    // we keep the arpeggio in sync with the hosts tempo (see ARPEGGIO_DURATION)
    // but don't want a very slow (or too fast!) moving pulse

    if ( tempo >= 400.f )
        return 4;

    if ( tempo >= 200.f )
        return 8;

    else if ( tempo >= 120.f )
        return 16;

    else if ( tempo >= 50.f )
        return 32;

    else if ( tempo >= 40.f )
        return 64;

    // what kind of ominous, slow chiptune music are you creating??
    return 128;
}

uint16 Synthesizer::generateNextNoteId()
{
    uint16 id = ++note_ids;

    if ( note_ids >= SHRT_MAX ) {
        note_ids = 0;
    }
    return id;
}

bool Synthesizer::isArpeggiatedNote( Note* note )
{
    return std::find( arpeggiatedNotes.begin(), arpeggiatedNotes.end(), note->id ) != arpeggiatedNotes.end();
}

}
