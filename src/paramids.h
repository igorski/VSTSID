/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2023 Igor Zinken - https://www.igorski.nl
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
#ifndef __PARAMIDS_HEADER__
#define __PARAMIDS_HEADER__

enum
{
    // ids for all visual parameters

    kAttackId = 0,     // < ADSR attack time
    kDecayId,          // < ADSR decay time
    kSustainId,        // < ADSR sustain level
    kReleaseId,        // < ADSR release time

    kCutoffId,         // filter cutoff
    kResonanceId,      // filter resonance
    kLFORateId,        // filter LFO rate

    kRingModRateId,    // ring modulator rate

    kLFODepthId,       // filter LFO depth (added in v1.0.1)

    kBypassId,         // bypass process (added in v1.0.3)
    kMasterTuningId,   // pitch bend (added in v1.1.0)
    kPitchBendRangeId, // pitch bend range (added in v1.1.0)
    kPortamentoId,     // portamento (added in v1.1.0)
};

#endif
