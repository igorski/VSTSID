/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2024 Igor Zinken - http://www.igorski.nl
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
void Filter::process( SampleType** sampleBuffer, int amountOfChannels, int bufferSize )
{
    float initialLFOOffset = _hasLFO ? _lfo->getAccumulator() : 0.f;
    float orgCutoff        = _tempCutoff;

    for ( int32 c = 0; c < amountOfChannels; ++c )
    {
        // when processing each new channel restore to the same LFO offset to get the same movement ;)
        if ( _hasLFO && c > 0 )
        {
            _lfo->setAccumulator( initialLFOOffset );
            _tempCutoff = orgCutoff;
            calculateParameters();
        }

        for ( int32 i = 0; i < bufferSize; ++i )
        {
            SampleType input  = sampleBuffer[ c ][ i ];
            SampleType output = _a1 * input + _a2 * _in1[ c ] + _a3 * _in2[ c ] - _b1 * _out1[ c ] - _b2 * _out2[ c ];

            _in2 [ c ] = _in1[ c ];
            _in1 [ c ] = input;
            _out2[ c ] = _out1[ c ];
            _out1[ c ] = output;

            // oscillator attached to Filter ? travel the cutoff values
            // between the minimum and maximum frequencies

            if ( _hasLFO )
            {
                // multiply by .5 and add .5 to make bipolar waveform unipolar
                float lfoValue = _lfo->peek() * .5f  + .5f;
                _tempCutoff = fmin( _lfoMax, _lfoMin + _lfoRange * lfoValue );

                calculateParameters();
            }

            // commit the effect
            sampleBuffer[ c ][ i ] = output;
        }
    }
}

} // E.O. namespace Igorski