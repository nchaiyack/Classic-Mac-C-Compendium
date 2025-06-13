#include <Memory.h>
#include <Sound.h>

#define MAXCHANNELS (4)

#define mus_noSynth (0)

struct musdata {
	int numchan; /* number of allocated channels */
	SndChannelPtr chan[MAXCHANNELS]; /* the list of allocated channels */
};

static struct musdata *mdat = NULL;

OSErr musInitialize(void);
void musFinalize(void);
OSErr musAllocChannels(short newnum);
OSErr musPlay(void);
long musAnalyzeSnd(Handle sndHandle, short *retDataType, short *retWaveLength);

static pascal void musCallBack(SndChannelPtr chan, SndCommand cmd);

/* Initialize the sound package. 
	This just clears out the arrays and sets numchan (the number of allocated
	channels) to zero. */
OSErr musInitialize()
{
	int ix;
	OSErr err;
	
	if (mdat) {
		return 1; /* ### already inited */
	}
	
	mdat = (struct musdata *)NewPtr(sizeof(struct musdata));
	if (!mdat) {
		err = MemError();
		return err;
	}
	
	mdat->numchan = 0;
	for (ix=0; ix<MAXCHANNELS; ix++) {
		mdat->chan[ix] = NULL;
	}
	for (ix=0; ix<MAXCHANNELS; ix++) {
		/* originally, I wanted to pre-allocate the four data blocks which
			store the channel structures. It didn't work. So now I just set
			the pointer to NULL and let SndNewChannel() allocate the memory
			for me. */
		/*mdat->chan[ix] = (SndChannelPtr)NewPtrClear(sizeof(SndChannel));
		if (!mdat->chan[ix]) {
			err = MemError();
			for (ix=0; ix<MAXCHANNELS; ix++) {
				if (mdat->chan[ix])
					DisposePtr(mdat->chan[ix]);	
			}
			return err;
		}*/
		mdat->chan[ix] = NULL;
	}
	
	return noErr;
}

/* Shut it all down. 
	reduce the number of allocated channels to zero, and free the memory used by
	the package. */
void musFinalize()
{
	if (!mdat) {
		return; /* already not inited */
	}
	
	musAllocChannels(0);
	
	DisposePtr((Ptr)mdat);
	mdat = NULL;
}

/* Set the number of allocated channels to newnum, by calling SndNewChannel or
	SndDisposeChannel as appropriate. */
OSErr musAllocChannels(short newnum) 
{
	int ix, sofar;
	OSErr err, suberr;
	
	if (newnum < 0 || newnum > MAXCHANNELS) {
		return 1; /* ### bad number */
	}
	
	if (newnum > mdat->numchan) {
		err = noErr;
		for (ix=mdat->numchan; ix<newnum; ix++) {
			/*printf("ix=%d; snc(%ld)\n", ix, mdat->chan[ix]);*/
			err = SndNewChannel(&(mdat->chan[ix]), sampledSynth, (long)0, (SndCallBackUPP)musCallBack);
			/*printf("err = %d\n", err);*/
			if (err) {
				sofar = ix;
				for (ix = sofar-1; ix >= 0; ix--) {
					SndDisposeChannel(mdat->chan[ix], TRUE);
				}
				mdat->numchan = 0;
				return err;
			}
		}
		mdat->numchan = newnum;
		return err;
	}
	else if (newnum < mdat->numchan) {
		err = noErr;
		for (ix = mdat->numchan-1; ix >= newnum; ix--) {
			suberr = SndDisposeChannel(mdat->chan[ix], TRUE);
			if (suberr)
				err = suberr;
		}
		mdat->numchan = newnum;
		return err;
	}
	return noErr;
}

