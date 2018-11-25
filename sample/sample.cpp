// g++ sample.cpp -I/usr/include/SDL2 -g -lSDL2 -lasound -ldl -lm -pthread -o sample
// clang++ sample.cpp -I/usr/include/SDL2 -g -lSDL2 -lasound -ldl -lm -pthread -o sample

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

	soloud.init();
	//soloud.setVisualizationEnable(1);

	wav.load("sound.wav");
	wav.setLooping(1);
	int handle1 = soloud.play(wav);
	//soloud.setVolume(handle1, 0.5f);
	//soloud.setPan(handle1, -0.2f);
	//soloud.setRelativePlaySpeed(handle1, 0.9f);

	// Wait for voice to finish
	while (soloud.getVoiceCount() > 1)
	{
		// Still going, sleep for a bit
		SoLoud::Thread::sleep(100);
	}
	fprintf(stdout, "playing sfx.\n");
	mygetch();
	soloud.stop(handle1); // stop the wind sound

	SoLoud::Soloud gSoloud;
	SoLoud::WavStream gMusic1, gMusic2;
	int gMusichandle1, gMusichandle2;

	gMusic1.load("music1.mp3");
	gMusic2.load("msuci2.ogg");

	gMusic1.setLooping(1);
	gMusic2.setLooping(1);

	gSoloud.init(SoLoud::Soloud::CLIP_ROUNDOFF | SoLoud::Soloud::ENABLE_VISUALIZATION);

	gMusichandle1 = gSoloud.play(gMusic1, 1, 0, 1);
	gMusichandle2 = gSoloud.play(gMusic2, 0, 0, 1);

	SoLoud::handle grouphandle = gSoloud.createVoiceGroup();
	gSoloud.addVoiceToGroup(grouphandle, gMusichandle1);
	gSoloud.addVoiceToGroup(grouphandle, gMusichandle2);

	gSoloud.setProtectVoice(grouphandle, 1); // protect all voices in group
	gSoloud.setPause(grouphandle, 0);        // unpause all voices in group
	fprintf(stdout, "playing music 1, press key to fade to music 2.\n");
	mygetch();

	gSoloud.fadeVolume(gMusichandle1, 0, 2);
	gSoloud.fadeVolume(gMusichandle2, 1, 2);
	fprintf(stdout, "playing music 2.\n");
	mygetch();
	gSoloud.destroyVoiceGroup(grouphandle); // remove group, leaves voices alone


	// Clean up SoLoud
	soloud.deinit();

	// All done.
	return 0;
}
