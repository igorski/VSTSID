/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __LFO_H_INCLUDED__
#define __LFO_H_INCLUDED__

#include "global.h"

namespace Igorski {
namespace LFO {

    // maximum and minimum rate of oscillation in Hz
    // also see vstsid.xml to update the controls to match

    static const float MAX_LFO_RATE() { return 10.f; }
    static const float MIN_LFO_RATE() { return .1f; }

    // triangle waveform used for the oscillator
    static const float TABLE[ 128 ] = { 0.015625, 0.03125, 0.046875, 0.0625, 0.078125, 0.09375, 0.109375, 0.125, 0.140625, 0.15625, 0.171875, 0.1875, 0.203125, 0.21875, 0.234375, 0.25, 0.265625, 0.28125, 0.296875, 0.3125, 0.328125, 0.34375, 0.359375, 0.375, 0.390625, 0.40625, 0.421875, 0.4375, 0.453125, 0.46875, 0.484375, 0.5, 0.515625, 0.53125, 0.546875, 0.5625, 0.578125, 0.59375, 0.609375, 0.625, 0.640625, 0.65625, 0.671875, 0.6875, 0.703125, 0.71875, 0.734375, 0.75, 0.765625, 0.78125, 0.796875, 0.8125, 0.828125, 0.84375, 0.859375, 0.875, 0.890625, 0.90625, 0.921875, 0.9375, 0.953125, 0.96875, 0.984375, 1, 0.984375, 0.96875, 0.953125, 0.9375, 0.921875, 0.90625, 0.890625, 0.875, 0.859375, 0.84375, 0.828125, 0.8125, 0.796875, 0.78125, 0.765625, 0.75, 0.734375, 0.71875, 0.703125, 0.6875, 0.671875, 0.65625, 0.640625, 0.625, 0.609375, 0.59375, 0.578125, 0.5625, 0.546875, 0.53125, 0.515625, 0.5, 0.484375, 0.46875, 0.453125, 0.4375, 0.421875, 0.40625, 0.390625, 0.375, 0.359375, 0.34375, 0.328125, 0.3125, 0.296875, 0.28125, 0.265625, 0.25, 0.234375, 0.21875, 0.203125, 0.1875, 0.171875, 0.15625, 0.140625, 0.125, 0.109375, 0.09375, 0.078125, 0.0625, 0.046875, 0.03125, 0.015625, 0 };
    static const int TABLE_SIZE     = 128;

    extern void init( float sampleRate );

    extern float getRate();
    extern void setRate( float value );

    // accumulators are used to retrieve a sample from the wave table

    float getAccumulator();
    void setAccumulator( float offset );

    // used internally

    extern float _rate;
    extern float _accumulator;   // is read offset in wave table buffer
    extern float SR_OVER_LENGTH;

    /**
     * retrieve a value from the wave table for the current
     * accumulator position, this method also increments
     * the accumulator and keeps it within bounds
     */
    inline float peek()
    {
        // the wave table offset to read from
        int readOffset = ( _accumulator == 0.f ) ? 0 : ( int ) ( _accumulator / SR_OVER_LENGTH );

        // increment the accumulators read offset
        _accumulator += _rate;

        // keep the accumulator within the bounds of the sample frequency
        if ( _accumulator > Igorski::Vst::SAMPLE_RATE )
            _accumulator -= Igorski::Vst::SAMPLE_RATE;

        // return the sample present at the calculated offset within the table
        return TABLE[ readOffset ];
    }

}
};

#endif
