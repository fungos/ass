#if defined(WITH_OPENAL)

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#undef OPENAL
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#undef OPENAL
 // AL.h defines OPENAL without anything, this breaks Backend::OPENAL
#endif

#endif