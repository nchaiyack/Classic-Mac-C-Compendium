/**********
 * Sound Confusion
 *
 *		Copyright © 1992 Bernie Bernstein
 *		9/11/92 - 10/14/92
 *
 *		updated 9/4/93
 ***********/
 

#include	<GestaltEqu.h>
#include	<Memory.h>
#include	<OSEvents.h>
#include	<Sound.h>
#include	<SoundInput.h>
#include	<StdIO.h>
#include	<Types.h>

#include "Confusion.h"

/* how much memory should the program reserve after making the buffers? */
#define kExtraMem		0x5000


/* reset the channels every kResetInterval seconds */
#define kResetInterval	30


/*The HeaderSize constant is used to skip 20 bytes of the buffer when calling bufferCmd.
The first 20 byte of the sound header need to be skipped so that the bufferCmd will be
pointing at a SoundHeader Record and not an 'snd ' resource header. */
#define kHeaderSize		20

typedef struct {
	short		OnOff;
	short	 	Level;
	short	 	Length;
} Level;


Boolean	inStereo;
short gThreshold;
long gBuffSize;
SCGlobals *globs;
BufInfo gExtraBuffer;
Boolean gPlayNRecord;

extern Boolean gQuit;

/***
 * BufPlay
 *		This routine takes an snd buffer and a sound channel and turns the information
 *		into a bufferCmd to the channel.
 ***/
void BufPlay (Handle Buf, SndChannelPtr	channel)
{
	SndCommand		localSndCmd;
	OSErr			err;
	
	localSndCmd.cmd = bufferCmd;
	localSndCmd.param1 = 0;
	localSndCmd.param2 = (long) ((*Buf) + kHeaderSize);
	
	err = SndDoCommand (channel, &localSndCmd, FALSE);
	if (err != noErr)
		AlertUser(err, iPlaying);
	return;
}


/***
 * SetUpSounds
 *		SetUpSounds is a routine which takes a buffer handle, sound headers size and
 *		sample rate value and turns it into a snd buffer with the proper header.
 *		It then returns the buffer handle back to the caller.
 ***/
Handle SetUpSounds (Handle Buf, short *HeaderSize, Fixed sampRate)
{
	OSErr		err;
	short		dummy;
	
	Buf = NewHandle(gBuffSize);
	if ((err = MemError()) != noErr || Buf == nil)
		AlertUser(err, iSetupBuffer);
	HLockHi (Buf);
	if ((err = MemError()) != noErr)
		AlertUser(err, iSetupBuffer);
	
	/* The call to SetupSndHeader is done twice in order to make sure that the 
	Header size is set correctly.  */

	err = SetupSndHeader (Buf, 1, sampRate, 8, 'NONE', 0x3C, 0, HeaderSize);
	if (err != noErr)
		AlertUser(err, iSetupBuffer);
	err = SetupSndHeader (Buf, 1, sampRate, 8, 'NONE', 0x3C, gBuffSize - *HeaderSize, &dummy);
	if (err != noErr)
		AlertUser(err, iSetupBuffer);
		
	return (Buf);
}

/***
 * RndRange
 *		Pick a random number from min to max.
 ***/
short RndRange(short min, short max)
{
	unsigned short r = (unsigned short)Random();
	short d = max-min+1;
	return r / (0x10000/d) + min;
}


/***
 * PickPlayableBuffer
 *		Return the index of a buffer which is ready to be played. It randomly
 *		picks one from the playable buffers, that is, ones which have some
 *		sound in them. If it cant find one, then it returns a value too big.
 ***/
short PickPlayableBuffer(BufInfo *buffers, short max)
{
	short rnd;
	short i;

	for(rnd = RndRange(0,max-1), i=0; 
			!buffers[rnd].playable; 
			i++)
		{
		/* try 100 times to find a good buffer before quitting */
		/* oh shuddup, I know its dumb. */
		if (i>100)
			return max+1;

		rnd = RndRange(0,max-1);
		}
	return rnd;
}


/***
 * PickReadableBuffer
 *		Return the index of a buffer which can read some new sounds
 *		First it fills up each buffer, then it randomly picks new ones
 *		once they are all full.
 ***/
