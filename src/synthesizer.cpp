/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - https://www.igorski.nl
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
#include "synthesizer.h"

using namespace Steinberg;

namespace Synthesizer {

    std::vector<int> notes;

    void noteOn() {
        notes.push_back( 0 );
    }

    void noteOff() {
        notes.clear();
    }

    bool synthesize( float** outputBuffers, int numChannels, int bufferSize, uint32 sampleFramesSize ) {

        // clear existing output buffer contents
        for ( int32 i = 0; i < numChannels; i++ )
            memset( outputBuffers[ i ], 0, sampleFramesSize );

        if ( notes.size() == 0 )
            return false; // nothing to do

        for ( int32 i = 0; i < numChannels; i++ )
        {
            int32 samples = bufferSize;
            float* ptrOut = ( float* ) outputBuffers[ i ];
            while ( --samples >= 0 )
            {
                ( *ptrOut++ ) = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            }
        }
        return true;
    }

}

//namespace Steinberg {
//namespace Vst {
//
////------------------------------------------------------------------------
//template <typename SampleType>
//SampleType VSTSID::processAudio( SampleType** in, SampleType** out, int32 numChannels,
//                                 int32 sampleFrames, float gain )
//{
//    SampleType vuPPM = 0;
//
//    // in real Plug-in it would be better to do dezippering to avoid jump (click) in gain value
//    for (int32 i = 0; i < numChannels; i++)
//    {
//        int32 samples = sampleFrames;
//        SampleType* ptrIn = (SampleType*)in[i];
//        SampleType* ptrOut = (SampleType*)out[i];
//        SampleType tmp;
//        while (--samples >= 0)
//        {
//            // apply gain
//            tmp = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//            (*ptrOut++) = tmp;
//
//            // check only positive values
//            if (tmp > vuPPM)
//            {
//                vuPPM = tmp;
//            }
//        }
//    }
//    return vuPPM;
//}
//
//} // Vst
//} // Igorski
//
//#endif
