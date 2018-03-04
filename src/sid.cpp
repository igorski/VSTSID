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
#include "sid.h"
#include "miditable.h"
#include <algorithm>
#include <math.h>

using namespace Steinberg;

namespace Igorski {
namespace SID {

    SIDProperties      SID;
    std::vector<Note*> notes;
    bool doArpeggiate  = false;

    double TEMPO = 120.f;

    float TWO_PI_OVER_SR     = TWO_PI / 44100.f,
          ENVELOPE_INCREMENT = 0.f,
          ENVELOPE_LENGTH    = 0.f;

    int SAMPLE_RATE          = 44100,
        BUFFER_SIZE          = 256,
        MAX_ENVELOPE_LENGTH  = 22,
        ARPEGGIO_DURATION    = 16;

    void init( int sampleRate, double tempo )
    {
        SAMPLE_RATE    = sampleRate;
        TWO_PI_OVER_SR = TWO_PI / ( float ) SAMPLE_RATE;

        // max envelope length is desired time in milliseconds
        int envelopeMaxInMillis = 1000.f;
        MAX_ENVELOPE_LENGTH = ( int ) round( envelopeMaxInMillis / ( SAMPLE_RATE / 1000.f ));

        TEMPO = tempo;
    }

    void noteOn( int16 pitch )
    {
        Note* note = new Note();

        bool hadNotes = notes.size() > 0;

        note->pitch          = pitch;
        note->released       = false;
        note->muted          = false;
        note->baseFrequency  = MIDITable::frequencies[ pitch ];
        note->frequency      = note->baseFrequency;
        note->phase          = 0.f;
        note->pwm            = 0.f;
        note->arpIndex       = 0;
        note->arpOffset      = 0;

        notes.push_back( note );
        handleNoteAmountChange();

        if ( doArpeggiate ) {
            int fullMeasure   = round((( float ) SAMPLE_RATE * 60.f ) / TEMPO );
            ARPEGGIO_DURATION = fullMeasure / ARPEGGIATOR_SPEED;
        }

        // no note ringing prior to this one ? reset instrument envelopes

        if ( !hadNotes && ENVELOPE_LENGTH == 0 ) {
            ENVELOPE_LENGTH = ( float ) BUFFER_SIZE / (( float ) SAMPLE_RATE / 1000.f );
        }

        // set Note ADSR properties from current model

        note->adsr.attack     = SID.attack;
        note->adsr.attackStep = 0.f;

        if ( note->adsr.attack > 0.f ) {
            note->adsr.attackLength = ( float ) MAX_ENVELOPE_LENGTH * note->adsr.attack;
        }
        note->adsr.decay     = SID.decay;
        note->adsr.decayStep = 0.f;
        note->adsr.sustain   = SID.sustain;

        // release is only set on noteOff (so the last known release is used if it is updated during Note playback)
    }

    void noteOff( int16 pitch )
    {
        Note* note = getExistingNote( pitch );

        if ( note == nullptr )
            return;

        // instant removal when release is at 0

        if ( SID.release == 0 ) {
            removeNote( note );
            return;
        };

        // apply release envelope to this event (will be disposed in render loop)

        if ( !note->released ) {
            note->adsr.release     = SID.release;
            note->adsr.releaseStep = 0.f;
            note->released         = true;
        }

        // reset arpeggiator of associated keyboard (event is never removed for
        // arpeggiated sequences as they weren't added in this queue list!)

        //if ( doArpeggiate  ) {
        //    var rm = audioEvent.sid.keyboard.removePitchFromArpeggiatorSequence( audioEvent.sid.frequency );
            //lib.debug.log("freq " + sidEvent.frequency + " removed from arp seq > " + rm);
        //}
    }

    Note* getExistingNote( int16 pitch )
    {
        for ( int32 i = 0; i < notes.size(); ++i ) {
            if ( notes.at( i )->pitch == pitch )
                return notes.at( i );
        }
        return nullptr;
    }

    bool removeNote( Note* note )
    {
        bool removed = false;

        if ( std::find( notes.begin(), notes.end(), note ) != notes.end())
        {
            notes.erase( std::find( notes.begin(), notes.end(), note ));
            handleNoteAmountChange();
            removed = true;
        }
        return removed;
    }

    void reset()
    {
        while ( notes.size() > 0 )
            removeNote( notes.at( 0 ));
    }

    void handleNoteAmountChange()
    {
        int amountOfNotes = notes.size();
        doArpeggiate = amountOfNotes >= ARPEGGIATOR_THRESHOLD;

        for ( int32 i = 0; i < amountOfNotes; ++i )
        {
            Note* note = notes.at( i );

            // when arpeggiated, we impose a limit on audible notes as we will use
            // a pitch cycle in the render function to play this notes frequency

            bool audible = !doArpeggiate || ( i < ARPEGGIATOR_THRESHOLD );
            note->muted  = ( i == 0 ) ? false : !audible;
        }
    }

