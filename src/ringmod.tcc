namespace Steinberg {
namespace Vst {
namespace mda {

template <typename SampleType>
void RingModulator::apply( SampleType** outputBuffers, int numChannels,
                           int bufferSize, uint32 sampleFramesSize )
{
    // if ring modulation is off, don't do anything
    if ( _rate == 0.f )
        return;

    int32 sampleFrames = bufferSize;

    SampleType* in1  = outputBuffers[ 0 ];
    SampleType* in2  = outputBuffers[ 1 ];
    SampleType* out1 = outputBuffers[ 0 ];
    SampleType* out2 = outputBuffers[ 1 ];

    SampleType a, b, c, d, g;
    SampleType p, dp, tp = twoPi, fb, fp, fp2;

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

        g = ( SampleType ) sin( p );

        // the SID used a square wave for ring modulation
        // note: as the volume gets a huge boost we
        // normalize the square wave to .5f

        g = ( g >= 0.f ) ? .5f : -.5f;

        // E.O. SID-ifying

        p = ( SampleType ) fmod( p + dp, tp );

        fp  = ( fb * fp + a ) * g;
        fp2 = ( fb * fp + b ) * g;

        c = fp;
        d = fp2;

        *++out1 = static_cast<SampleType>( c );
        *++out2 = static_cast<SampleType>( d );
    }
    fPhi  = p;
    fprev = fp;
}

}}}