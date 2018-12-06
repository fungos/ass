# ASS: Audio Stupidly Simple

A single header library for audio decoding and playback.

To KISS*, ASS supports only:
- Wav/Mp3/Ogg/Flac formats
- OpenAL/SDL2/ALSA/OSS/WASAPI/WINMM/PortAudio backends

Easy to use and no weird build scripts needed, just copy-compile-play!

*Dependencies may be required based on the backend used.

## Using

The `ass.h` embeds all the necessary decoding dependencies in it, you only need to do:

```
#define ASS_IMPLEMENTATION
#include "ass.h"
```
and ready to go. Otherwise, if you already have dr_libs and stb_vorbis in your project, use the `ass_lite.h` version:

```
#define ASS_IMPLEMENTATION
#include "ass_lite.h"
```

## Sample

To test the sample copy any 3 files into the sample folder, named as:
- sound.wav
- music1.mp3
- music2.ogg

And run:

```
$ cd sample && ./compile.sh && ./sample
```

## License

`ass.h` and `ass_lite.h` are basically [SoLoud](https://github.com/jarikomppa/soloud) minus some features, thus licensed under SoLoud license terms.

[dr_mp3, dr_flac, dr_wav](https://github.com/mackron/dr_libs) and [stb_vorbis](https://github.com/nothings/stb) are under their own respective licenses.

build/* is under MIT License.
