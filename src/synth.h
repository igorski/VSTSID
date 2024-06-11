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
#ifndef __SYNTHESIZER_HEADER__
#define __SYNTHESIZER_HEADER__

#include "global.h"
#include "ringmod.h"
#include <vector>

using namespace Steinberg;

namespace Igorski {

    // data type for a single Note

    struct Note {
        uint16 id;      // used internally to reference Notes
        int16 pitch;    // provided by noteOn|Off events from host, used to map to playing Note
        bool released;
        bool muted;
        float volume;
        float baseFrequency; // frequency (in Hz) at noteOn
        float frequency;     // current render frequency (can be shifted by arpeggiator!)
        float phase;
        float pwm;

        // arpeggio specific
        int arpOffset;
        int arpIndex;
        float* arpFreqs;

        struct PORTAMENTO {
            bool enabled;
            int steps;          // amount of samples over which portamento is executed
            float increment;    // pitch increment in Hz (per step)
            std::vector<int16> orgPitches; // history of pitches played before latest note was synthesizd

            PORTAMENTO() {
                enabled   = false;
                steps     = 0;
                increment = 0.f;
            }
        };
        PORTAMENTO portamento;

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

    enum Waveforms
    {
        TRIANGLE,
        PWM
    };

    class Synthesizer {

        public:
            Synthesizer();
            ~Synthesizer();

            double TEMPO; // in BPM, taken from host

            // methods

            void init( int sampleRate, double tempo );

            // create a new Note for a MIDI noteOn/noteOff event
            void noteOn ( int16 pitch, float normalizedVelocity, float tuning );
            void noteOff( int16 pitch );

            void updateProperties(
                 float attack, float decay, float sustain, float release,
                 float ringModRate, float pitchBend, float portamento
            );

            // the whole point of this exercise: synthesizing sweet, sweet PWM !

            template <typename SampleType>
            bool synthesize( SampleType** outputBuffers, int numChannels, int bufferSize, Steinberg::uint32 sampleFramesSize );

            // the amount of simultaneous notes at which arpeggiation begins

            const int ARPEGGIATOR_THRESHOLD = 3;

            // SID model (e.g. synthesizer properties)

            struct SIDProperties {
                float attack;
                float decay;
                float sustain;
                float release;
                float pitchBend; // 1 == no shift, >1 == shift up, <1 == shift down
                float glide;     // 0 == no portamento
            };
            SIDProperties props;

        private:

            Steinberg::Vst::mda::RingModulator* ringModulator;

            // collection of Notes registered for playback

            std::vector<Note*> notes;
            std::vector<int>   arpeggiatedNotes;

            // synthesis related properties

            const float PI     = 3.141592653589793f;
            const float TWO_PI = 2.0f * PI;
            const float PWR    = PI / 1.05f;
            const float PW_AMP = 0.075f;

            float TWO_PI_OVER_SR;
            int SAMPLE_RATE,     // in Hz, taken from host
                BUFFER_SIZE,
                MAX_ENVELOPE_SAMPLES,
                ARPEGGIO_DURATION;

            bool doArpeggiate = false;

            inline bool doGlide() {
                return props.glide > 0.f;
            }

            // retrieves an existing Note for given arguments, if none
            // could be found, nullptr is returned
            Note* getExistingNote( int16 pitch );
            Note* getNoteById( int32 id );

            // removes a Note from the list (used internally when
            // release phase has completed after "noteOff")
            bool removeNote( Note* note );

            // removes all currently playing notes
            void reset();

            // internal update routines used to determine whether the
            // currently playing notes should play back in polyphony
            // or as an arpeggiated sequence

            void handleNoteAmountChange();
            uint16 generateNextNoteId();
            uint16 note_ids;
            bool restorePitchOnRelease( Note* note );

            float getArpeggiatorFrequency( int index );
            bool isArpeggiatedNote( Note* note );

            // get the appropriate arpeggiator speed for given tempo
            int getArpeggiatorSpeedByTempo( float tempo );
    };
}

#include "synth.tcc"

#endif