short PickReadableBuffer(BufInfo *buffers, short max)
{
	short rnd;
	short i;

	for(i=0; i<max; i++)
		{
		if (!buffers[i].playable)
			return i;
		}

	return RndRange(0,max-1);
}


#define Abs(x)		((x<0)?-(x):x)
#define BlankSpace	0x1000

/***
 * TrimBuffer
 *		If the buffer is silent, then return FALSE. Otherwise, reduce the
 *		buffer to just the parts that have sound.
 ***/
long TrimBuffer(Handle buffer, long headerlen)
{
	Boolean result = FALSE;
	long oldBuffSize = GetHandleSize(buffer);
	long oldEnd = 0;
	long oldStart = headerlen;
	long newIndex = 0;
	long copySize = 0;
	long bufByte = 0;
	Boolean firstTime = TRUE;
	Handle tmpbuffer = gExtraBuffer.buffer;

	/* copy header into new buffer */
	BlockMove(*buffer, *tmpbuffer, headerlen);

	/* determine if the buffer was loud enough */
	/* loud means that a byte in the buffer is far enough away from the */
	/* center to get above the gThreshold */
	for (bufByte=0; bufByte<oldBuffSize-headerlen; bufByte++)
		{
		short delta = 0x80 - (unsigned char)(*buffer)[headerlen + bufByte];
		if (Abs(delta) << 1 > gThreshold)
			{
			/* the first time we hear something, save the previous BlankSpace bytes */
			/* until now. */
			if (firstTime)
				{
				/* If we are close to the beginning, copy from the beginning. */
				/* otherwise, copy blank space until this spot */
				if (bufByte > BlankSpace)
					oldStart = bufByte - BlankSpace;
				else
					oldStart = 0;
					
				/* If we are near the end, copy to the end. */
				/* otherwise, copy blank space after this spot */
				oldEnd = bufByte + BlankSpace;
				if (headerlen + oldEnd > oldBuffSize)
					oldEnd = oldBuffSize - headerlen;
				copySize = oldEnd - oldStart;

				BlockMove(&(*buffer)[headerlen+oldStart], &(*tmpbuffer)[headerlen+newIndex], copySize);
				newIndex += copySize;
				bufByte = oldEnd;
				firstTime = FALSE;
				}
			else
				{
				/* After the first time, bufByte > BlankSpace. */
				oldStart = bufByte - BlankSpace;
				if (oldStart < oldEnd)
					oldStart = oldEnd+1;

				oldEnd = bufByte + BlankSpace;
				if (headerlen + oldEnd > oldBuffSize)
					oldEnd = oldBuffSize - headerlen;
				copySize = oldEnd - oldStart;

				BlockMove(&(*buffer)[headerlen+oldStart], &(*tmpbuffer)[headerlen+newIndex], copySize);
				newIndex += copySize;
				bufByte = oldEnd;
				}
			result = TRUE;
			}
		}
	
	if (result)
		{
		BlockMove(*tmpbuffer, *buffer, newIndex+headerlen);
		return newIndex;
		}
	else
		{
		return 0L;
		}
}



#define TestBit(l,b)	((l>>b)&0x01)

/***
 * InitSoundConfusion
 *		Allocate memory and initialize the sound driver
 ***/
