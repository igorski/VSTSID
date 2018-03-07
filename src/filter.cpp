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
#include "filter.h"
#include <math.h>

using namespace Steinberg;

Filter::Filter( float aCutoffFrequency, float aResonance )
{
    _resonance = aResonance;

    // stereo probably enough...
    int numChannels = 8;

    in1  = new float[ numChannels ];
    in2  = new float[ numChannels ];
    out1 = new float[ numChannels ];
    out2 = new float[ numChannels ];

    for ( int i = 0; i < numChannels; ++i )
    {
        in1 [ i ] = 0.0;
        in2 [ i ] = 0.0;
        out1[ i ] = 0.0;
        out2[ i ] = 0.0;
    }
    setCutoff( aCutoffFrequency );
}

Filter::~Filter()
{
    //delete _lfo; // nope... belongs to routeable oscillator in the instrument

    delete[] in1;
    delete[] in2;
    delete[] out1;
    delete[] out2;
}

/* public methods */

void Filter::process( float** sampleBuffer, int amountOfChannels, int bufferSize )
{
    for ( int32 c = 0; c < amountOfChannels; ++c )
    {
        for ( int32 i = 0; i < bufferSize; ++i )
        {
            float input = sampleBuffer[ c ][ i ];
            output      = a1 * input + a2 * in1[ i ] + a3 * in2[ i ] - b1 * out1[ i ] - b2 * out2[ i ];

            in2 [ i ] = in1[ i ];
            in1 [ i ] = input;
            out2[ i ] = out1[ i ];
            out1[ i ] = output;

            // commit the effect
            sampleBuffer[ c ][ i ] = output;
        }
    }
}

void Filter::setCutoff( float frequency )
{
    _cutoff = frequency;

    if ( _cutoff >= sampleRate * .5 )
        _cutoff = sampleRate * .5 - 1;

//    if ( _cutoff < minFreq )
//        _cutoff = minFreq;
//
    calculateParameters();
}

float Filter::getCutoff()
{
    return _cutoff;
}

void Filter::setResonance( float resonance )
{
    _resonance = resonance;
    calculateParameters();
}

float Filter::getResonance()
{
    return _resonance;
}

void Filter::calculateParameters()
{
    c  = 1 / tan( 3.141592653589793f * _cutoff / sampleRate );
    a1 = 1.0 / ( 1.0 + _resonance * c + c * c );
    a2 = 2 * a1;
    a3 = a1;
    b1 = 2.0 * ( 1.0 - c * c ) * a1;
    b2 = ( 1.0 - _resonance * c + c * c ) * a1;
}
