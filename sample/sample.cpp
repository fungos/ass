// clang++ sample.cpp -DWITH_<BACKEND> -I/usr/include/SDL2 -I/usr/include/AL -g -lSDL2 -lasound -lopenal -ldl -pthread -o sample
//
// On my linux, SoLoud OpenAL and OSS backends are broken, it might be as well broken to you.
//
#ifdef _WIN32
#define WITH_WINMM
#else
#define WITH_SDL
#endif

#define ASS_IMPLEMENTATION
//#include "../ass_lite/ass_lite.h"
#include "../ass.h"

#ifdef _MSC_VER
#include <conio.h>
int mygetch()
{
    return _getch();
}
#else
#include <termios.h>
#include <unistd.h>
int mygetch( )
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

int main(int argc, char *argv[])
{
    SoLoud::Soloud soloud;
    SoLoud::Wav wav;
    SoLoud::WavStream mp3, ogg;
    int mp3Handle, oggHandle;

    soloud.init();
    wav.load("sound.wav");
    wav.setLooping(1);
    int wavHandle = soloud.play(wav);

    while (soloud.getVoiceCount() > 1)
    {
        SoLoud::Thread::sleep(100);
    }
    fprintf(stdout, "playing sfx, press any key to continue...\n");
    mygetch();
    soloud.stop(wavHandle);

    mp3.load("music1.mp3");
    ogg.load("music2.ogg");

    mp3.setLooping(1);
    ogg.setLooping(1);

    mp3Handle = soloud.play(mp3, 1, 0, 1);
    oggHandle = soloud.play(ogg, 0, 0, 1);

    SoLoud::handle groupHandle = soloud.createVoiceGroup();
    soloud.addVoiceToGroup(groupHandle, mp3Handle);
    soloud.addVoiceToGroup(groupHandle, oggHandle);

    soloud.setProtectVoice(groupHandle, 1);
    soloud.setPause(groupHandle, 0);
    fprintf(stdout, "playing mp3, press any key to fade to ogg...\n");
    mygetch();

    soloud.fadeVolume(mp3Handle, 0, 2);
    soloud.fadeVolume(oggHandle, 1, 2);
    fprintf(stdout, "playing ogg, press any key to quit.\n");
    mygetch();
    soloud.destroyVoiceGroup(groupHandle);

    soloud.deinit();
    return 0;
}
