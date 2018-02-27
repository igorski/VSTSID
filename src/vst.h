#ifndef __VST_HEADER__
#define __VST_HEADER__

#include "public.sdk/source/vst2.x/audioeffectx.h"

enum
{
    kGain,
    kNumParams
};

class VST;

class VSTProgram
{
    friend class VST;
    public:
        VSTProgram();
        ~VSTProgram() {}

    private:
        float fGain;
        char name[ 24 ];
};


class VST : public AudioEffectX
{
    public:
        VST (audioMasterCallback audioMaster);
        ~VST ();

        virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);

        virtual void setProgram (VstInt32 program);
        virtual void setProgramName (char* name);
        virtual void getProgramName (char* name);
        virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

        virtual void setParameter (VstInt32 index, float value);
        virtual float getParameter (VstInt32 index);
        virtual void getParameterLabel (VstInt32 index, char* label);
        virtual void getParameterDisplay (VstInt32 index, char* text);
        virtual void getParameterName (VstInt32 index, char* text);

        virtual void resume();

        virtual bool getEffectName (char* name);
        virtual bool getVendorString (char* text);
        virtual bool getProductString (char* text);
        virtual VstInt32 getVendorVersion () { return 1000; }

        virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }

    protected:
        VSTProgram* programs;
        float fGain;
};

#endif
