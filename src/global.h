#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

namespace Global {
    static const int ID = 0;

    static const char* NAME   = "VSTSID";
    static const char* VENDOR = "igorski.nl";
};

namespace Steinberg {
namespace Vst {

static const FUID VSTSIDProcessorUID (0x84E8DE5F, 0x92554F53, 0x96FAE413, 0x3C935A18);
static const FUID VSTSIDWithSideChainProcessorUID (0x41347FD6, 0xFED64094, 0xAFBB12B7, 0xDBA1D441);
static const FUID VSTSIDControllerUID (0xD39D5B65, 0xD7AF42FA, 0x843F4AC8, 0x41EB04F0);

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

#define MAJOR_VERSION_STR "1"
#define MAJOR_VERSION_INT 1

#define SUB_VERSION_STR "3"
#define SUB_VERSION_INT 3

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR "1" // Build number to be sure that each result could identified.
#define BUILD_NUMBER_INT 1

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename	"vstsid.vst3"
#if PLATFORM_64
#define stringFileDescription	"VSTSID VST3-SDK (64Bit)"
#else
#define stringFileDescription	"VSTSID VST3-SDK"
#endif
#define stringCompanyName		"igorski.nl\0"
#define stringLegalCopyright	"© 2018 igorski.nl"
#define stringLegalTrademarks	"VST is a trademark of Steinberg Media Technologies GmbH"

#endif
