#ifndef __again__
#include "again.h"
#endif

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
    return new AGain(audioMaster);
}

