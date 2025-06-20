/* SoundOutput.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#pragma options(pack_enums)
#include <Sound.h>
#include <SANE.h>
#include <Errors.h>
#pragma options(!pack_enums)

#include "SoundOutput.h"
#include "Memory.h"


#define MinimumInitialNumberOfBuffers (3)

typedef struct MyStructure
	{
		struct MyStructure*		Next;
		struct MyStructure*		Previous;
		SndCommand						MySoundCommand;
		SndCommand						MyCallbackCommand;
		volatile short				InUseFlag; /* interrupt level flag; hence "volatile" */
		ExtSoundHeader				Header;
		char*									SampleArea;
	} MyStructure;


static MyBoolean			SoundSystemInUse = False;

static MyStructure*		NextAvailableBuffer;
static MyBoolean			IsCurrentBufferCheckedOut;

static long						MaxFramesPerBuffer;
static int						BytesPerFrame;
static int						MaxBuffersToAllocate;
static int						CurrentBufferCount;
static long						TheSamplingRate;
static MyBoolean			StereoFlag;
static MyBoolean			SixteenBitFlag;

static SndChannel*		MySoundChannel;


static void							AllocateANewBuffer(void);
static void							DisposeBuffers(void);
static pascal void			MyCallBack(SndChannel* Channel, SndCommand* Command);


/* attempt to obtain a sound channel.  returns True if the sound channel was opened */
/* or False if it couldn't be (if already in use or machine doesn't support sound) */
MyBoolean		OpenSoundChannel(long SamplingRate, SoundOutputStereo WantStereo,
							SoundOutputNumBits NumBits, long FramesPerBuffer, int MaxNumBuffers,
							int InitialNumBuffers)
	{
		OSErr							Error;
		SndCommand				Cmd;
		long							InitOptions;

		/* open the sound channel */
		if (SoundSystemInUse)
			{
				return False;
			}
		InitOptions = initNoInterp /* | initNoDrop */;
		if (WantStereo == eStereo)
			{
				InitOptions |= initStereo;
			}
		MySoundChannel = NIL;
		Error = SndNewChannel(&MySoundChannel,sampledSynth,InitOptions,(void*)&MyCallBack);
		if (Error == noErr)
			{
				/* we want maximum volume on the sound channel */
				Cmd.cmd = ampCmd;
				Cmd.param1 = 255;
				Cmd.param2 = 0;
				Error = SndDoImmediate(MySoundChannel,&Cmd);
				if (Error == noErr)
					{
						/* initialize the variables */
						TheSamplingRate = SamplingRate;
						StereoFlag = (WantStereo == eStereo);
						SixteenBitFlag = (NumBits == e16bit);
						NextAvailableBuffer = NIL;
						MaxFramesPerBuffer = FramesPerBuffer;
						BytesPerFrame = 1;
						if (StereoFlag)
							{
								BytesPerFrame *= 2; /* double the number of words per sample frame */
							}
						if (SixteenBitFlag)
							{
								BytesPerFrame *= (sizeof(short) / sizeof(char)); /* words, not bytes */
							}
						MaxBuffersToAllocate = MaxNumBuffers;
						CurrentBufferCount = 0;
						IsCurrentBufferCheckedOut = False;
						/* allocate the buffers that were requested to begin with */
						if (InitialNumBuffers < MinimumInitialNumberOfBuffers)
							{
								InitialNumBuffers = MinimumInitialNumberOfBuffers;
							}
						while (InitialNumBuffers > 0)
							{
								AllocateANewBuffer();
								InitialNumBuffers -= 1;
							}
						/* notice that we escape here */
						if (CurrentBufferCount > 1)
							{
								/* we need at least 2 buffers, otherwise the sound will skip so */
								/* there'd be no point in doing it with 1 buffer.  Other systems */
								/* may not have this restriction (e.g. UNIX) */
								SoundSystemInUse = True;
								return True;
							}
						DisposeBuffers();
					}
				SndDisposeChannel(MySoundChannel,True);
			}
		return False;
	}


