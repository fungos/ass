#ifndef SOLOUD_TSF_H
#define SOLOUD_TSF_H

#include "soloud.h"

namespace SoLoud
{
	class Midi;
	class File;

	class MidiInstance : public AudioSourceInstance
	{
		Midi *mParent;
		void *mTrack;
		double mMsec;
		int mPlaying;

		int tick(float *stream, int SampleCount);
	public:
		MidiInstance(Midi *aParent);
		virtual ~MidiInstance();
		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
		virtual void seek(float aSeconds, float *mScratch, int mScratchSize);
		virtual unsigned int rewind();
		virtual bool hasEnded();
	};

	class SoundFont
	{
		friend class MidiInstance;
	protected:
		void *mHandle;
		char *mData;
		unsigned int mDataLen;
	public:
		SoundFont();
		virtual ~SoundFont();
		result load(const char* aFilename);
		result loadMem(unsigned char *aMem, unsigned int aLength, bool aCopy = false, bool aTakeOwnership = true);
		result loadFile(File *aFile);
	};

	class Midi : public AudioSource
	{
		friend class SoundFont;
		friend class MidiInstance;
	protected:
		SoundFont *mSoundFont;
		void *mHandle;
		char *mData;
		unsigned int mDataLen;
	public:
		Midi();
		virtual ~Midi();
		result load(const char* aFilename, SoundFont &sf);
		result loadMem(unsigned char *aMem, unsigned int aLength, SoundFont &sf, bool aCopy = false, bool aTakeOwnership = true);
		result loadFile(File *aFile, SoundFont &sf);
		virtual AudioSourceInstance *createInstance();
	};
};

#endif
