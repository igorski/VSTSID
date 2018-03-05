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

namespace Igorski {
namespace SID {

    // synthesis related properties

    const float PI     = 3.141592653589793f;
    const float TWO_PI = 2.0f * PI;
    const float PWR    = PI / 1.05f;
    const float PW_AMP = 0.075f;
    extern float TWO_PI_OVER_SR;

    extern double TEMPO;        // in BPM, taken from host
    extern int SAMPLE_RATE,     // in Hz, taken from host
               BUFFER_SIZE,
               MAX_ENVELOPE_SAMPLES,
               ARPEGGIO_DURATION;

    extern bool doArpeggiate;

    // the amount of simultaneous notes at which arpeggiation begins

    const int ARPEGGIATOR_THRESHOLD = 3;
    const int ARPEGGIATOR_SPEED     = 16;   // is a subdivision of a full measure, e.g. "16" is 16th note

    // SID model (e.g. synthesizer properties)

    struct SIDProperties {
        float attack;
        float decay;
        float sustain;
        float release;
    };
    extern SIDProperties SID;

    // event model (e.g. all currently playing notes)

    // data type for a single Note
    struct Note {
        int16 pitch;
        bool released;
        bool muted;
        float baseFrequency; // frequency for event's noteOn
        float frequency;     // current render frequency (can be shifted by arpeggiator!)
        float phase;
        float pwm;

        // arpeggio specific
        int arpOffset;
        int arpIndex;
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

            float attackDuration;
            float attackValue;
            float attackIncrement;

            float decayDuration;
            float decayValue;
            float decayIncrement;

            float releaseDuration;
            float releaseValue;
            float releaseIncrement;

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

                attackDuration   = 0.f;
                attackValue      = 0.f;
                attackIncrement  = 0.f;
                decayDuration    = 0.f;
                decayValue       = 0.f;
                decayIncrement   = 0.f;
                releaseDuration  = 0.f;
                releaseValue     = 0.f;
                releaseIncrement = 0.f;
            }
        };
        ADSR adsr;
    };

    // collection of Notes registered for playback

    extern std::vector<Note*> notes;
    extern std::vector<Note*> arpNotes;

    // methods

    extern void init( int sampleRate, double tempo );

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

    // internal update routines
    extern void handleNoteAmountChange();
    extern void updateSIDProperties( float fAttack, float fDecay, float fSustain, float fRelease );

    // the whole point of this exercise: synthesizing sweet, sweet PWM !
    extern bool synthesize( float** outputBuffers, int numChannels,
        int bufferSize, Steinberg::uint32 sampleFramesSize );

    extern float getArpeggiatorFrequency( int index );
}
}

#endif