    void updateSIDProperties( float fAttack, float fDecay, float fSustain, float fRelease )
    {
        SID.attack  = fAttack;
        SID.decay   = fDecay;
        SID.sustain = fSustain;
        SID.release = fRelease;
    }

    bool synthesize( float** outputBuffers, int numChannels, int bufferSize, uint32 sampleFramesSize )
    {
        // clear existing output buffer contents
        for ( int32 i = 0; i < numChannels; i++ )
            memset( outputBuffers[ i ], 0, sampleFramesSize );

        if ( notes.size() < 0 )
            return false; // nothing to do

        ENVELOPE_INCREMENT = 1.0f / ( float ) bufferSize;

        float pmv, dpw, amp, phase, envelope;

        int voiceAmount = notes.size();
        int arpIndex    = -1;

        int32 j = notes.size();

        while ( j-- )
        {
            Note* event = notes.at( j );

            // is event muted (e.g. is the amount of notes above the arpeggio threshold) ?

            if ( event->muted )
                continue;

            float phase = event->phase;

            if ( arpIndex == -1 && doArpeggiate )
                arpIndex = event->arpIndex;

            bool disposeEvent = false;

            bool doAttack  = event->adsr.attack  > 0.f;
            bool doDecay   = event->adsr.decay   > 0.f;
            bool doRelease = event->adsr.release > 0.f;

            float attackIncrement = doAttack ? 1.0f / event->adsr.attackLength : 0.0f;
            float releaseAmount   = event->adsr.envelope;

            // TODO: can we cache this upfront ?

            event->adsr.decayAmount   = ENVELOPE_LENGTH * ( event->adsr.decay - event->adsr.sustain );
            event->adsr.releaseAmount = ENVELOPE_LENGTH * event->adsr.release;

            // E.O. TODO

            for ( int32 i = 0; i < bufferSize; ++i )
            {
                // update note's frequency when arpeggiators offset
                // has exceeded the current length

                if ( doArpeggiate && event->arpOffset == 0 ) {

                    if ( ++arpIndex == voiceAmount )
                        arpIndex = 0;

                    event->frequency = getArpeggiatorFrequency( arpIndex );
                    event->arpOffset = ARPEGGIO_DURATION;

                    // WebSID had a maximum, not sure why we'd want that actually =)
//                    if ( voiceAmount > 4 )
//                        break;
                }

                // synthesize waveform

                pmv   = i + ( ++event->pwm );
                dpw   = sinf( pmv / ( float ) 0x4800 ) * PWR;
                amp   = phase < PI - dpw ? PW_AMP : -PW_AMP;
                phase = phase + ( TWO_PI_OVER_SR * event->frequency );
                phase = phase > TWO_PI ? phase - TWO_PI : phase;
                //am    = sin( pmv / ( float ) 0x1000 );

                amp *= 4.f; // make louder !

                if ( event->arpOffset > 0 )
                    --event->arpOffset;

                // apply envelopes

                envelope = 1.f;

                // release

                if ( doRelease && event->released ) {

                    envelope = event->adsr.sustain - ( event->adsr.releaseStep / releaseAmount );

                    if ( envelope < 0.f )
                        envelope = 0.f;

                    if (( event->adsr.releaseStep += ENVELOPE_INCREMENT ) > releaseAmount ) {
                        disposeEvent = true;
                    }
                    amp *= envelope;
                }
                else {

                    // attack
                    if ( doAttack && event->adsr.attackStep < 1.f ) {

                        event->adsr.envelope    = event->adsr.attackStep;
                        event->adsr.attackStep += attackIncrement;

                        amp *= event->adsr.envelope;
                    }
                    // decay
                    else if ( doDecay && event->adsr.decayStep < event->adsr.decayAmount ) {

                        event->adsr.envelope  *= ( 1.f - ( event->adsr.decayStep / event->adsr.decayAmount ));
                        event->adsr.envelope  += event->adsr.sustain;
                        event->adsr.decayStep += ENVELOPE_INCREMENT;

                        if ( event->adsr.envelope > 1.f )
                            event->adsr.envelope = 1.f;

                        amp *= event->adsr.envelope;
                    }
                    // sustain
                    else {
                        amp *= event->adsr.sustain;
                    }
                }

                // write into output buffers
                // this is currently essentially a mono synth

                for ( int32 c = 0; c < numChannels; ++c )
                    outputBuffers[ c ][ i ] += amp;

                // if event can be disposed, halt this loop

                if ( disposeEvent ) {
                    break;
                }
            }

            if ( disposeEvent ) {
                // remove event from the list
                removeNote( event );
            }
            else {
                // commit updated properties back into event
                event->phase = phase;

                if ( doArpeggiate )
                    event->arpIndex = arpIndex;
            }
        }
        return true;
    }

    float getArpeggiatorFrequency( int index )
    {
        if ( notes.size() == 0 )
            return 0.f;

        index = std::min( index, ( int ) ( notes.size() - 1 ));
        return notes.at( index )->baseFrequency;
    }
}
}
