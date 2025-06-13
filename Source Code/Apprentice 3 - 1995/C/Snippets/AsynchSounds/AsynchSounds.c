/*  =====================================================================
    ===========================AsynchSounds.c============================
    =====================================================================
    
    ©1994 by Thomas Reed
    Written using Metrowerks CodeWarrior 4

    Please do not distribute modified versions of this code.

    If you decide to use this code in a program without making significant
    changes, I don't ask for much -- just mention my name in the About box or
    in a Read Me file (since About box real estate can be valuable!  ;-)
    
	To use, follow these steps:
	
	1)  Change the kSoundErrors and kSoundAlert constants to match the
	    appropriate resources in your project.
	    
	2)	Call InitChan() to open a new channel.
	
	3)	Call PlaySoundsID(myID) to play a sound of ID myID.
	
	4)	Once you've played with the channel all you want, call
		SendCallBack().  This will place a callback command in the queue
		after all your other commands.  When this command is reached,
		IdleSounds() will notice that sound is done and dispose of the
		channel.
	
	5)	Instead of calling PlaySoundsID() with SendCallBack(), you can
		call PlayResFromDisk(soundID, bufferSize).  This will play the
		sound having ID soundID from disk, loading only bufferSize Kbytes
		 at a time.  For large sounds, this uses less memory than
		 PlaySoundsID.  The information in step 5 also applies.
	
	6)	Repeatedly call IdleSounds() if you want the channel disposed as
		soon as the sound is finished.  If you want to kill the sounds at
		a particular time, call KillSounds().  Sound will cease instantly,
		and the channel will be disposed of.  BE SURE THAT THE CHANNEL IS
		DISPOSED IN ONE OF THESE WAYS!!!  If the channel doesn't get
		disposed, it spells big trouble the next time the system, or
		another program, tries to make a noise!
	
	7)	To pause the channel, call TellChanWait(myTime).  A waitCmd will be
		added to the queue and the channel will pause when it reaches
		that command.  myTime is the time to pause in half-milliseconds.
		
	8)	If you use this library, all I ask is some credit in an About box or a
		Read Me file somewhere.  If you have any questions, you can get hold of
		me at reed@visar.wustl.edu.
		
    ===================================================================== */
    
#include "AsynchSounds.h"
#include "Sound.h"

#define NIL			0L

#define kSoundErrors	128
#define kSoundAlert		128

#define errCallBack			1
#define errSoundChannel		2
#define errFlush			3
#define errDisposeSound		4
#define errDisposeChannel	5
#define errBuffer			6
#define errMissingSound		7
#define errGetSound			8
#define errQuiet			9
#define errWait				10
#define errDiskPlay			11

typedef enum {SR_Unused = 1, SR_Inuse, SR_Finished}	 soundRecordState;

typedef struct
{
  soundRecordState	state;
  SndChannelPtr		sound_chan;
} mySoundRecord;

mySoundRecord	gSounds;
Handle			gSoundToDispose = NIL;


pascal void ChanCallBack(SndChannelPtr chan, SndCommand cmd);
pascal void Completion(SndChannelPtr chan);


pascal void ChanCallBack(SndChannelPtr chan, SndCommand cmd)
{
  long	myA5;
  
#ifndef powerc
  myA5 = SetA5(cmd.param2);	/* set A5 to app's A5 */
#endif
  gSounds.state = SR_Finished;
#ifndef powerc
  myA5 = SetA5(myA5);	/* set back to the old A5. */
#endif
}

void SendCallBack(void)
{
  SndCommand	myWish;
  OSErr			oe;
  
  myWish.cmd = callBackCmd;
  myWish.param1 = 0;
  myWish.param2 = SetCurrentA5();
  oe = SndDoCommand(gSounds.sound_chan, &myWish, FALSE);
  if (oe != noErr)
    ErrorAlert(errCallBack);
}

void InitChan(void)
{
  extern		pascal void ChanCallBack(SndChannelPtr chan, SndCommand cmd);
  OSErr			oe;
  SndCommand	myWish;

  gSounds.state = SR_Unused;
  gSounds.sound_chan = NIL;
  oe = SndNewChannel(&gSounds.sound_chan, sampledSynth, initMono, NewSndCallBackProc(ChanCallBack));
  if (oe != noErr)
    ErrorAlert(errSoundChannel);

  myWish.cmd = flushCmd;
  myWish.param1 = 0;
  myWish.param2 = 0;
  oe = SndDoImmediate(gSounds.sound_chan, &myWish);
  if (oe != noErr)
    ErrorAlert(errFlush);
}

