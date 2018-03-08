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
#ifndef __FILTER_H_INCLUDED__
#define __FILTER_H_INCLUDED__

#include "global.h"
#include <math.h>

namespace Igorski {
namespace Filter {

    const float FILTER_MIN_FREQ      = 50.0f;
    const float FILTER_MAX_FREQ      = 12000.f;
    const float FILTER_MIN_RESONANCE = 0.1f;
    const float FILTER_MAX_RESONANCE = sqrt( 2.f ) * .5f;

    extern void init( float aSampleRate );
    extern void destroy();

    extern void  setCutoff( float frequency );
    extern float getCutoff();
    extern void  setResonance( float resonance );
    extern float getResonance();

    extern void calculateParameters();
    extern void process( float** sampleBuffer, int amountOfChannels, int bufferSize );

    extern float _cutoff;
    extern float _resonance;

    // used internally

    extern float _sampleRate;
    extern float _a1;
    extern float _a2;
    extern float _a3;
    extern float _b1;
    extern float _b2;
    extern float _c;

    extern float* _in1;
    extern float* _in2;
    extern float* _out1;
    extern float* _out2;
}
}

#endif
