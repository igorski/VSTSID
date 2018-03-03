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
#include "synthesizer.h"
#include <math.h>

using namespace Steinberg;

namespace Synthesizer {

    int IDs = 0;
    std::vector<Note*> notes;
    bool doAttack, doDecay, doRelease = false;

    float TWO_PI_OVER_SR      = TWO_PI / 44100.f,
          ENVELOPE_INCREMENT  = 0.0f;

    int SAMPLE_RATE = 44100,
        BUFFER_SIZE = 256,
        MAX_ENVELOPE_LENGTH = 22;

    void init( int sampleRate )
    {
        SAMPLE_RATE         = sampleRate;
        TWO_PI_OVER_SR      = TWO_PI / ( float ) SAMPLE_RATE;
        MAX_ENVELOPE_LENGTH = ( int ) round( 1000.f / ( SAMPLE_RATE / 1000.f ));
    }

    void noteOn()
    {
        Note* note = new Note();

        note->id         = ++IDs;
        note->released   = false;
        note->muted      = false;
        note->frequency  = 440.0f, // TODO
        note->phase      = 0.f;
        note->pwm        = 0.f;
        note->arpeggiate = false; // TODO
        note->arpIndex   = 0;
        note->arpeggioLength = 0;

        // TODO: can we just store this inside Synthesizer ?
        note->voiceAmount    = 0;

        // TODO: set ADSR from current model settings

        notes.push_back( note );

        /*
                if ( audioEvent.sid.arpeggiate ) {
                    self.arpeggioLength = Math.round(
                        websid.helpers.BufferHelper.calculateSamplesPerBeat( self.BPM, self.SAMPLE_RATE ) / 16
                    );
                }
                // no note ringing prior to this one ? reset instrument envelopes

                if ( q.length === 0 ) {
                    var adsr  = self.adsr;
                    var props = self.modules.properties;

                    if ( props.envelopes.slope === 0 ) {
                        props.envelopes.slope = websid.helpers.BufferHelper.bufferToMilliseconds( self.BUFFER_SIZE, self.SAMPLE_RATE );
                    }
                    // max envelope length is desired time in seconds (multiplied by 1000 for milliseconds)
                    adsr.maxLength = websid.helpers.BufferHelper.millisecondsToBuffer( 1 * 1000, self.SAMPLE_RATE );
                }

                // set envelopes from the current instrument setting

                audioEvent.sid.adsr = self.adsr.clone();
        */
    }

    void noteOff()
    {
        // TODO : retrieve existing note and set its release phase
        if ( notes.size() == 0 )
            return;

        Note* note = notes.at( 0 );

        // instant removal when release is at 0
        // TODO: take last model value of release ?
        if ( note->adsr.release == 0 ) {
            removeNote( note );
            return;
        };

        // apply release envelope to this event (will be disposed in render loop)

        if ( !note->released ) {
            // TODO: take last model value of release ?
            //note->adsr.release = ( self.adsr.release );
            note->released = true;
        }

        // reset arpeggiator of associated keyboard (event is never removed for
        // arpeggiated sequences as they weren't added in this queue list!)

        //if ( audioEvent.sid.keyboard.arpeggiated ) {
        //    var rm = audioEvent.sid.keyboard.removePitchFromArpeggiatorSequence( audioEvent.sid.frequency );
            //lib.debug.log("freq " + sidEvent.frequency + " removed from arp seq > " + rm);
        //}
    }

    bool removeNote( Note* note )
    {
        bool removed = false;

        if ( std::find( notes.begin(), notes.end(), note ) != notes.end())
        {
            notes.erase( std::find( notes.begin(), notes.end(), note ));
            removed = true;
        }
        return removed;
    }

    Note* getExistingNote()
    {
        // TODO: finish implementation, how do we identify this note
        return nullptr;
    }

    void reset()
    {
        while ( notes.size() > 0 )
            removeNote( notes.at( 0 ));

        IDs = 0;
    }

