#define CHANGE_VOLUME		0

#include "music layer.h"
#include <Sound.h>

#define gSound3Volume		0x00400040
#define gSoundVolume		2
#define kMusicResID			2000
#define kNumDice			5
#define kMaxForOneOverF		32		/* should be 2^kNumDice */
#define kVolumeMultipler	10
#define kMinVolume			20
#define kLengthMultipler	30
#define kMinLength			30
#define gStartingNote		53

MusicStatusTypes	gMusicStatus;
Boolean				gMusicAvailable;

static	short			gNotes[kMaxForOneOverF];
static	short			gLengths[kMaxForOneOverF];
static	short			gVolumes[kMaxForOneOverF];
static	short			gNoteIndex, gLengthIndex, gVolumeIndex;
static	SndChannelPtr	gMusicChannel;
static	Handle			gMusicHandle=0L;
#if CHANGE_VOLUME
static	long			gOldSound3Volume;
static	short			gOldSoundVolume;
static	Boolean			gHasSoundManager3;
#endif

static	void ComputeOneOverF(short *val);
static	pascal void MusicIsComplete(SndChannelPtr theChannel, SndCommand theCmd);

void InitTheMusic(void)
{
#if CHANGE_VOLUME
	long			gestalt_temp;
	OSErr			isHuman;
	
	isHuman=Gestalt(gestaltSoundAttr, &gestalt_temp);
	gHasSoundManager3=((isHuman==noErr) &&
		((gestalt_temp&(1<<gestaltSndPlayDoubleBuffer)) ||
		 (gestalt_temp&(1<<gestaltMultiChannels)) ||
		 (gestalt_temp&(1<<gestalt16BitAudioSupport))));
	
	if (gHasSoundManager3)
		GetDefaultOutputVolume(&gOldSound3Volume);
	else
		GetSoundVol(&gOldSoundVolume);
#endif
	
	gMusicChannel=0L;
	gMusicAvailable=(SndNewChannel(&gMusicChannel, sampledSynth, initMono, 0L)==noErr);
	if (gMusicAvailable)
		CloseTheMusicChannel();
	gMusicHandle=GetResource('snd ', kMusicResID);
	if (gMusicHandle!=0L)
		HLockHi(gMusicHandle);
	ComputeOneOverF(gNotes);
	ComputeOneOverF(gLengths);
	ComputeOneOverF(gVolumes);
	gNoteIndex=gLengthIndex=gVolumeIndex=0;
}

void StartTheMusic(void)
/* this is oblivious to gMusicStatus -- check it yourself before calling StartTheMusic() */
{
	SndCommand		myCommand;
// hey, this will leak memory if compiled for native PowerMac
	SndCallBackUPP	callbackProc=NewSndCallBackProc(MusicIsComplete);
	long			realA5;
	
	realA5=SetA5(0L);
	SetA5(realA5);
	
#if CHANGE_VOLUME
	if (gHasSoundManager3)
		SetDefaultOutputVolume(gSound3Volume);
	else
		SetSoundVol(gSoundVolume);
#endif
	
	if (gMusicAvailable)
	{
		if (gMusicHandle)
		{
			if (SndNewChannel(&gMusicChannel, sampledSynth, initMono, callbackProc) != noErr)
			{
				gMusicChannel = 0;
				gMusicAvailable = FALSE;
			}
			if (gMusicChannel!=0L)
			{
				gMusicChannel->userInfo = (long)realA5;
				SndPlay(gMusicChannel, (SndListHandle)gMusicHandle, TRUE);
				myCommand.cmd=callBackCmd;
				myCommand.param1=myCommand.param2=0;
				SndDoCommand(gMusicChannel, &myCommand, FALSE);
			}
		}
	}
}

void CloseTheMusicChannel(void)
{
	if (gMusicChannel)
		SndDisposeChannel(gMusicChannel, TRUE);
	gMusicChannel=0L;
#if CHANGE_VOLUME
	if (gHasSoundManager3)
		SetDefaultOutputVolume(gOldSound3Volume);
	else
		SetSoundVol(gOldSoundVolume);
#endif
}

Boolean MusicIsPlayingQQ(void)
{
	return (gMusicChannel!=0L);
}

static	pascal void MusicIsComplete(SndChannelPtr theChannel, SndCommand theCmd)
{
	long			oldA5;
	SndCommand		myCommand;
	short			i;
	long			theVolume;
	
	oldA5=SetA5((long)theChannel->userInfo);
	
	myCommand.cmd=flushCmd;
	myCommand.param1=myCommand.param2=0;
	SndDoImmediate(theChannel, &myCommand);
	
	for (i=0; i<4; i++)
	{
		myCommand.cmd=volumeCmd;
		myCommand.param1=0;
		theVolume=kVolumeMultipler*(gVolumes[gVolumeIndex++]-kNumDice)+kMinVolume;
		myCommand.param2=(theVolume<<16)+theVolume;
		SndDoCommand(theChannel, &myCommand, FALSE);
		
		myCommand.cmd=freqDurationCmd;
		myCommand.param1=kLengthMultipler*(gLengths[gLengthIndex++]-kNumDice)+kMinLength;
		myCommand.param2=gNotes[gNoteIndex++]-kNumDice+gStartingNote;	/* no multiplier; half-steps are consecutive ints */
		SndDoCommand(theChannel, &myCommand, FALSE);
	}
	if (gVolumeIndex>=kMaxForOneOverF)
	{
		ComputeOneOverF(gVolumes);
		gVolumeIndex=0;
	}
	if (gLengthIndex>=kMaxForOneOverF)
	{
		ComputeOneOverF(gLengths);
		gLengthIndex=0;
	}
	if (gNoteIndex>=kMaxForOneOverF)
	{
		ComputeOneOverF(gNotes);
		gNoteIndex=0;
	}
	
	myCommand.cmd=callBackCmd;
	myCommand.param1=myCommand.param2=FALSE;
	SndDoCommand(theChannel, &myCommand, FALSE);
	
	SetA5(oldA5);
}

static	void ComputeOneOverF(short *val)
{
	short			counter;
	short			max;
	short			i;
	short			sum;
	short			dice[kNumDice];
	short			bit[kNumDice];
	
	max=1;
	for (i=0; i<kNumDice; i++)
	{
		bit[i]=max;
		max*=2;
		dice[i]=(Random()&0x7fff)%6+1;
	}
	counter=0;
	while (counter<max)
	{
		sum=0;
		for (i=0; i<kNumDice; i++)
			sum+=dice[i];
		val[counter]=sum;
		counter++;
		for (i=0; i<kNumDice; i++)
			if (counter&bit[i])
				dice[i]=(Random()&0x7fff)%6+1;
	}
}
