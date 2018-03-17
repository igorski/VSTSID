/**
 * RingModulator based on mdaRingModulator.h (mda-vst3)
 *
 * Created by Arne Scheffler on 6/14/08.
 *
 * mda VST Plug-ins
 *
 * Copyright (c) 2008 Paul Kellett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ringmod.h"
#include <math.h>

using namespace Igorski;

namespace Steinberg {
namespace Vst {
namespace mda {

/* constructor / destructor */

RingModulator::RingModulator()
{
    _rate     = 0.0f; // Hz
    _fine     = 0.0f; // Hz 0 - 1 range
    _feedback = 0.0f; // %  0 - 1 range

    fPhi  = 0.f;
    twoPi = ( float ) 6.2831853f;
    fprev = 0.f;
    
    recalculate();
}

RingModulator::~RingModulator()
{

}

/* public methods */

void RingModulator::setRate( float ratePercentage )
{
    float rate = SID::MIN_RING_MOD_RATE() + ( ratePercentage * ( SID::MAX_RING_MOD_RATE() - SID::MIN_RING_MOD_RATE() ));

    // 0.0625 is 1 kHz, divide 1000 to get the per-Hz value
    _rate = ( 0.0625f / 1000.f ) * rate;

    recalculate();
}

float RingModulator::getRate()
{
    return _rate;
}

void RingModulator::apply( float** outputBuffers, int numChannels,
                           int bufferSize, uint32 sampleFramesSize )
{
    // if ring modulation is off, don't do anything
    if ( _rate == 0.f )
        return;

    int32 sampleFrames = bufferSize;

    float* in1  = outputBuffers[ 0 ];
    float* in2  = outputBuffers[ 1 ];
    float* out1 = outputBuffers[ 0 ];
    float* out2 = outputBuffers[ 1 ];

    float a, b, c, d, g;
    float p, dp, tp = twoPi, fb, fp, fp2;

    p  = fPhi;
    dp = fdPhi;
    fb = ffb;
    fp = fprev;

    --in1;
    --in2;
    --out1;
    --out2;

    while (--sampleFrames >= 0)
    {
        a = *++in1;
        b = *++in2;

        g = ( float ) sin( p );

        // the SID used a square wave for ring modulation
        // note: as the volume gets a huge boost we
        // normalize the square wave to .5f

        g = ( g >= 0.f ) ? .5f : -.5f;

        // E.O. SID-ifying

        p = ( float ) fmod( p + dp, tp );

        fp  = ( fb * fp + a ) * g;
        fp2 = ( fb * fp + b ) * g;

        c = fp;
        d = fp2;

        *++out1 = c;
        *++out2 = d;
    }
    fPhi  = p;
    fprev = fp;
}

//-----------------------------------------------------------------------------
void RingModulator::recalculate ()
{
    fdPhi = ( float ) ( twoPi * 100.f * ( _fine + ( 160.f * _rate )) / SID::SAMPLE_RATE );
    ffb   = 0.95f * _feedback;
}

}}} // namespaces