void InitSoundConfusion(void)
{
	Level				myLevel;
	OSErr				err;
	long				feature;
	BufInfo				*Buffers = nil;
	short				i;
	short				numBuffers;
	short				meterState;
	SPBPtr				RecordRec = nil;
	Str255				settingStr;
	long				tmplong;

	/* use Gestalt to make sure the app will work */
	err = Gestalt(gestaltSoundAttr, &feature);
	if (err == noErr)
		{
		if (!TestBit(feature, gestaltHasSoundInputDevice))
			{
			AlertUser(0, iNoInput);
			ExitToShell();
			}
		else
			{

			struct {
				short ignore;
				char majSys;
				char minSys;
				} sysVersion;

			/* does the machine have stereo? */
			inStereo = TestBit(feature, gestaltStereoCapability);
			
			/* can the machine play and record simultaneously? */
			/* prior to system 7.1, we could assume that if the machine */
			/* had stereo, then it has the better Sound Chip and thus */
			/* can play and record at the same time. 7.1 added the */
			/* gestaltPlayAndRecord bit. */
			err = Gestalt(gestaltSystemVersion, (long*)&sysVersion);
			if (err != noErr)
				{
				AlertUser(err, iGestaltFailed);
				ExitToShell();
				}
			gPlayNRecord = inStereo || TestBit(feature, gestaltPlayAndRecord);
			}
		}
	else
		{
		AlertUser(err, iGestaltFailed);
		ExitToShell();
		}
	
	/* the threshold is a setting in the STR# resource */
	GetIndString(settingStr, rSettingStrings, iThreshold);
	StringToNum(settingStr, &tmplong);
	gThreshold = tmplong;

	/* the buffer size is also kept as a string */
	GetIndString(settingStr, rSettingStrings, iBufferSize);
	StringToNum(settingStr, &tmplong);
	gBuffSize = tmplong;
	
	/* There was a time when I was thinking of making an INIT, so I was */
	/* going to keep all globals in a handle, but I changed my mind */
	/* some of these can just be normal global variables, but what */
	/* the heck. */
	globs = (SCGlobals*)NewPtr(sizeof(SCGlobals));
	if ((err = MemError()) != noErr || globs == nil)
		{
		AlertUser(err, iMakingGlobals);
		return;
		}
	globs->fullBuffer = TRUE;
	globs->loudEnough = FALSE;
	globs->bufferGettingFilled = 0;
	globs->leftChan = nil;
	globs->rightChan = nil;
	globs->buffers = nil;
	globs->numBuffers = 0;
	globs->RecordRec = nil;
	globs->direction = iPlay;		/* next time through, it records */
	
	/* Open sound input drive (whichever one is selected in the sound cdev) */
	err = SPBOpenDevice (nil, siWritePermission, &globs->SoundRefNum);
	if (err != noErr)
		{
		AlertUser(err, iOpeningDevice);
		return;
		}

	/* Get the sample rate information for the snd header */
	err = SPBGetDeviceInfo (globs->SoundRefNum,siSampleRate, (Ptr) &globs->sampleRate);
	if (err != noErr)
		{
		AlertUser(err, iGettingRate);
		return;
		}
	
	/* build the RecordRec pointer and fill in the fields */
	RecordRec = (SPBPtr) NewPtr(sizeof (SPB));
	if ((err = MemError()) != noErr || RecordRec == nil)
		{
		AlertUser(err, iMakingRecordRec);
		return;
		}
	globs->RecordRec = RecordRec;

	/* how many buffers can we make? (leave space for an extra buffer and then some memory) */
	numBuffers = ((FreeMem() - kExtraMem) / gBuffSize) - 1;
	if (numBuffers < 1)
		{
		AlertUser(0, iMemory);
		return;
		}
	
	Buffers = (BufInfo*)NewPtr(sizeof(BufInfo)*numBuffers);
	if ((err = MemError()) != noErr || Buffers == nil)
		{
		AlertUser(err, iAllocatingBuffers);
		return;
		}
	globs->buffers = Buffers;
	globs->numBuffers = numBuffers;
		
	
	/* build the snd buffers */
	for (i=0; i<numBuffers; i++)
		{
		Buffers[i].buffer = SetUpSounds(Buffers[i].buffer, &Buffers[i].headerlength, globs->sampleRate);
		Buffers[i].buffSize = gBuffSize - Buffers[i].headerlength;
		Buffers[i].playable = FALSE;
		}

	gExtraBuffer.buffer = SetUpSounds(gExtraBuffer.buffer, &gExtraBuffer.headerlength, globs->sampleRate);

	RecordRec->inRefNum = globs->SoundRefNum;
	RecordRec->count =  gBuffSize - Buffers[0].headerlength;
	RecordRec->milliseconds = 0;
	RecordRec->bufferLength = gBuffSize - Buffers[0].headerlength;
	RecordRec->bufferPtr = (Ptr) ((*Buffers[0].buffer) + Buffers[0].headerlength);
	RecordRec->completionRoutine = (ProcPtr) &MyRecComp;
	RecordRec->interruptRoutine = nil;
	RecordRec->userLong = (long)globs;
	RecordRec->error = 0;
	RecordRec->unused1 = 0;
	
	globs->firstTime = TRUE;
	
}


