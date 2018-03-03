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
#ifndef __SYNTHESIZER_HEADER__
#define __SYNTHESIZER_HEADER__

#include "global.h"
#include <vector>

using namespace Steinberg;

namespace Synthesizer {

    // synthesis related properties

    const float PI      = 3.141592653589793f;
    const float TWO_PI  = 2.0f * PI;
    const float PWR     = PI / 1.05f;
    const float PW_AMP  = 0.075f;

    extern float TWO_PI_OVER_SR, ENVELOPE_INCREMENT;
    extern bool doAttack, doDecay, doRelease;
    extern int SAMPLE_RATE, BUFFER_SIZE, MAX_ENVELOPE_LENGTH;

    // event model (e.g. all currently playing notes)

    // data type for a single Note
    struct Note {
        int16 pitch;
        bool released;
        bool muted;
        float frequency;
        float phase;
        float pwm;

        // arpeggio specific
        bool arpeggiate;
        int arpeggioLength;
        int arpIndex;
        int voiceAmount;
        float* arpFreqs;

        struct ADSR {
            float attack;
            float decay;
            float sustain;
            float release;
            float envelope;
            int maxLength;

            // the "_length"-properties describe the total length (in buffer cycles)
            // the envelope lasts for
            // the "_step"-properties describe the current offset in the total attack
            // envelope slope (relative to total slope length "attack_length")

            float attackLength;
            float attackStep;
            float decayAmount;
            float decayStep;
            float releaseAmount;
            float releaseStep;

            // data type for a Note's ADSR properties
            // ADSR is applied per Note, note for the entirety of the
            // instrument as VSTSID is awesome

            ADSR() {
                attack    = 0.0f;
                decay     = 0.0f;
                sustain   = 1.0f;
                release   = 0.0f;
                envelope  = 1.0f;
                maxLength = 0;

                attackLength  = 0.f;
                attackStep    = 0.f;
                decayAmount   = 0.f;
                decayStep     = 0.f;
                releaseAmount = 0.f;
                releaseStep   = 0.f;
            }
        };
        ADSR adsr;
    };

    // methods

    extern void init( int sampleRate );

    // create a new Note for a MIDI noteOn/noteOff Event
    extern void noteOn ( int16 pitch );
    extern void noteOff( int16 pitch );

    // retrieves an existing Note for given arguments, if none
    // could be found, nullptr is returned
    extern Note* getExistingNote( int16 pitch );

    // removes a Note from the list (used internally when
    // release phase has completed after "noteOff")
    extern bool removeNote( Note* note );

    // removes all currently playing notes
    extern void reset();

    extern void updateADSR( float fAttack, float fDecay, float fSustain, float fRelease );

    // the whole point of this exercise: synthesizing sweet, sweet PWM !
    extern bool synthesize( float** outputBuffers, int numChannels,
        int bufferSize, Steinberg::uint32 sampleFramesSize );

    // collection of Notes registered for playback
    extern std::vector<Note*> notes;
}

#endif
