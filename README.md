# ASS: Audio Stupidly Simple

A really very simple audio library composed of only two files (implementation and header) generated from a processed [SoLoud project](https://github.com/jarikomppa/soloud), removing audio synthesis features.

To KISS*, ASS supports only:
- Wav/Mp3/Ogg/Flac formats
- OpenAL/SDL2/ALSA/OSS/WASAP/WINMM/PortAudio backends

Easy to use, no weird build scripts needed, just copy-compile-play!

*Dependencies may be required based on the backend used.

## Sample

To test the sample, copy 3 files into the sample folder:
- sound.wav
- music1.mp3
- music2.ogg

And run:

```
$ cd sample && ./compile.sh && ./sample
```

## License

ass.cpp/ass.h is basically SoLoud, thus it is under SoLoud license terms.
SoLoud, dr_mp3, dr_wav, dr_flac, stb_vorbis are under their own respective licenses.
build/* is under MIT License.