/***
 * ResetChannels
 *
 *		Close (if not already) and then open the sound channel(s)
 ***/
void ResetChannels()
{
	SndCommand		mycmd;
	OSErr			err;
	static Boolean channelsExist = FALSE;
	
	if (channelsExist)
		{
		if (globs->leftChan)
			{
			/* kill the left channel (TRUE makes it shut up too) */
			err = SndDisposeChannel (globs->leftChan, TRUE);
			if (err != noErr)
				{
				AlertUser(err, iClosing);
				return;
				}
			}
		if (inStereo && globs->rightChan)
			{
			/* kill the right channel  (TRUE makes it shut up too) */
			err = SndDisposeChannel (globs->rightChan, TRUE);
			if (err != noErr)
				{
				AlertUser(err, iClosing);
				return;
				}
			}
		}

	/* if the machine has stereo, then open the right and left channels */
	if (inStereo)
		{
		/* open the left channel which I can play from */
		globs->leftChan = nil;
		err = SndNewChannel (&globs->leftChan, sampledSynth, initChanLeft, nil);
		if (err != noErr)
			{
			AlertUser(err, iMakingLeft);
			return;
			}
		
		/* open the right channel which I can play from */
		globs->rightChan = nil;
		err = SndNewChannel (&globs->rightChan, sampledSynth, initChanRight, nil);
		if (err != noErr)
			{
			AlertUser(err, iMakingRight);
			return;
			}
		}
	else
		{
		/* open a mono channel and keep in the 'left' variable */
		globs->leftChan = nil;
		err = SndNewChannel (&globs->leftChan, sampledSynth, initMono, nil);
		if (err != noErr)
			{
			AlertUser(err, iMakingMono);
			return;
			}
		}

	channelsExist = TRUE;
}


/***
 * ConfuseSound
 *		This is called during idle time from the event loop.
 *		It starts recording from a buffer and playing from another one.
 ***/
