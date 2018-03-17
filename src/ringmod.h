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
#ifndef __RING_MOD_HEADER__
#define __RING_MOD_HEADER__

#include "global.h"

namespace Steinberg {
namespace Vst {
namespace mda {

class RingModulator
{
    public:
        RingModulator();
        ~RingModulator();

        void apply( float** outputBuffers, int numChannels,
                    int bufferSize, uint32 sampleFramesSize );

        // set the rate of the ring modulator to a percentage (0 - 1) range
        // of its maximum range (see global.h)

        void setRate( float ratePercentage );
        float getRate();

    protected:

        void recalculate();

        float _rate,
              _fine,
              _feedback;

        float fPhi;
        float fdPhi;
        float nul;
        float twoPi;
        float ffb, fprev;
};

}}}

#endif
