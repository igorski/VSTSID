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
#include <algorithm>

using namespace Steinberg;

namespace Igorski {
namespace Filter {

    float _sampleRate = 44100.f;
    float _cutoff     = FILTER_MIN_FREQ;
    float _resonance  = FILTER_MIN_RESONANCE;

    float* _in1  = 0;
    float* _in2  = 0;
    float* _out1 = 0;
    float* _out2 = 0;

    float _a1 = 0.f;
    float _a2 = 0.f;
    float _a3 = 0.f;
    float _b1 = 0.f;
    float _b2 = 0.f;
    float _c  = 0.f;

    void init( float aSampleRate )
    {
        _sampleRate = aSampleRate;

        // stereo (2) probably enough...
        int numChannels = 8;

        _in1  = new float[ numChannels ];
        _in2  = new float[ numChannels ];
        _out1 = new float[ numChannels ];
        _out2 = new float[ numChannels ];

        for ( int i = 0; i < numChannels; ++i )
        {
            _in1 [ i ] = 0.f;
            _in2 [ i ] = 0.f;
            _out1[ i ] = 0.f;
            _out2[ i ] = 0.f;
        }
        setCutoff( FILTER_MAX_FREQ / 2 );
    }

    void destroy()
    {
        //delete _lfo; // nope... belongs to routeable oscillator in the instrument

        delete[] _in1;
        delete[] _in2;
        delete[] _out1;
        delete[] _out2;
    }

    /* public methods */

    void updateProperties( float cutoffPercentage, float resonancePercentage )
    {
        float co  = std::max( FILTER_MIN_FREQ,      cutoffPercentage * FILTER_MAX_FREQ );
        float res = std::max( FILTER_MIN_RESONANCE, resonancePercentage * FILTER_MAX_RESONANCE );

        if ( _cutoff != co || _resonance != res ) {
            setCutoff( co );
            setResonance( res );
        }
    }

    void process( float** sampleBuffer, int amountOfChannels, int bufferSize )
    {
        for ( int32 c = 0; c < amountOfChannels; ++c )
        {
            for ( int32 i = 0; i < bufferSize; ++i )
            {
                float input  = sampleBuffer[ c ][ i ];
                float output = _a1 * input + _a2 * _in1[ c ] + _a3 * _in2[ c ] - _b1 * _out1[ c ] - _b2 * _out2[ c ];

                _in2 [ c ] = _in1[ c ];
                _in1 [ c ] = input;
                _out2[ c ] = _out1[ c ];
                _out1[ c ] = output;

                // commit the effect
                sampleBuffer[ c ][ i ] = output;
            }
        }
    }

    void setCutoff( float frequency )
    {
        _cutoff = frequency;

        if ( _cutoff >= _sampleRate * .5 )
            _cutoff = _sampleRate * .5 - 1;

    //    if ( _cutoff < minFreq )
    //        _cutoff = minFreq;
    //
        calculateParameters();
    }

    float getCutoff()
    {
        return _cutoff;
    }

    void setResonance( float resonance )
    {
        _resonance = resonance;
        calculateParameters();
    }

    float getResonance()
    {
        return _resonance;
    }

    void calculateParameters()
    {
        _c  = 1.0 / tan( 3.141592653589793f * _cutoff / _sampleRate );
        _a1 = 1.0 / ( 1.0 + _resonance * _c + _c * _c );
        _a2 = 2 * _a1;
        _a3 = _a1;
        _b1 = 2.0 * ( 1.0 - _c * _c ) * _a1;
        _b2 = ( 1.0 - _resonance * _c + _c * _c ) * _a1;
    }

}
}