OSErr musPlay()
{
	OSErr err;
	Handle inst1, inst2, inst3, inst4, song1, song2, song3, song4;
	
	/* allocate 4 channels */
	err = musAllocChannels(4);
	if (err) return err;
	
	inst1 = Get1Resource('snd ', 9000);
	inst2 = Get1Resource('snd ', 9001);
	inst3 = Get1Resource('snd ', 9002);
	inst4 = Get1Resource('snd ', 9003);
	if (!inst1 || !inst2 || !inst3 || !inst4)
		return ResError();
		
	{ /* analyze the sound resources and install them in the four channels */
	
		SndCommand cmd;
		long offset;
		short type, len;
		
		offset = musAnalyzeSnd(inst1, &type, &len);
		/* ((*inst)+offset) is the SoundHeader, CmpSoundHeader, or ExtSoundHeader
			(depending on the encode field) */
		if (type != sampledSynth) {
			return 1; /* ### no sound sample in inst */
		}
		if ((((SoundHeaderPtr)((*inst1)+offset))->encode) != stdSH) {
			return 2; /* compressed or extended sound */
		}
		HLock(inst1);
		cmd.cmd = soundCmd;
		cmd.param1 = 0;
		cmd.param2 = (long)((*inst1)+offset);
		err = SndDoImmediate(mdat->chan[0], &cmd);
		
		offset = musAnalyzeSnd(inst2, &type, &len);
		/* ((*inst)+offset) is the SoundHeader, CmpSoundHeader, or ExtSoundHeader
			(depending on the encode field) */
		if (type != sampledSynth) {
			return 1; /* ### no sound sample in inst */
		}
		if ((((SoundHeaderPtr)((*inst3)+offset))->encode) != stdSH) {
			return 2; /* compressed or extended sound */
		}
		HLock(inst2);
		cmd.param2 = (long)((*inst2)+offset);
		err = SndDoImmediate(mdat->chan[1], &cmd);
		offset = musAnalyzeSnd(inst3, &type, &len);
		
		/* ((*inst)+offset) is the SoundHeader, CmpSoundHeader, or ExtSoundHeader
			(depending on the encode field) */
		if (type != sampledSynth) {
			return 1; /* ### no sound sample in inst */
		}
		if ((((SoundHeaderPtr)((*inst3)+offset))->encode) != stdSH) {
			return 2; /* compressed or extended sound */
		}
		HLock(inst3);
		cmd.param2 = (long)((*inst3)+offset);
		err = SndDoImmediate(mdat->chan[2], &cmd);
		offset = musAnalyzeSnd(inst4, &type, &len);
		
		/* ((*inst)+offset) is the SoundHeader, CmpSoundHeader, or ExtSoundHeader
			(depending on the encode field) */
		if (type != sampledSynth) {
			return 1; /* ### no sound sample in inst */
		}
		if ((((SoundHeaderPtr)((*inst4)+offset))->encode) != stdSH) {
			return 2; /* compressed or extended sound */
		}
		HLock(inst4);
		cmd.param2 = (long)((*inst4)+offset);
		err = SndDoImmediate(mdat->chan[3], &cmd);

		song1 = Get1Resource('snd ', 9008);
		song2 = Get1Resource('snd ', 9009);
		song3 = Get1Resource('snd ', 9010);
		song4 = Get1Resource('snd ', 9011);
		if (!song1 || !song2 || !song3 || !song4)
			return ResError();
			
		/* play the four songs in the four channels. This does not even attempt
			to synchronize them, which is why it sounds terrible. */
		HLock(song1);
		HLock(song2);
		HLock(song3);
		HLock(song4);
		SndPlay(mdat->chan[3], (SndListHandle)song4, TRUE);
		SndPlay(mdat->chan[2], (SndListHandle)song3, TRUE);
		SndPlay(mdat->chan[1], (SndListHandle)song2, TRUE);
		SndPlay(mdat->chan[0], (SndListHandle)song1, TRUE);
		HUnlock(song4);
		HUnlock(song3);
		HUnlock(song2);
		HUnlock(song1);
	}

	return err;
}

/* musAnalyzeSnd: gratefully stolen straight from SoundApp.
	Must be called with a valid, non-purged snd handle (may be unlocked, though.) */
/* the following structures are helpful: */
struct Snd1Header {
	short format, numSynths;
};
typedef struct Snd1Header *Snd1HdrPtr;
typedef struct Snd1Header **Snd1HdrHndl;
struct SynthInfo {
	short synthID;
	long initOption;
};
typedef struct SynthInfo *SynthInfoPtr;
struct Snd2Header {
	short format, refCount;
};
typedef struct Snd2Header *Snd2HdrPtr;
typedef struct Snd2Header **Snd2HdrHndl;


long musAnalyzeSnd(Handle sndHandle, short *retDataType, short *retWaveLength)
{
	short dataType, waveLength;
	short synths, howManyCmds;
	long retval;
	Ptr cruisePtr;
	
	retval = 0;
	dataType = mus_noSynth;
	waveLength = 0;
	cruisePtr = *sndHandle;
	if (cruisePtr) {
		 if (((Snd1HdrPtr)cruisePtr)->format == firstSoundFormat) {
			synths = ((Snd1HdrPtr)cruisePtr)->numSynths;
			cruisePtr += sizeof(struct Snd1Header);
			cruisePtr += (sizeof(struct SynthInfo) * synths);
		}
		else {
			cruisePtr += sizeof(struct Snd2Header);
		}
		howManyCmds = *((short *)cruisePtr);
		cruisePtr += sizeof(howManyCmds);

		do {
			switch ((((SndCommand *)cruisePtr)->cmd) & (~dataOffsetFlag)) {

				case soundCmd:
				case bufferCmd:
					dataType = sampledSynth;
					retval = ((SndCommand *)cruisePtr)->param2;
					howManyCmds = 0;
					break;

				case waveTableCmd:
					dataType = waveTableSynth;
					waveLength = ((SndCommand *)cruisePtr)->param1;
					retval = ((SndCommand *)cruisePtr)->param2;
					howManyCmds = 0;
					break;

				default:
					cruisePtr += sizeof(SndCommand);
					howManyCmds -= 1;
					break;

			}
		}
		while (howManyCmds >= 1);
	}
	
	if (retDataType)
		*retDataType = dataType;
	if (retWaveLength)
		*retWaveLength = waveLength;
	return retval;
}

static pascal void musCallBack(SndChannelPtr chan, SndCommand cmd)
{

}