/* internal routine to add a buffer to the existing ring of buffers. */
static void				AllocateANewBuffer(void)
	{
		MyStructure*		Buffer;
		long double			Fred;

		Buffer = (MyStructure*)AllocPtrCanFail(sizeof(MyStructure),"SoundBufHeader");
		if (Buffer != NIL)
			{
				Buffer->SampleArea = AllocPtrCanFail(MaxFramesPerBuffer
					* BytesPerFrame,"SndBuffer");
				if (Buffer->SampleArea == NIL)
					{
						ReleasePtr((char*)Buffer);
						return;
					}
				Buffer->InUseFlag = False;
				Buffer->Header.samplePtr = Buffer->SampleArea;
				if (SixteenBitFlag)
					{
						Buffer->Header.sampleSize = 16;
					}
				 else
					{
						Buffer->Header.sampleSize = 8;
					}
				if (StereoFlag)
					{
						Buffer->Header.numChannels = 2;
					}
				 else
					{
						Buffer->Header.numChannels = 1;
					}
				Buffer->Header.sampleRate = (unsigned long)TheSamplingRate << 16;
				Buffer->Header.loopStart = 0;
				Buffer->Header.loopEnd = 0;
				Buffer->Header.encode = extSH;
				Buffer->Header.baseFrequency = 64;
				Buffer->Header.markerChunk = NULL;
				Buffer->Header.futureUse1 = 0;
				Buffer->Header.futureUse2 = 0;
				Buffer->Header.futureUse3 = 0;
				Buffer->Header.futureUse4 = 0;
				Fred = TheSamplingRate;
				x96tox80(&Fred,&(Buffer->Header.AIFFSampleRate)); /* extended type */
				/* now, link the buffer */
				if (NextAvailableBuffer != NIL)
					{
						/* link this block in */
						Buffer->Next = NextAvailableBuffer;
						Buffer->Previous = NextAvailableBuffer->Previous;
						/* link enclosing buffers to it */
						NextAvailableBuffer->Previous->Next = Buffer;
						NextAvailableBuffer->Previous = Buffer;
						/* stick it where we can use it */
						NextAvailableBuffer = Buffer;
					}
				 else
					{
						Buffer->Next = Buffer;
						Buffer->Previous = Buffer;
						NextAvailableBuffer = Buffer;
					}
				CurrentBufferCount += 1;
			}
	}


/* internal routine to dispose of all of the buffers */
/* don't call this unless they're InUseFlag is clear!!! */
static void				DisposeBuffers(void)
	{
		while (NextAvailableBuffer != NIL)
			{
				MyStructure*		Temp;

				Temp = NextAvailableBuffer;
				if (NextAvailableBuffer->Next == NextAvailableBuffer)
					{
						/* degenerate */
						NextAvailableBuffer = NIL;
					}
				 else
					{
						NextAvailableBuffer->Previous->Next = NextAvailableBuffer->Next;
						NextAvailableBuffer->Next->Previous = NextAvailableBuffer->Previous;
						NextAvailableBuffer = NextAvailableBuffer->Next;
					}
				ReleasePtr(Temp->SampleArea);
				ReleasePtr((char*)Temp);
			}
	}


#if __option(profile)
	#define Profiling (True)
#else
	#define Profiling (False)
#endif

#pragma options(!profile)

/* asynchronous callback routine which marks buffers as now unused */
/* It would be very bad to profile this since profiling tampers with the */
/* stack invocation and uses A5 global variables!  (believe me, I've tried) */
static pascal void    MyCallBack(SndChannel* Channel, SndCommand* Command)
  {
    *(short*)(Command->param2) = 0;
  }

#if Profiling
	#pragma options(profile)
#endif


/* close the sound channel and clean up the buffers */
/* waits until all buffers have played out */
void				CloseSoundChannel(void (*Callback)(void* Refcon), void* Refcon)
	{
		MyStructure*		Scan;

		ERROR(!SoundSystemInUse,PRERR(ForceAbort,
			"CloseSoundChannel called, but sound channel isn't open"));
		SoundSystemInUse = False;
	 LoopPoint:
		Scan = NextAvailableBuffer;
		do
			{
				if (Scan->InUseFlag)
					{
						if (Callback != NIL)
							{
								(*Callback)(Refcon);
							}
						goto LoopPoint;
					}
				Scan = Scan->Next;
			} while (Scan != NextAvailableBuffer /* "Full Circle" */);
		/* dispose of the blasted thing */
		SndDisposeChannel(MySoundChannel,True/*shutupnow*/);
		/* release the memory */
		DisposeBuffers();
	}