void ConfuseSound(void)
{
	short			dummy;
	OSErr			err;
	BufInfo			*Buffers;
	short			i;
	SndCommand		mycmd;
	long			bufByte;
	short			bufNum;
	Boolean			stillWorking = FALSE;
	Boolean			stillPlaying = FALSE;
	Boolean			stillRecording = FALSE;
	SCStatus		chanStatL;
	SCStatus		chanStatR;
#ifdef IFNEEDTORESETOCCASIONALLY
	static	unsigned long	lastResetTime = 0L;
	unsigned long	currTime;

	GetDateTime(&currTime);
	
	if (currTime - lastResetTime > kResetInterval)
		{
		ResetChannels();
		lastResetTime = currTime;
		if (gQuit)
			return;
		}
#else
	static Boolean	needToReset = TRUE;
	/* only need to reset the first time, to setup the channels, etc */
	if (needToReset)
		{
		ResetChannels();
		needToReset = FALSE;
		if (gQuit)
			return;
		}
#endif

	/* easier access to the buffers */
	Buffers = globs->buffers;

	/* some stuff to prevent us from playing and recording simultaneously */
	/* on a machine that is incapable of it. */
	if (globs->firstTime)
		{
		stillPlaying = FALSE;
		stillRecording = FALSE;
		}
	else 
		{
		if (globs->direction == iPlay || gPlayNRecord)
			{
			err = SndChannelStatus(globs->leftChan, sizeof(SCStatus), &chanStatL);
			if (err != noErr)
				{
				AlertUser(err, iPlaying);
				return;
				}
			stillPlaying = chanStatL.scChannelBusy;

			if (inStereo && !stillPlaying)
				{
				err = SndChannelStatus(globs->rightChan, sizeof(SCStatus), &chanStatR);
				if (err != noErr)
					{
					AlertUser(err, iPlaying);
					return;
					}
				stillPlaying = chanStatR.scChannelBusy;
				}
				
			}
		if (globs->direction == iRecord || gPlayNRecord)
			{
			stillRecording = !globs->fullBuffer;
			}
		}
	
	stillWorking = stillPlaying || stillRecording;
	
	/* if we cant play and record, then exit if we are in the middle of */
	/* playing or recording. we can pass through as soon as the process is done */
	if ((!gPlayNRecord && stillWorking) || (stillPlaying && stillRecording))
		return;


	/* if this is the first time through, then prepare to record into a buffer */
	/* if a buffer recording has finished (ie, the buffer is full), then prepare */
	/* to record into another buffer */
	if (!stillRecording)
		{
		globs->fullBuffer = FALSE;
	
		if (!globs->firstTime && (gPlayNRecord || globs->direction == iRecord))
			{
			long newSize;
			bufNum = globs->bufferGettingFilled;
			
			/* trim the buffer by removing silent (quiet) sections */
			newSize = TrimBuffer(Buffers[bufNum].buffer, Buffers[bufNum].headerlength);
			Buffers[bufNum].playable = globs->loudEnough = (newSize > 0);
			Buffers[bufNum].buffSize = newSize;
	
			/* if the buffer was loud enough, then prepare to record the next readable buffer */
			if (globs->loudEnough)
				{
				short nextbuf = PickReadableBuffer(Buffers, globs->numBuffers);
				globs->bufferGettingFilled = nextbuf;
				globs->loudEnough = FALSE;
				Buffers[nextbuf].playable = FALSE;
				HUnlock(Buffers[nextbuf].buffer);
				SetHandleSize(Buffers[nextbuf].buffer, gBuffSize);
				HLockHi(Buffers[nextbuf].buffer);
				globs->RecordRec->bufferPtr = (*Buffers[nextbuf].buffer) + Buffers[nextbuf].headerlength;
				}
			}
				
		globs->firstTime = FALSE;

		if (gPlayNRecord)
			{
			/* start recording into the buffer asynchronously */
			err = SPBRecord (globs->RecordRec, TRUE); // start recording
			if (err != noErr)
				{
				AlertUser(err, iRecording);
				return;
				}
			}
		}
	
	if (gPlayNRecord && !stillPlaying)
		{
		/* pick a buffer to play from */	
		i = PickPlayableBuffer(Buffers, globs->numBuffers);
		if (i<=globs->numBuffers)
			{
			long bsize = Buffers[i].buffSize;
			err = SetupSndHeader (Buffers[i].buffer, 1, globs->sampleRate, 8, 'NONE', 0x3C, bsize, &dummy);
			if (err != noErr)
				{
				AlertUser(err, iPlaying);
				return;
				}
			/* play it asynchronously */
			BufPlay(Buffers[i].buffer, 
					inStereo ? (RndRange(0,1) ? globs->leftChan : globs->rightChan) : globs->leftChan);
			}
		}
	
	
	/* if we cant play and record simultaneously, then do the one that */
	/* we werent doing last. in other words, take turns playing and recording */
	if (!gPlayNRecord)
		{
		if (globs->direction == iPlay)
			{
			globs->fullBuffer = FALSE;
			/* start recording into the buffer asynchronously */
			err = SPBRecord (globs->RecordRec, TRUE); // start recording
			if (err != noErr)
				{
				AlertUser(err, iRecording);
				return;
				}
			globs->direction = iRecord;
			}
		else /* if (globs->direction == iRecord) */
			{
			/* pick a buffer to play from */	
			i = PickPlayableBuffer(Buffers, globs->numBuffers);
			if (i<=globs->numBuffers)
				{
				long bsize = Buffers[i].buffSize;
				err = SetupSndHeader (Buffers[i].buffer, 1, globs->sampleRate, 8, 'NONE', 0x3C, bsize, &dummy);
				if (err != noErr)
					{
					AlertUser(err, iPlaying);
					return;
					}
				/* play it asynchronously */
				BufPlay(Buffers[i].buffer, 
						inStereo ? (RndRange(0,1) ? globs->leftChan : globs->rightChan) : globs->leftChan);
				}
			globs->direction = iPlay;
			globs->fullBuffer = TRUE;
			}
		}
}


