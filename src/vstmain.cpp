#include "vst.h"

AudioEffect* createEffectInstance( audioMasterCallback audioMaster )
{
    return new VST( audioMaster );
}
