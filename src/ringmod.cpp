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
    float rate = VST::MIN_RING_MOD_RATE() + ( ratePercentage * ( VST::MAX_RING_MOD_RATE() - VST::MIN_RING_MOD_RATE() ));

    // 0.0625 is 1 kHz, divide 1000 to get the per-Hz value
    _rate = ( 0.0625f / 1000.f ) * rate;

    recalculate();
}

float RingModulator::getRate()
{
    return _rate;
}

//-----------------------------------------------------------------------------
void RingModulator::recalculate ()
{
    fdPhi = ( float ) ( twoPi * 100.f * ( _fine + ( 160.f * _rate )) / VST::SAMPLE_RATE );
    ffb   = 0.95f * _feedback;
}

}}} // namespaces