/* obtain a pointer to one of the [nonrelocatable] sound buffers.  Data in this */
/* buffer is interpreted as such:  For 8-bit mono, the buffer is an array of */
/* signed chars.  For 16bit mono, the buffer is an array of 2-byte signed integers in */
/* the machine's native endianness.  For 8-bit stereo, the buffer is an array of */
/* 2-byte tuples; the byte lower in memory is the left channel.  For 16-bit stereo, */
/* the buffer is an array of 2-(2-byte) tuples, the left channel is lower in memory. */
/* If there are no buffers currently available (and new ones couldn't be allocated) */
/* then it returns NIL */
char*				CheckOutSoundBuffer(void)
	{
		ERROR(!SoundSystemInUse,PRERR(ForceAbort,
			"CheckOutSoundBuffer called, but sound channel isn't open"));
		ERROR(IsCurrentBufferCheckedOut,PRERR(ForceAbort,
			"CheckOutSoundBuffer called while a buffer has already been checked out"));
		if (NextAvailableBuffer->InUseFlag)
			{
				/* oops, buffer is in use, try to make another */
				if (CurrentBufferCount < MaxBuffersToAllocate)
					{
						AllocateANewBuffer();
					}
				if (NextAvailableBuffer->InUseFlag)
					{
						return NIL;
					}
			}
		IsCurrentBufferCheckedOut = True;
		return NextAvailableBuffer->SampleArea;
	}


/* submit a buffer to be queued to the system's sound channel.  The number of frames */
/* in the buffer actually used is specified to allow less than the full buffer to */
/* be used. */
void				SubmitBuffer(char* Buffer, long NumUsedFrames,
							void (*Callback)(void* Refcon), void* Refcon)
	{
		OSErr			Error;
		long			Scan;

		ERROR(!SoundSystemInUse,PRERR(ForceAbort,
			"SubmitBuffer called, but sound channel isn't open"));
		ERROR(!IsCurrentBufferCheckedOut,PRERR(ForceAbort,
			"SubmitBuffer called but no buffer has been checked out"));
		ERROR(Buffer != NextAvailableBuffer->SampleArea,
			PRERR(ForceAbort,"SubmitBuffer:  Wrong buffer was submitted"));
		ERROR((NumUsedFrames < 0) || (NumUsedFrames > MaxFramesPerBuffer),
			PRERR(ForceAbort,"SubmitBuffer:  Number of used frames exceeds buffer size!"));
		IsCurrentBufferCheckedOut = False;
		ERROR(NextAvailableBuffer->InUseFlag,PRERR(ForceAbort,
			"SubmitBuffer:  Internal error -- InUseFlag is set but shouldn't be"));
		/* adjust the signed samples to be unsigned */
		/* due to Apple's silliness, this only needs to be done for 8-bit samples */
		if (StereoFlag)
			{
				if (!SixteenBitFlag)
					{
						/* stereo, 8-bit */
						for (Scan = (2 * NumUsedFrames) - 1; Scan >= 0; Scan -= 1)
							{
								((char*)Buffer)[Scan] += 0x80;
							}
					}
			}
		 else
			{
				if (!SixteenBitFlag)
					{
						/* mono, 8-bit */
						for (Scan = NumUsedFrames - 1; Scan >= 0; Scan -= 1)
							{
								((char*)Buffer)[Scan] += 0x80;
							}
					}
			}
		/* submit the command to actually play the buffer */
	 TryAgainPoint1:
		NextAvailableBuffer->InUseFlag = 1;
		NextAvailableBuffer->MySoundCommand.cmd = bufferCmd;
		NextAvailableBuffer->MySoundCommand.param1 = 0;
		NextAvailableBuffer->MySoundCommand.param2 = (long)&(NextAvailableBuffer->Header);
		NextAvailableBuffer->Header.numFrames = NumUsedFrames;
		Error = SndDoCommand(MySoundChannel,&(NextAvailableBuffer->MySoundCommand),True);
		if (queueFull == Error)
			{
				/* oops, we filled up the OS queue; wait a little and try again */
				if (Callback != NIL)
					{
						(*Callback)(Refcon);
					}
				goto TryAgainPoint1;
			}
		/* submit the callback routine request.  the callback is an interrupt level */
		/* thing that clears a buffer so we can use it again. */
	 TryAgainPoint2:
		NextAvailableBuffer->MyCallbackCommand.cmd = callBackCmd;
		/* say where to store the "0" */
		NextAvailableBuffer->MyCallbackCommand.param2
			= (long)&(NextAvailableBuffer->InUseFlag);
		Error = SndDoCommand(MySoundChannel,&(NextAvailableBuffer->MyCallbackCommand),True);
		if (queueFull == Error)
			{
				if (Callback != NIL)
					{
						(*Callback)(Refcon);
					}
				goto TryAgainPoint2;
			}
		/* advance to the next buffer */
		NextAvailableBuffer = NextAvailableBuffer->Next;
	}