    void updateADSR( float fAttack, float fDecay, float fSustain, float fRelease )
    {
        doAttack  = fAttack  > 0.0f;
        doDecay   = fDecay   > 0.0f;
        doRelease = fRelease > 0.0f;

        for ( int32 i = 0; i < notes.size(); ++i ) {
            Note* note = notes.at( i );

            // attack
            if ( note->adsr.attack != fAttack ) {
                note->adsr.attack     = fAttack;
                note->adsr.attackStep = 0;

                if ( fAttack > 0 ) {
                    note->adsr.attackLength = ( float ) MAX_ENVELOPE_LENGTH * fAttack;
                }
            }

            // decay

            if ( note->adsr.decay != fDecay ) {
                note->adsr.decay     = fDecay;
                note->adsr.decayStep = 0;
            }

            // sustain

            note->adsr.sustain = fSustain;

            // release

            if ( note->adsr.release != fRelease ) {
                note->adsr.release     = fRelease;
                note->adsr.releaseStep = 0;
            }
        }
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

        int ARPEGGIO_LENGTH_IN_SAMPLES = 2048;
        // TODO
        //.arpeggioLength = Math.round(
        //                websid.helpers.BufferHelper.calculateSamplesPerBeat( self.BPM, self.SAMPLE_RATE ) / 16
        //            );

        int arpeggioLength = ARPEGGIO_LENGTH_IN_SAMPLES;
        int arpIndex       = -1;

        int32 j = notes.size();

        while ( j-- )
        {
            Note* event = notes.at( j );

            // is event muted (e.g. amount of notes above the arpeggio threshold) ?

            if ( event->muted )
                continue;

            float phase     = event->phase;
            bool arpeggiate = event->arpeggiate && event->voiceAmount > 2;

            if ( arpIndex == -1 && arpeggiate )
                arpIndex = event->arpIndex;

            bool disposeEvent = false;

            float attackIncrement = doAttack ? 1.0f / event->adsr.attackLength : 0.0f;
            float releaseAmount   = event->adsr.envelope;

            // TODO: can we cache this upfront ?

            float envelopeLength = ( float ) bufferSize / (( float ) SAMPLE_RATE / 1000.f );
            event->adsr.decayAmount   = envelopeLength * ( event->adsr.decay - event->adsr.sustain );
            event->adsr.releaseAmount = envelopeLength * event->adsr.release;

            // E.O. TODO

            for ( int32 i = 0; i < bufferSize; ++i )
            {
                // update note's frequency when arpeggiating

                if ( arpeggiate && event->arpeggioLength == 0 ) {

                    if ( ++arpIndex == event->voiceAmount )
                        arpIndex = 0;

                    event->frequency      = event->arpFreqs[ arpIndex ];
                    event->arpeggioLength = arpeggioLength;

                    if ( event->voiceAmount > 4 )
                        break;
                }

                // synthesize waveform

                pmv   = i + ( ++event->pwm );
                dpw   = sinf( pmv / ( float ) 0x4800 ) * PWR;
                amp   = phase < PI - dpw ? PW_AMP : -PW_AMP;
                phase = phase + ( TWO_PI_OVER_SR * event->frequency );
                phase = phase > TWO_PI ? phase - TWO_PI : phase;
                //am    = sin( pmv / ( float ) 0x1000 );

                amp *= 4.0f; // make louder !

                if ( event->arpeggioLength > 0 )
                    --event->arpeggioLength;

                // apply envelopes

                envelope = 1.0f;

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

                        event->adsr.envelope  *= ( 1.0f - ( event->adsr.decayStep / event->adsr.decayAmount ));
                        event->adsr.envelope  += event->adsr.sustain;
                        event->adsr.decayStep += ENVELOPE_INCREMENT;

                        if ( event->adsr.envelope > 1.0f )
                            event->adsr.envelope = 1.0f;

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

                if ( arpeggiate )
                    event->arpIndex = arpIndex;
            }
        }
        return true;
    }
}
