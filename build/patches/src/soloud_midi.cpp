#include <stdlib.h>
#include <stdio.h>

#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#include "tsf.h"
#define TML_IMPLEMENTATION
#define TML_NO_STDIO
#include "tml.h"

#include "soloud_midi.h"
#include "soloud_file.h"

namespace SoLoud
{
	int MidiInstance::tick(float *stream, int SampleCount)
	{
		tml_message *mf = (tml_message *)mTrack;
		tsf *sf = (tsf *)mParent->mSoundFont->mHandle;

		float *begin = stream;
		int SampleBlock = 0;
		for (SampleBlock = 64; SampleCount; SampleCount -= SampleBlock, stream += SampleBlock)
		{
			if (SampleBlock > SampleCount)
				SampleBlock = SampleCount;

			for (mMsec += SampleBlock * (1000.0 / 44100.0); mf && mMsec >= mf->time; mf = mf->next)
			{
				switch (mf->type)
				{
					case TML_PROGRAM_CHANGE:
						tsf_channel_set_presetnumber(sf, mf->channel, mf->program, (mf->channel == 9));
						break;
					case TML_NOTE_ON:
						tsf_channel_note_on(sf, mf->channel, mf->key, mf->velocity / 127.0f);
						break;
					case TML_NOTE_OFF:
						tsf_channel_note_off(sf, mf->channel, mf->key);
						break;
					case TML_PITCH_BEND:
						tsf_channel_set_pitchwheel(sf, mf->channel, mf->pitch_bend);
						break;
					case TML_CONTROL_CHANGE:
						tsf_channel_midi_control(sf, mf->channel, mf->control, mf->control_value);
						break;
				}
			}
			mTrack = mf;
			tsf_render_float(sf, stream, SampleBlock, 0);
		}
		return stream - begin;
	}

	MidiInstance::MidiInstance(Midi *aParent)
	{
		mParent = aParent;
		mParent->mSoundFont->mHandle = tsf_load_memory((const void*)mParent->mSoundFont->mData, mParent->mSoundFont->mDataLen);
		mParent->mHandle = tml_load_memory((const void*)mParent->mData, mParent->mDataLen);

		tsf_channel_set_bank_preset((tsf *)mParent->mSoundFont->mHandle, 9, 128, 0);
		tsf_set_output((tsf *)mParent->mSoundFont->mHandle, TSF_STEREO_UNWEAVED, 44100, 5.0f);

		mTrack = mParent->mHandle;
		mPlaying = mTrack != NULL;
	}

	unsigned int MidiInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int)
	{
		if (mParent->mHandle == NULL || mParent->mSoundFont->mHandle == NULL || mTrack == NULL)
			return 0;

		aSamplesToRead = tick(aBuffer, aSamplesToRead);
		mPlaying = mTrack != NULL;
		if (!mPlaying)
			fprintf(stdout, "ENDED\n");

		return aSamplesToRead;
	}

	void MidiInstance::seek(float aSeconds, float *mScratch, int mScratchSize)
	{
		mMsec = 0;
		double targetSec = aSeconds;
		tml_message *mf = (tml_message *)mParent->mHandle;
		fprintf(stdout, "SEEK TO: %lf\n", aSeconds);
		for (mMsec += 8 * (1000.0 / 44100.0); mf && mMsec < targetSec && mMsec <= mf->time; mf = mf->next)
		{
			;
		}
		mTrack = mf;
	}

	unsigned int MidiInstance::rewind()
	{
		mTrack = (tml_message *)mParent->mHandle;
		mMsec = 0;
		return 0;
	}

	bool MidiInstance::hasEnded()
	{
		return !mPlaying;
	}

	MidiInstance::~MidiInstance()
	{
		if (mParent->mData)
		{
			tml_free((tml_message *)mParent->mData);
		}
		mParent->mData = 0;
	}

	result SoundFont::loadMem(unsigned char *aMem, unsigned int aLength, bool aCopy, bool aTakeOwnership)
	{
		MemoryFile f;
		int res = f.openMem(aMem, aLength, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
			return res;

		return loadFile(&f);
	}

	result SoundFont::load(const char *aFilename)
	{
		DiskFile f;
		int res = f.open(aFilename);
		if (res != SO_NO_ERROR)
			return res;

		return loadFile(&f);
	}

	result SoundFont::loadFile(File *aFile)
	{
		if (mData)
		{
			delete[] mData;
		}

		mDataLen = aFile->length();
		mData = new char[mDataLen];
		if (!mData)
		{
			mData = 0;
			mDataLen = 0;
			return OUT_OF_MEMORY;
		}
		aFile->read((unsigned char*)mData, mDataLen);
/*
		mHandle = tsf_load_memory((const void*)mData, mDataLen);
		if (!mHandle)
		{
			delete[] mData;
			mDataLen = 0;
			return FILE_LOAD_FAILED;
		}
		tsf_channel_set_bank_preset((tsf *)mHandle, 9, 128, 0);
		tsf_set_output((tsf *)mHandle, TSF_STEREO_UNWEAVED, 44100, 5.0f);*/
		return 0;
	}

	SoundFont::SoundFont()
	{
		mData = 0;
		mDataLen = 0;
	}

	SoundFont::~SoundFont()
	{
		tsf_close((tsf *)mHandle);
		delete[] mData;
		mData = 0;
		mDataLen = 0;
	}

	result Midi::loadMem(unsigned char *aMem, unsigned int aLength, SoundFont &sf, bool aCopy, bool aTakeOwnership)
	{
		MemoryFile f;
		int res = f.openMem(aMem, aLength, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
			return res;

		return loadFile(&f, sf);
	}

	result Midi::load(const char *aFilename, SoundFont &sf)
	{
		DiskFile f;
		int res = f.open(aFilename);
		if (res != SO_NO_ERROR)
			return res;

		return loadFile(&f, sf);
	}

	result Midi::loadFile(File *aFile, SoundFont &sf)
	{
		if (mData)
		{
			delete[] mData;
		}

		mDataLen = aFile->length();
		mData = new char[mDataLen];
		if (!mData)
		{
			mData = 0;
			mDataLen = 0;
			return OUT_OF_MEMORY;
		}
		aFile->read((unsigned char*)mData, mDataLen);
/*
		mHandle = tml_load_memory((const void*)mData, mDataLen);
		if (!mHandle)
		{
			delete[] mData;
			mDataLen = 0;
			return FILE_LOAD_FAILED;
		}*/

		mSoundFont = &sf;
		return 0;
	}

	Midi::Midi()
	{
		mBaseSamplerate = 44100;
		mChannels = 2;
		mData = 0;
		mDataLen = 0;
		mSoundFont = 0;
	}

	Midi::~Midi()
	{
		stop();
		delete[] mData;
		mData = 0;
		mDataLen = 0;
		mSoundFont = 0;
	}

	AudioSourceInstance * Midi::createInstance()
	{
		return new MidiInstance(this);
	}

};
