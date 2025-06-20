#include	"FabTaskManager.h"
#include	"SoundHandling.h"

static SndCallBackUPP	gmyCallbackUPP = nil;
static unsigned short	gNumAsyncSnds = 0;

static pascal void myCallback(SndChannelPtr theChan, const SndCommand *sc);
static void CloseSoundChannel(long sndChan);
static void GetRidOfSndChannel(SndChannelPtr sndChan);

unsigned short GetSoundsRemaining(void)
{
return gNumAsyncSnds;
}

void InitMySoundHandling(void)
{
gmyCallbackUPP = NewSndCallBackProc(myCallback);
}

pascal void myCallback(SndChannelPtr theChan, const SndCommand *sc)
{
PROCCALLEDATINTERRUPTTIME((TMInfoPtr)sc->param2, (long)theChan);
}

void DoSound(short sndResID)
{
SndListHandle	sndH;

if (sndH = (SndListHandle)Get1Resource('snd ', sndResID))
	DoSoundHandle(sndH);
}

void DoSoundHandle(SndListHandle sndH)
{
SndCommand	mySndCmd;
SndChannelPtr	mySndChan;

mySndChan = (SndChannelPtr)NewPtr(sizeof(SndChannel));
if (mySndChan) {
	mySndChan->qLength = 4;
	if (SndNewChannel(&mySndChan, 0, 0, gmyCallbackUPP) == noErr) {
		if (gNumAsyncSnds == 0)
			HLockHi((Handle)sndH);
		mySndChan->userInfo = (long)sndH;
		mySndCmd.cmd = callBackCmd;
		mySndCmd.param2 = (long)NewPtr(sizeof(TMInfo));
		if (mySndCmd.param2) {
			if (SndPlay(mySndChan, sndH, true) == noErr)
				gNumAsyncSnds++;
			InitTaskRecord(CloseSoundChannel, (TMInfoPtr)mySndCmd.param2);
			(void)SndDoCommand(mySndChan, &mySndCmd, true);
			}
		else
			GetRidOfSndChannel(mySndChan);
		}
	}
}

void CloseSoundChannel(long sndChan)
{
if (--gNumAsyncSnds == 0)
	HUnlock((Handle)((SndChannelPtr)sndChan)->userInfo);
GetRidOfSndChannel((SndChannelPtr)sndChan);
}

static void GetRidOfSndChannel(SndChannelPtr sndChan)
{
(void)SndDisposeChannel(sndChan, false);
DisposePtr((Ptr)sndChan);
}