void IdleSounds(void)
{
  if (gSounds.state == SR_Finished)
  {
    gSounds.state = SR_Unused;
    if (gSoundToDispose != NIL)
    {
      HUnlock(gSoundToDispose);
      ReleaseResource(gSoundToDispose);
      if (ResError())
        ErrorAlert(errDisposeSound);
    }
    FinishSounds();
  }
}

void FinishSounds(void)
{
  OSErr		oe;

  if (gSounds.sound_chan != NIL)
  {
    oe = SndDisposeChannel(gSounds.sound_chan, FALSE);
    if (oe != noErr)
      ErrorAlert(errDisposeChannel);
  }
}

void PlaySounds (Handle theSound)
{
  OSErr			oe;
  SndCommand	myWish;

  gSoundToDispose = theSound;	/* in case somebody else loaded up */
  								/* theSound rather than calling */
  								/* PlaySoundsID. */

  if ((theSound != NIL) && (*theSound != NIL))
  {
    IdleSounds();
    MoveHHi(theSound);
    HLock(theSound);
    myWish.cmd = bufferCmd;
    myWish.param1 = 0;
    myWish.param2 = (long) (*theSound + 20);
    oe = SndDoCommand(gSounds.sound_chan, &myWish, FALSE);
    if (oe == noErr)
      gSounds.state = SR_Inuse;
    else
    {
      ErrorAlert(errBuffer);
      gSounds.state = SR_Finished;
    }
  }
  else
    ErrorAlert(errMissingSound);
}

void PlaySoundsID (short id)
{
  gSoundToDispose = GetResource('snd ', id);
  if (gSoundToDispose == NIL)
    ErrorAlert(errGetSound); 
  PlaySounds(gSoundToDispose);
}

Boolean SoundsDone(void)
{
  if (gSounds.state == SR_Inuse)
    return (FALSE);
  else
    return (TRUE);
}

void KillSounds(void)
{
  SndCommand	myWish;
  OSErr			err;
  
  if (SoundsDone())
    return;
  
  myWish.cmd = flushCmd;
  myWish.param1 = 0;
  myWish.param2 = 0;
  err = SndDoImmediate(gSounds.sound_chan, &myWish);
  if (err != noErr)
    ErrorAlert(errFlush);
  
  myWish.cmd = quietCmd;   /* params ignored */
  myWish.param1 = 0;
  myWish.param2 = 0;
  err = SndDoImmediate(gSounds.sound_chan, &myWish);
  if (err != noErr)
    ErrorAlert(errQuiet);
  
  gSounds.state = SR_Unused;
  if (gSoundToDispose != NIL)
  {
    HUnlock(gSoundToDispose);
    ReleaseResource(gSoundToDispose);
    if (ResError())
      ErrorAlert(errDisposeSound);
  }
  FinishSounds();
}

void TellChanWait(short halfMilliSecs)
{
  SndCommand	myWish;
  OSErr			oe;
  
  myWish.cmd = waitCmd;
  myWish.param1 = halfMilliSecs;
  myWish.param2 = 0;
  oe = SndDoCommand(gSounds.sound_chan, &myWish, FALSE);
  if (oe != noErr)
    ErrorAlert(errWait);
}

pascal void Completion(SndChannelPtr chan)
{
  long	myA5;
  
#ifndef powerc
  myA5 = SetA5(chan->userInfo);	/* set A5 to app's A5 */
#endif
  gSounds.state = SR_Finished;
#ifndef powerc
  myA5 = SetA5(myA5);	/* set back to the old A5. */
#endif
}

void PlayResFromDisk(short soundID, short bufSize)
{
  OSErr			oe;
  SndCommand	myWish;
  Handle		theSound;
  
  gSounds.sound_chan->userInfo = SetCurrentA5();
  oe = SndStartFilePlay(gSounds.sound_chan, 0, soundID, bufSize*1024, NIL, NIL, Completion, TRUE);
  if (oe == noErr)
    gSounds.state = SR_Inuse;
  else
  {
    ErrorAlert(errDiskPlay);
    gSounds.state = SR_Finished;
  }
}

void ErrorAlert(short errNumber)
{
  short		itemHit;
  Str255	theMessage;
  
  SetCursor(&qd.arrow);
  GetIndString(theMessage, kSoundErrors, errNumber);
  ParamText(theMessage, NIL, NIL, NIL);
  itemHit = StopAlert(kSoundAlert, NIL);
}
