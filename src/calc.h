/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2023 Igor Zinken - https://www.igorski.nl
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
#ifndef __CALC_HEADER__
#define __CALC_HEADER__

#include <cmath>
#include <algorithm>
#include "global.h"

/**
 * convenience utilities to process values
 * common to the VST plugin context
 */
namespace Igorski {
namespace Calc {

    /**
     * convert given value in seconds to the appropriate
     * value in samples (for the current sampling rate)
     */
    inline int secondsToBuffer( float seconds )
    {
        return ( int )( seconds * Igorski::VST::SAMPLE_RATE );
    }

    /**
     * convert given value in milliseconds to the appropriate
     * value in samples (for the current sampling rate)
     */
    inline int millisecondsToBuffer( float milliseconds )
    {
        return secondsToBuffer( milliseconds / 1000.f );
    }

} // E.O. namespace Calc
} // E.O. namespace Igorski

#endif
