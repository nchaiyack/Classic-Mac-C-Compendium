/************************************************************************************
 * GameSounds.c
 *
 * CheeseToast by Jim Bumgardner
 *
 * Note: When startup sound is played, I'm getting an audio glitch
 * if sound manager 3.0 is installed.
 ************************************************************************************/

#include "CToast.h"
#include <Sound.h>

static Handle			sounds[S_NbrSounds];
static SndChannelPtr	scp;
static SndCommand		tCmd;						/* Sound Command */
static short			savedVolume;

pascal void MyCallBack(SndChannelPtr chan, SndCommand *cmd)
{
	chan->userInfo = 0;
}

void InitSounds()
{
	short	i;
	OSErr	oe;
	for (i = 0; i < S_NbrSounds; ++i) {
		sounds[i] = Get1Resource('snd ',128+i);
		if (sounds[i] != NULL) {
			HNoPurge(sounds[i]);
			MoveHHi(sounds[i]);
			HLock(sounds[i]);
		}
	}
	scp = (SndChannelPtr) NewPtrClear((long) sizeof(SndChannel));
	scp->qLength = stdQLength;
	if ((oe = SndNewChannel(&scp,sampledSynth,initMono,(ProcPtr) MyCallBack)) != noErr)
		DebugStr("\pSound Problem");

	GetSoundVol(&savedVolume);
	SetSoundVol(gPrefs.soundLevel);
}

void EndSounds()
{
	if (scp) {
		SndDisposeChannel(scp,true);
		DisposPtr((Ptr) scp);
		scp = 0L;
	}
	SetSoundVol(savedVolume);
}

void PlaySound(short i, short priority)
{
	if (gPrefs.soundLevel > 0 && i < S_NbrSounds && sounds[i] &&
		priority > scp->userInfo) {
		tCmd.cmd = quietCmd;
		SndDoImmediate(scp,&tCmd);
		tCmd.cmd = flushCmd;
		SndDoImmediate(scp,&tCmd);
		SndPlay(scp, sounds[i], true);
		scp->userInfo = priority;
		tCmd.cmd = callBackCmd;
		SndDoCommand(scp,&tCmd,true);
	}
	if (sounds[S_Startup] != NULL && i != S_Startup) {
		DisposeHandle(sounds[S_Startup]);
		sounds[S_Startup] = NULL;
	}
}