/***
 * StopConfusion
 *		Deallocate memory and close stuff.
 ***/
void StopConfusion(void)
{
	OSErr			err;
	SndCommand		mycmd;
	short			i;

	if (!globs)
		return;

	if (globs->leftChan)
		{
		/* kill the left channel */
		err = SndDisposeChannel (globs->leftChan, TRUE);
		if (err != noErr)
			AlertUser(err, iClosing);
		}
	if (inStereo && globs->rightChan)
		{
		/* kill the right channel */
		err = SndDisposeChannel (globs->rightChan, TRUE);
		if (err != noErr)
			AlertUser(err, iClosing);
		}

	if (globs->SoundRefNum != 0)
		{
		/* stop recording and close the input device */
		err = SPBStopRecording(globs->SoundRefNum);
		if (err != noErr)
			AlertUser(err, iClosing);
		err = SPBCloseDevice (globs->SoundRefNum);
		if (err != noErr)
			AlertUser(err, iClosing);
		}
	
	if (globs->buffers)
		{
		/* kill the snd buffers */
		for (i=0; i<globs->numBuffers; i++)
			{
			if (globs->buffers[i].buffer)
				{
				HUnlock(globs->buffers[i].buffer);
				DisposeHandle(globs->buffers[i].buffer);
				}
			}
		}

	if (gExtraBuffer.buffer)
		{
		HUnlock(gExtraBuffer.buffer);
		DisposeHandle(gExtraBuffer.buffer);
		}

	/* kill the buffers */
	DisposePtr ((Ptr) globs->buffers);
	DisposePtr ((Ptr) globs->RecordRec);
	DisposePtr ((Ptr) globs);
}


/**********************************/

void DoAbout()
{
	Str31 numChannelsStr;
	Str31 maxCPUStr;
	Str31 curCPUStr;
	SMStatus stat;

	OSErr err;
	Str31 numBufsStr;
	Str31 numFilledStr;
	short numFilled = 0;
	short i;
	short itemHit;
	DialogPtr dlg;
	
	err = SndManagerStatus(sizeof(SMStatus), &stat);
	if (err != noErr)
		{
		AlertUser(err, iGettingSMStatus);
		}
	NumToString((long)stat.smNumChannels, numChannelsStr);
	NumToString((long)stat.smMaxCPULoad, maxCPUStr);
	NumToString((long)stat.smCurCPULoad, curCPUStr);
	Pstrcat(curCPUStr, "\p/");
	Pstrcat(curCPUStr, maxCPUStr);
	
	for (i=0; i<globs->numBuffers; i++)
		{
		if (globs->buffers[i].playable)
			numFilled++;
		}
	NumToString((long)globs->numBuffers, numBufsStr);
	NumToString((long)numFilled, numFilledStr);
	ParamText(numBufsStr, numFilledStr, numChannelsStr, curCPUStr);

	dlg = GetNewDialog(rAboutAlert, NULL, (WindowPtr)-1L);
	ModalDialog(NULL, &itemHit);
	DisposDialog(dlg);
}


/***
 * MyRecComp
 *
 * This is the Completion Routine which is called every time the buffer,
 * which is being recorded into, is full.
 ***/
pascal void MyRecComp (SPBPtr inParamPtr)
{
	SCGlobals *glob;
	
	glob = (SCGlobals*)inParamPtr->userLong;
	glob->fullBuffer = TRUE;

	if (inParamPtr->error != noErr && !gQuit)
		{
		AlertUser(inParamPtr->error, iPlaying);
		}

	return;
}


/***
 * Pstrcat
 *
 *		Concat two pascal strings
 ***/
unsigned char *
Pstrcat(
	register Str255			s1,
	register Str255			s2 )
{
	register unsigned short	i;
	unsigned short		l = s2[0];
	unsigned char		*s = s1;
	
	s1 = &s1[s1[0]];
	for( i = 1; i <= l; i++ ) {
		*++s1 = *++s2;
	}
	s[0] += --i;
	
	return( s );
}
