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
namespace Igorski {

template <typename SampleType>
bool Synthesizer::synthesize( SampleType** outputBuffers, int numChannels, int bufferSize, uint32 sampleFramesSize )
{
    // clear existing output buffer contents
    for ( int32 i = 0; i < numChannels; i++ ) {
        memset( outputBuffers[ i ], 0, sampleFramesSize );
    }

    if ( notes.size() == 0 ) {
        return false; // nothing to do
    }
    SampleType pmv, dpw, amp, frequency, phase, envelope, tmp;

    int voiceAmount = notes.size();
    int arpIndex    = -1;

    // in case ring modulator is active, synthesize as a triangle

    int waveform = ( ringModulator->getRate() == 0.f ) ? Waveforms::PWM : Waveforms::TRIANGLE;

    // reverse loop as we might splice notes during render
    int32 j = notes.size();
    while ( j-- )
    {
        Note* note = notes.at( j );

        bool doAttack  = note->adsr.attack  > 0.f;
        bool doDecay   = note->adsr.decay   > 0.f && note->adsr.sustain != 1.f;
        bool doRelease = note->adsr.release > 0.f && note->released;

        // is note muted (e.g. is the amount of notes above the arpeggio threshold) ?

        if ( note->muted ) {
            if ( doRelease ) {
                // if note is released, increments it release value, if the resulting
                // envelope is silent, remove the note
                note->adsr.releaseValue += ( note->adsr.releaseIncrement * bufferSize );
                if ( note->adsr.sustain - note->adsr.releaseValue < 0.f ) {
                    removeNote( note );
                }
            }
            continue;
        }

        bool portamento = note->portamento.enabled && note->portamento.steps > 0;
        bool arpeggiate = !portamento && doArpeggiate && isArpeggiatedNote( note );

        if ( arpIndex == -1 && arpeggiate ) {
            arpIndex = note->arpIndex;
        }
        bool disposeNote = false;

        phase = note->phase;

        for ( int32 i = 0; i < bufferSize; ++i )
        {
            // update note's frequency when arpeggiators offset
            // has exceeded the current length

            if ( arpeggiate && note->arpOffset == 0 ) {

                if ( ++arpIndex == voiceAmount ) {
                    arpIndex = 0;
                }
                note->frequency = getArpeggiatorFrequency( arpIndex );
                note->arpOffset = ARPEGGIO_DURATION;

                // WebSID had a maximum voice count, not sure why we'd want that actually =)
//                    if ( voiceAmount > 4 )
//                        break;
            }

            // synthesize waveform

            if ( portamento ) {
                note->frequency += note->portamento.increment;
                if ( --note->portamento.steps == 0 ) {
                    // glide completed, disable portamento for this render iteration
                    portamento = false;
                }
            }
            // apply global pitch bend onto note pitch
            SampleType frequency = note->frequency * props.pitchBend;

            switch ( waveform )
            {
                case Waveforms::TRIANGLE:
                    // 0 == triangle
                    if ( phase < .5f )
                    {
                        tmp = ( phase * 4.f - 1.f );
                        amp = ( 1.f - tmp * tmp );
                    }
                    else {
                        tmp = ( phase * 4.f - 3.f );
                        amp = ( tmp * tmp - 1.f );
                    }
                    // the actual triangulation function
                    amp = amp < 0 ? -amp : amp;

                    phase += ( frequency / ( SampleType ) SAMPLE_RATE );

                    // keep phase within range
                    if ( phase > 1.f )
                        phase -= 1.f;

                    break;

                case Waveforms::PWM:
                    // 1 == PWM
                    pmv   = i + ( ++note->pwm );
                    dpw   = sinf( pmv / ( SampleType ) 0x4800 ) * PWR;
                    amp   = phase < PI - dpw ? PW_AMP : -PW_AMP;
                    phase = phase + ( TWO_PI_OVER_SR * frequency );
                    phase = phase > TWO_PI ? phase - TWO_PI : phase;
                    //am    = sinf( pmv / ( SampleType ) 0x1000 );

                    amp *= 4.f; // make louder !
                    break;
            }

            if ( note->arpOffset > 0 ) {
                --note->arpOffset;
            }

            // apply envelopes

            envelope = 1.f;

            // release cancels all other phases (e.g. early noteOff before other phases have completed)

            if ( doRelease ) {

                envelope = note->adsr.sustain - note->adsr.releaseValue;

                if ( envelope < 0.f ) {
                    envelope = 0.f;
                    disposeNote = true;
                }
                note->adsr.releaseValue += note->adsr.releaseIncrement;
                amp *= envelope;
            }
            else {

                // attack phase
                if ( doAttack && note->adsr.attackValue < note->adsr.attack ) {

                    note->adsr.envelope     = note->adsr.attackValue;
                    note->adsr.attackValue += note->adsr.attackIncrement;

                    amp *= note->adsr.envelope;
                }
                // decay phase
                else if ( doDecay && note->adsr.envelope > note->adsr.sustain ) {

                    note->adsr.envelope -= note->adsr.decayIncrement;
                    amp *= note->adsr.envelope;
                }
                // sustain phase
                else {
                    amp *= note->adsr.sustain;
                }
            }

            // write into output buffers
            // this is (currently?) essentially a mono synth

            for ( int32 c = 0; c < numChannels; ++c ) {
                outputBuffers[ c ][ i ] += ( amp * note->volume );
            }

            // if note can be disposed, break this notes write loop

            if ( disposeNote ) {
                break;
            }
        }

        if ( disposeNote ) {
            // remove note from the list
            removeNote( note );
        }
        else {
            // commit updated properties back into note
            note->phase = phase;

            if ( arpeggiate ) {
                note->arpIndex = arpIndex;
            }
        }
    }

    // ring modulation

    ringModulator->apply( outputBuffers, numChannels, bufferSize, sampleFramesSize );

    return true;
}

} // E.O. namespace Igorski