/* macintosh_audio.c */

/* All the stuff in this file was written by Thomas R. Lawrence. */
/* See the "mac_readme" or "mac_programmer_info" files for more information */
/* about the Macintosh port */

#include <Sound.h>
#include <sane.h>
#include "mac_event.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "extern.h"
#include "song.h"
#include "channel.h"

#define CONSTRAIN(value,min,max) MAX(MIN(value,max),min)

#define MAXNUMBUFFERS (128)
#define MAXBUFFERSIZE (8192)
#define MinFreeMem (32768 + MAXBUFFERSIZE + 256)

/* comment this out to use C-code */
#define USE_ASSEMBLY_CODE (1) /* 1 = yes, 0 = no */

typedef struct MyS
	{
		struct MyS*			Next; /* circular linked list embedded in a static array */
		SndCommand			MySoundCommand;
		SndCommand			MyCallbackCommand;
		volatile short	InUseFlag; /* interrupt level flag; hence "volatile" */
		ExtSoundHeader	Header;
		char						SampleArea[MAXBUFFERSIZE];
	} MyStructure;

static pascal void		MyCallBack(SndChannel* Channel, SndCommand* Command);

extern Boolean				QuitPending;

static int						mac_stereo;
/* 256th of primary/secondary source for that side. */
static int						primary=256, secondary=0;
extern short					Loudness;

/* the following array is grouped as 16*256 2-byte words.	Bits 9..12 of the */
/* index are the most significant bits of the fraction of the pointer for */
/* anti-aliasing.	Bits 1..8 are the sample's value.	If Bit 0 of the address is */
/* 0, then this is the partially weighted left hand side for the antialiasing */
/* average.	Bit 0 == 1 is the right hand side.	The left hand side is */
/* the fraction * sample.	The right hand side is 2s complement of fraction * sample */
static char*					AliasTable = NULL;

/* the following array is a 1024-byte array for converting samples overall loudness */
/* with clipping instead of roll-over.	This is because each raw sample value */
/* ranges from -128..127.	We convert this to unsigned for the table: 0..255 */
/* There are 4 channels, so the maximum is 4*255 == 1020.	Thus, we provide */
/* 1024 (a nice round number) values.	When the table is constructed, the */
/* values are constrained (i.e. clipped) so that rollover doesn't occur.	This */
/* makes the sound a bit better when the volume is high enough to cause */
/* rollover.	The maximum value of abs(FinalVolumeTablePrimary) is "primary" */
/* and the max of abs(FinalVolumeTableSecondary) is "secondary".	See */
/* "resample" for usage notes */
static char*					FinalVolumeTablePrimary = NULL;
static char*					FinalVolumeTableSecondary = NULL;

/* the following array is a 0..64*256 entry array for converting volume */
/* (in the upper bits) and an 8-bit sample value into a new sample value. */
/* an upper value of 64 represents no scaling. */
/* You use this like this: */
/* SubVolumeTable[<8-bit sample> + 256 * <6-bit volume level>] */
static short*				 SubVolumeTable = NULL;

/* the following array is used for workspace */
static long						WorkspaceBytes = 0;
static short*					WorkspaceArray1 = NULL;
static short*					WorkspaceArray2 = NULL;

static SndChannel*		MySoundChannel; /* here's where it all happens, folks */

static MyStructure*		Buffers[MAXNUMBUFFERS];
static int						ActualNumBuffers;
static MyStructure*		CurrentBuffer;
static int						buf_index;

short									Pausing = false;

/* this is dragged in from "mac_event" */
extern short					NumBits; /* what they want */
static int						ActualNumBits; /* what we're actually using */

/* the following defines have also been lifted from "audio.c" */
extern int allocated;
#define MAX_CHANNELS 8
#define DO_NOTHING 0
#define PLAY 1
#define REPLAY 2
extern struct audio_channel
   {
   struct sample_info *samp;
   int mode;
   unsigned long pointer;
   unsigned long step;
   int volume;
   int pitch;
   } chan[MAX_CHANNELS];

/* this is called from the event cycle which is called from the synthesizer via */
/* the invocation of "may_getchar()".	When the program is playing */
/* (Pausing == false), it sets Pausing to true, pauses the Macintosh sound */
/* channel, and then spinwaits, calling may_getchar to provide processing time */
/* to other programs.	When the event to unpause is received, this program is */
/* invoked again (the first activation record is still present--recursive */
/* invocation).	The second call unpauses the sound channel and clears the flag. */
/* As the call stack is unrolled, control will be returned here.	Since the */
/* Pausing flag is now clear, this routine will exit and things will carry */
/* on normaly.	FLAW:	pauseCmd does not seem to have an effect.	The buffered */
/* sound will play out and sound will stop only when all prepared buffers */
/* have been played.	I don't know how to fix this; it seems to be a problem in */
/* the Macintosh OS, not in this program. */
void				TogglePause(void)
	{
		SndCommand			MyCommand;

		if (Pausing)
			{
				Pausing = false;
				MyCommand.cmd = resumeCmd;
				MyCommand.param1 = 0;
				MyCommand.param2 = 0;
				SndDoImmediate(MySoundChannel,&MyCommand);
			}
		 else
			{
				Pausing = true;
				MyCommand.cmd = pauseCmd;
				MyCommand.param1 = 0;
				MyCommand.param2 = 0;
				SndDoImmediate(MySoundChannel,&MyCommand);
				while (Pausing && !QuitPending)
					{
						may_getchar();
					}
			}
	}


/* adjust the stereo mixing.	Percent is 0..100; each channel takes on a value */
/* in 0..256, providing 8-bits of scaling.	At full volume for a channel, */
/* 256 represents a shift left by 8.	Thus, a shift right by 8 is the necessary */
/* correction after scaling.	"primary" is for the main channel and "secondary" */
/* is for the other channel.	They do not correspond strictly to left and right. */
void			set_mix(int percent)
	{
		percent *= (256/2);
		percent /= 100;
		secondary = percent;
		primary = 256 - percent;
	}


/* recalibrate the volume lookup table using a new value.	This is done on */
/* the fly as volume levels are adjusted. */
void			ResetVolumeTable(void)
	{
		short				Scan;
		short				Intermediate;
		short				Radius;

		if (FinalVolumeTablePrimary == NULL)
			{
				FinalVolumeTablePrimary = (char*)NewPtr(1024 * sizeof(char));
				if (FinalVolumeTablePrimary == NULL)
					{
						perror("Not enough memory to play song.");
						end_all("");
					}
			}
		/* primary ranges between 0..255; abs(sample) ranges between 0..127. */
		/* so we divide by 2 and subtract 1 just in case -128 gets in.	The radius */
		/* is the maximum value we'll allow the sample to be.	Anything above */
		/* is clipped.	Primary Radius + Secondary Radius == 127 (barring rounding */
		/* errors.)	Thus we clip just before arithmetic rollover occurs. */
		Radius = (primary / 2) - 1;
		if (Radius < 0)
			{
				Radius = 0;
			}
		for (Scan = -512; Scan <= 511; Scan += 1)
			{
				Intermediate = (Scan * Loudness * primary) / (64 * 256);
				if (Intermediate > Radius)
					{
						Intermediate = Radius;
					}
				if (Intermediate < -Radius)
					{
						Intermediate = -Radius;
					}
				FinalVolumeTablePrimary[Scan & 0x03ff] = Intermediate;
			}

		if (FinalVolumeTableSecondary == NULL)
			{
				FinalVolumeTableSecondary = (char*)NewPtr(1024 * sizeof(char));
				if (FinalVolumeTableSecondary == NULL)
					{
						perror("Not enough memory to play song.");
						end_all("");
					}
			}
		Radius = (secondary / 2) - 1;
		if (Radius < 0)
			{
				Radius = 0;
			}
		for (Scan = -512; Scan <= 511; Scan += 1)
			{
				Intermediate = (Scan * Loudness * secondary) / (64 * 256);
				if (Intermediate > Radius)
					{
						Intermediate = Radius;
					}
				if (Intermediate < -Radius)
					{
						Intermediate = -Radius;
					}
				FinalVolumeTableSecondary[Scan & 0x03ff] = Intermediate;
			}
	}


int				open_audio(int SampleRate, int StereoFlag)
	{
		OSErr							Error;
		short							Scan;
		short							Index;
		SndCommand				Cmd;
		long double				Fred;
		unsigned long			FixedSampleRate;
		MyStructure*			LastBuffer;

		if ((NumBits != 8) && (NumBits != 16))
			{
				NumBits = 8;
			}
		ActualNumBits = NumBits;
		FixedSampleRate = ((long)SampleRate) << 16;
		if (FixedSampleRate == 0)
			{
				FixedSampleRate = rate22khz;
			}

		if (AliasTable == NULL)
			{
				AliasTable = (char*)NewPtr(16*256*2);
				if (AliasTable == NULL)
					{
						perror("Not enough memory to play song.");
						end_all("");
					}
				for (Scan = 0; Scan <= 15; Scan += 1)
					{
						for (Index = -128; Index <= 127; Index += 1)
							{
								short				TableIndex;

								/* we precompute the antialiasing multiplication table here. */
								/* Scan == the high 4 bits of the fraction of the pointer which */
								/* is accessing the sampled sound.	Thus 0 IS a sample point, */
								/* 1 is just to the right of a sample point, and 15 is just to */
								/* the left of a sample point.	As we approach the right, the */
								/* value increases, so the right weight is just Scan.	As we */
								/* approach the left, the weight increases, but Scan decreases */
								/* so the value is 16-Scan.	When Scan == 0, the weight of the */
								/* right is 0, and the full left value is used.	This is because */
								/* at this point, the left sample is being pointed directly to */
								/* so it's actual value should be returned. */
								TableIndex = ((Scan << 8) | (Index & 0x00ff)) << 1;
								AliasTable[TableIndex + 0] = ((16 - Scan) * Index) / 16;
								AliasTable[TableIndex + 1] = (Scan * Index) / 16;
							}
					}
			}

		ResetVolumeTable();

		if (SubVolumeTable == NULL)
			{
				SubVolumeTable = (short*)NewPtr((MAX_VOLUME-MIN_VOLUME+1)*256*sizeof(short));
				if (SubVolumeTable == NULL)
					{
						perror("Not enough memory to play song.");
						end_all("");
					}
				for (Scan = 0; Scan <= MAX_VOLUME - MIN_VOLUME; Scan += 1)
					{
						for (Index = -128; Index <= 127; Index += 1)
							{
								SubVolumeTable[(Scan << 8) | (Index & 0x00ff)]
									= ((Scan + MIN_VOLUME) * Index + ((MAX_VOLUME - MIN_VOLUME)/2))
									/ (MAX_VOLUME - MIN_VOLUME);
								/* normally, that would be MAX_VOLUME - MIN_VOLUME + 1, but */
								/* we want to be 0..1 instead of 0..0.99999 because MAX_VOLUME */
								/* should not scale the volume at all. */
							}
					}
			}

		mac_stereo = StereoFlag;

		Error = SndNewChannel(&MySoundChannel,
			sampledSynth, /* what synthesizer to use */
			(StereoFlag * initStereo)
				| initNoInterp, /* we do oversampling ourselves */
			(void*)MyCallBack /* == our buffer unmarking callback routine */);
		if (Error != noErr)
			{
				perror("unable to open sound channel");
				end_all("");
			}
		Cmd.cmd = ampCmd;
		Cmd.param1 = 255;
		Cmd.param2 = 0;
		Error = SndDoImmediate(MySoundChannel,&Cmd);
		if (Error != noErr)
			{
				perror("unable to initialize sound channel");
			}

		/* create an initial workspace */
		WorkspaceBytes = (1500 * sizeof(short) + 3) & ~3L; /* should be plenty */
		WorkspaceArray1 = (void*)NewPtr(WorkspaceBytes);
		WorkspaceArray2 = (void*)NewPtr(WorkspaceBytes);
		if ((WorkspaceArray1 == NULL) || (WorkspaceArray2 == NULL))
			{
				FatalError(FatalErrorOutOfMemory);
				perror("No memory to allocate workspace.");
				end_all("");
			}

		/* we now try to make as many buffers as we have memory for, so we can */
		/* precompute the composite samples to be played as far ahead as we can */
		/* so that momentary losses of control don't make the sound skip.	We */
		/* need at least 3 buffers, for double buffering and to make our full */
		/* usage check work.	(We check to see if all buffers are waiting to be played */
		/* and not empty by counting the number of ones with a marked "InUseFlag" */
		/* However, one buffer (the one under construction) will always be NOT */
		/* marked.	If we settle for 2 buffers, then when we account for the buffer */
		/* we are constructing, it will always look like all buffers are in use */
		/* until all have stopped.	The sound will skip, so we might as well not */
		/* play it at all.	Hence the limit of 3 buffers.) */
		Scan = 0;
		do
			{
				MyStructure*			Temp;
				long							FreeMemory;

				FreeMemory = FreeMem();
				if (FreeMemory < MinFreeMem)
					{
						if (Scan < 3)
							{
								perror("Not enough memory to play song!");
								FatalError(FatalErrorOutOfMemory);
								FatalError(FatalErrorOutOfMemory);
								end_all("");
							}
						 else
							{
								goto StopMakingBuffers;
							}
					}
				Buffers[Scan] = (void*)NewPtr(sizeof(MyStructure));
				Buffers[Scan]->InUseFlag = 0;
				Buffers[Scan]->Header.samplePtr = &(Buffers[Scan]->SampleArea[0]);
				if (mac_stereo)
					{
						Buffers[Scan]->Header.sampleSize = ActualNumBits;
						Buffers[Scan]->Header.numChannels = 2;
					}
				 else
					{
						Buffers[Scan]->Header.sampleSize = ActualNumBits;
						Buffers[Scan]->Header.numChannels = 1;
					}
				Buffers[Scan]->Header.sampleRate = FixedSampleRate;
				Buffers[Scan]->Header.loopStart = 0;
				Buffers[Scan]->Header.loopEnd = 0;
				Buffers[Scan]->Header.encode = extSH;
				Buffers[Scan]->Header.baseFrequency = 64;
				Buffers[Scan]->Header.markerChunk = NULL;
				Buffers[Scan]->Header.futureUse1 = 0;
				Buffers[Scan]->Header.futureUse2 = 0;
				Buffers[Scan]->Header.futureUse3 = 0;
				Buffers[Scan]->Header.futureUse4 = 0;
				Fred = SampleRate;
				x96tox80(&Fred,&(Buffers[Scan]->Header.AIFFSampleRate));
				Scan += 1;
			} while (Scan < MAXNUMBUFFERS);
	 StopMakingBuffers:
		ActualNumBuffers = Scan;
		for (Scan = 0; Scan < ActualNumBuffers; Scan += 1)
			{
				/* now we establish the circular linked list so that we can */
				/* easily find out what the "next" buffer to use is */
				Buffers[Scan]->Next = Buffers[(Scan + 1) % ActualNumBuffers];
			}
		CurrentBuffer = Buffers[0];
		buf_index = 0;

		return FixedSampleRate >> 16;
	}


/* this evaluates to see if all but the current buffer have been filled */
/* and are waiting to be played.	If they have, then we can take a break in */
/* the event loop since there won't be any work to do for a while. */
static int		is_channel_full(void)
	{
		int					Scan;
		int					Count;

		Count = 0;
		for (Scan = 0; Scan < ActualNumBuffers; Scan += 1)
			{
				if (Buffers[Scan]->InUseFlag)
					{
						Count += 1;
					}
			}
		if (Count >= ActualNumBuffers - 1)
			{
				return TRUE;
			}
		 else
	 		{
	 			return FALSE;
	 		}
	}


/* here we check to see if all the buffers have played out.	This is done */
/* at the end so that we don't close the channel too early and cut off the */
/* end of the song. */
static int		is_channel_empty(void)
	{
		int					Scan;

		for (Scan = 0; Scan < ActualNumBuffers; Scan += 1)
			{
				if (Buffers[Scan]->InUseFlag)
					{
						return FALSE; /* nope, buffers are still in use */
					}
			}
		return TRUE;
	}


/* this counts the number of buffers waiting to be played.	The event loop uses */
/* this to get an idea of how long it can sit around before worrying about */
/* filling some more buffers. */
short				NumberPendingBlocks(void)
	{
		int					Scan;
		int					Count;

		Count = 0;
		for (Scan = 0; Scan < ActualNumBuffers; Scan += 1)
			{
				if (Buffers[Scan]->InUseFlag)
					{
						Count += 1;
					}
			}
		return Count;
	}


/* queues the data to be played by the sound manager. */
void			actually_flush_buffer(void)
	{
		OSErr					Error;

		while (CurrentBuffer->Next->InUseFlag)
			{
				/* since the list is a circular list, the next element is the "oldest" */
				/* element.	If it is full, then all the others are too, so we just wait. */
				/* if all the buffers have been filled, then we'll just spinwait */
				/* here for 0.25 of a second waiting for one to play out.	This could */
				/* be a problem.	If the number of buffers is small, then they might */
				/* appear to be full, but they could still play out in less than 0.25 */
				/* of a second.	See also "may_getchar" where a similar thing is done. */
				WaitForEvent(15);
			}

	 TryAgainPoint1:
		CurrentBuffer->InUseFlag = 1;
		CurrentBuffer->MySoundCommand.cmd = bufferCmd;
		CurrentBuffer->MySoundCommand.param1 = 0;
		CurrentBuffer->MySoundCommand.param2 = (long)&(CurrentBuffer->Header);
		if (mac_stereo)
			{
				/* stereo sample frames have 2 "samples" in them, so the index */
				/* into the buffer is twice the number of frames. */
				CurrentBuffer->Header.numFrames = buf_index / (2 * (ActualNumBits / 8));
			}
		 else
			{
				CurrentBuffer->Header.numFrames = buf_index / (1 * (ActualNumBits / 8));
			}
		Error = SndDoCommand(MySoundChannel,&(CurrentBuffer->MySoundCommand),1);
		if (queueFull == Error)
			{
				WaitForEvent(2);
				goto TryAgainPoint1;
			}

		/* the callback is an interrupt level thing that clears a buffer so */
		/* we can use it again. */
	 TryAgainPoint2:
		CurrentBuffer->MyCallbackCommand.cmd = callBackCmd;
		CurrentBuffer->MyCallbackCommand.param2 = (long)&(CurrentBuffer->InUseFlag);
		Error = SndDoCommand(MySoundChannel,&(CurrentBuffer->MyCallbackCommand),1);
		if (queueFull == Error)
			{
				WaitForEvent(2);
				goto TryAgainPoint2;
			}

		buf_index = 0;
		CurrentBuffer = CurrentBuffer->Next;
	}


/* this function is not actually used */
void			output_samples(int left, int right)
	{
		/* in the normal tracker, this is called EVERY TIME a sample frame is */
		/* output.	It normally results in a call to flush_buffer.	Since */
		/* THINK C won't inline functions, this was way to slow, so I dispensed with it */
	}


/* this flushes the buffer IF another cycle would overflow it.	Since the number */
/* of bytes added to the buffer is SamplingRate / Speed, at high sampling rates */
/* or ridiculously low speeds, larger than the buffer size could be created in */
/* just one call to "resample."	This would be BAD.	Therefore "resample" checks */
/* for this and returns a fatal error if it would happen. */
void			my_flush_buffer(int BytesGenerated)
	{
		if (buf_index + BytesGenerated >= MAXBUFFERSIZE - 1)
			actually_flush_buffer();
	}


/* waiting for all samples to play and then closing sound channel & disposing buffers */
void			close_audio(void)
	{
		int						Scan;
		EventRecord		StupidEvent;

		if (buf_index != 0)
			{
				/* make sure that last 1/8 of a second worth of song gets played. */
				actually_flush_buffer();
			}

		/* wait for all buffers to play out */
		/* if we receive a quit event, we just go away */
		/* but for end of song, we wait for the buffers to play out */
		/* SndDisposeChannel does this, but we want to be friendly to other */
		/* processes by calling WaitNextEvent(), so we do it ourselves. */
		while (!QuitPending && !is_channel_empty())
			{
				WaitForEvent(60);
			}

		/* close macintosh sound channel, cancel any callbacks */
#if 0
		/* I used to be using this quietCmd which, according to the great Inside */
		/* Macintosh VI, should stop all processing on the channel.	Unfortunately, */
		/* when I perform the call, the channel promptly becomes useless and the */
		/* system hangs in the SndDisposeChannel routine waiting for some event */
		/* that'll never happen (I have no idea what, but MacsBug clearly shows the */
		/* infinite loop in system code). */
		if (QuitPending)
			{
				SndCommand		Cmd;

				Cmd.cmd = quietCmd;
				Cmd.param1 = 0;
				Cmd.param2 = 0;
				SndDoImmediate(MySoundChannel,&Cmd);
			}
#endif
		/* if we are supposed to quit right away, then we send 1 as the second */
		/* parameter to do it right away, otherwise it'll wait for everything to */
		/* play out before closing the channel */
		SndDisposeChannel(MySoundChannel,QuitPending);

		/* release memory allocated for buffers */
		for (Scan = 0; Scan < ActualNumBuffers; Scan += 1)
			{
				/* this isn't really necessary unless you play more than one song */
				/* at a time and the buffer is flushed each time. */
				DisposPtr((void*)Buffers[Scan]);
			}
	}


/* what is this for? */
void			set_synchro(int s)
	{
	}

/* another mystery function */
int			 update_frequency(void)
	{
		return 0;
	}

void			discard_buffer(void)
	{
	}


#if __option(profile)
#define Profiling
#endif

#pragma options(!profile)

/* asynchronous callback routine which marks buffers as now unused */
/* It would be very bad to profile this since profiling tampers with the */
/* stack invocation and uses A5 global variables!	(believe me, I've tried) */
static pascal void		MyCallBack(SndChannel* Channel, SndCommand* Command)
	{
		*(short*)(Command->param2) = 0;
	}

#ifdef Profiling
#pragma options(profile)
#endif








/* stuff removed from "audio.c" */


/* prototypes for my single channel resamplers have been added */
void		SampleAntiAliased8(struct audio_channel* ch, short* Buffer, short Count);
void		SampleAliased8(struct audio_channel* ch, short* Buffer, short Count);
void		SampleAntiAliased16(struct audio_channel* ch, short* Buffer, short Count);
void		SampleAliased16(struct audio_channel* ch, short* Buffer, short Count);


/* a new "resample" function using my single channel resamplers has been added */
void resample(int oversample, int number)
	{
		short				i;
		short*			WA1Shadow;
		short*			WA2Shadow;
		short				BytesGenerated;

		/* flush the stuff to the buffer.	i.e. if we would overflow the */
		/* buffer during this cycle, we flush it and get a new buffer. */
		/* later on, checks are performed to make sure we won't overflow the buffer. */
		/* the only time that would happen would be if BytesGenerated is bigger */
		/* than the whole buffer. */
		/* note that flush_buffer now takes a parameter */
		if (mac_stereo)
			{
				BytesGenerated = number * 2 * (ActualNumBits / 8);
			}
		 else
			{
				BytesGenerated = number * (ActualNumBits / 8);
			}
		my_flush_buffer(BytesGenerated);

		if (BytesGenerated + buf_index > MAXBUFFERSIZE)
			{
				perror("Buffer size not set large enough");
				FatalError(FatalErrorInternalError);
				end_all("");
			}

		/* we need a workspace where we construct the channels before scaling */
		/* the final volume.	If we need a bigger one than originally anticipated */
		/* we attempt to reallocate it. */
	 Reallocate:
		if (WorkspaceBytes == 0)
			{
				/* workspace always holds 16-bit values */
				WorkspaceBytes = (number * sizeof(short) + 3) & ~3L;
				WorkspaceArray1 = (void*)NewPtr(WorkspaceBytes);
				WorkspaceArray2 = (void*)NewPtr(WorkspaceBytes);
				if ((WorkspaceArray1 == NULL) || (WorkspaceArray2 == NULL))
					{
						perror("Ran out of memory playing song.");
						FatalError(FatalErrorOutOfMemory);
						end_all("");
					}
			}
		 else
			{
				if (WorkspaceBytes < ((number * sizeof(short) + 3) & ~3L))
					{
						DisposPtr((void*)WorkspaceArray1);
						DisposPtr((void*)WorkspaceArray2);
						WorkspaceBytes = 0;
						goto Reallocate;
					}
			}

		/* first, erase the workspace, since SampleXXX adds to the workspace. */
		WA1Shadow = WorkspaceArray1;
		WA2Shadow = WorkspaceArray2;
		for (i = WorkspaceBytes / sizeof(long) - 1; i >= 0; i -= 1)
			{
				/* longs erase faster on 68030 */
				((long*)WA1Shadow)[i] = 0;
				((long*)WA2Shadow)[i] = 0;
			}

		if (ActualNumBits == 8)
			{
				char*			BufferAddress;

				/* 8-bit sampling */

				/* oversample == 1 is merely no antialiasing.	I didn't want to fiddle with */
				/* Espie's code, so I just use oversample instead of a new variable "antialiased" */
				if (oversample == 1)
					{
						/* WorkspaceArray1 is the left channel, WorkspaceArray2 is the right channel */
						SampleAliased8(&(chan[0]),WA1Shadow,number);
						SampleAliased8(&(chan[1]),WA2Shadow,number);
						SampleAliased8(&(chan[2]),WA2Shadow,number);
						SampleAliased8(&(chan[3]),WA1Shadow,number);
					}
				 else
					{
						SampleAntiAliased8(&(chan[0]),WA1Shadow,number);
						SampleAntiAliased8(&(chan[1]),WA2Shadow,number);
						SampleAntiAliased8(&(chan[2]),WA2Shadow,number);
						SampleAntiAliased8(&(chan[3]),WA1Shadow,number);
					}
				 /* notes about that stuff above:	workspace always holds 16-bit values, so */
				 /* we don't have to fool with ActualNumBits to figure out how many bytes */
				 /* the workspace should be. */

				BufferAddress = &(CurrentBuffer->SampleArea[buf_index]);
				if (mac_stereo)
					{
						do
							{
								/* Note how we add both the primary and secondary scaling tables. */
								/* hence each table is independently clipped. */
								/* left channel */
								*(BufferAddress++) = FinalVolumeTablePrimary[(*WA1Shadow) & 0x03ff]
									+ FinalVolumeTableSecondary[(*WA2Shadow) & 0x03ff] + 128;
								/* right channel */
								*(BufferAddress++) = FinalVolumeTablePrimary[*(WA2Shadow++) & 0x03ff]
									+ FinalVolumeTableSecondary[*(WA1Shadow++) & 0x03ff] + 128;
								number -= 1;
							} while (number > 0);
					}
				 else
					{
						do
							{
								/* for mono, we don't have any secondary sound coming in from */
								/* the other channel. */
								*(BufferAddress++) = FinalVolumeTablePrimary[(*(WA1Shadow++)
									+ *(WA2Shadow++)) & 0x03ff] + 128;
								number -= 1;
							} while (number > 0);
					}
			}
		 else
			{
				short*			BufferAddress;

				/* 16 bit sampling */

				/* oversample == 1 is merely no antialiasing.	I didn't want to fiddle with */
				/* Espie's code, so I just use oversample instead of a new variable "antialiased" */
				if (oversample == 1)
					{
						/* WorkspaceArray1 is the left channel, WorkspaceArray2 is the right channel */
						SampleAliased16(&(chan[0]),WA1Shadow,number);
						SampleAliased16(&(chan[1]),WA2Shadow,number);
						SampleAliased16(&(chan[2]),WA2Shadow,number);
						SampleAliased16(&(chan[3]),WA1Shadow,number);
					}
				 else
					{
						SampleAntiAliased16(&(chan[0]),WA1Shadow,number);
						SampleAntiAliased16(&(chan[1]),WA2Shadow,number);
						SampleAntiAliased16(&(chan[2]),WA2Shadow,number);
						SampleAntiAliased16(&(chan[3]),WA1Shadow,number);
					}
				 /* notes about that stuff above:	workspace always holds 16-bit values, so */
				 /* we don't have to fool with ActualNumBits to figure out how many bytes */
				 /* the workspace should be. */

				BufferAddress = (short*)&(CurrentBuffer->SampleArea[buf_index]);
				if (mac_stereo)
					{
						short				PrimaryV;
						short				SecondaryV;

						PrimaryV = primary * Loudness;
						SecondaryV = secondary * Loudness;
						do
							{
								/* left channel */
								/* Primary + Secondary = 256; each buffer has -16384..16383 */
								/* in it, Loudness could be 64, so we need to divide by 256*64 */
								/* (16384; shift left by 14) */
								*(BufferAddress++) = (short)((((*WA1Shadow) * PrimaryV)
									+ ((*WA2Shadow) * SecondaryV)) >> 14) /*+ (short)0x8000*/;
								/* right channel */
								*(BufferAddress++) = (short)((((*(WA2Shadow++)) * PrimaryV)
									+ ((*(WA1Shadow++)) * SecondaryV)) >> 14) /*+ (short)0x8000*/;
								number -= 1;
							} while (number > 0);
					}
				 else
					{
						short				LocalLoudness;

						LocalLoudness = Loudness;
						do
							{
								/* Loudness could be as high as 64, and the buffers have */
								/* -16384..16383, which, added together, is -32768..32767 */
								/* so we need to divide by 64 (shift left by 6) */
								*(BufferAddress++) = (short)(
									(((long)*(WA1Shadow++) + (long)*(WA2Shadow++)) * LocalLoudness)
									>> 6) /*+ (short)0x8000*/;
								number -= 1;
							} while (number > 0);
					}
			}

		buf_index += BytesGenerated; /* account for added bytes */
	}


/* the actual functions for single-channel resampling */


#if !USE_ASSEMBLY_CODE
void			SampleAliased8(struct audio_channel* ch, short* Buffer, short Count)
	{
		long				Pointer;
		long				FixLength;
		char*				SampleData;
		long				Step;
		long				LoopLength;
		short				Volume;

		if (ch->mode != DO_NOTHING)
			{
				Pointer = ch->pointer;
				FixLength = ch->samp->fix_length;
				SampleData = ch->samp->start;
				Step = ch->step;
				LoopLength = ch->samp->fix_rp_length;
				Volume = CONSTRAIN(ch->volume,MIN_VOLUME,MAX_VOLUME) - MIN_VOLUME;
			 LoopPoint:
				if (Pointer >= FixLength)
					{
						/* is there a replay ? */
						if (!ch->samp->rp_start)
							{
								ch->mode = DO_NOTHING;
								goto OutPoint;
							}
						ch->mode = REPLAY;
						Pointer -= LoopLength;
						goto LoopPoint;
					}
				*(Buffer++) += (Volume * SampleData[fix_to_int(Pointer)]) >> 6;
				Pointer += Step;
				Count -= 1;
				if (Count > 0)
					{
						goto LoopPoint;
					}
			 OutPoint:
				ch->pointer = Pointer;
			}
	}
#else
void			SampleAliased8(struct audio_channel* ch, short* Buffer, short Count)
	{
		#define Pointer	D0
		#define Step	D1
		#define FixLength	D2
		#define LocalCount	D3
		#define Temp	D4
		#define Temp2	D5

		#define SampleData	A0
		#define VolumeMap	A1
		#define Channel	A2
		#define SampPtr	A3
		#define LocalBuffer	A4

		asm
			{
				movem.l			D3-D5/A2-A4,-(A7)

				/* loading global variables & parameters:	Must NOT disturb A5 and A6 here */
				move.l			ch,Channel
				move.w			Count,LocalCount
				move.l			OFFSET(struct audio_channel,volume)(Channel),Temp	;got volume
				cmp.l				#MIN_VOLUME,Temp
				bge.s				@8
				moveq.l			#0,Temp	;if volume < 0, constrain it to 0
		@8: cmp.l				#MAX_VOLUME,Temp
				bmi.s				@7
				moveq.l			#MAX_VOLUME,Temp	;if volume > max volume, constain it to max
		@7:
#if 0 != MIN_VOLUME
				;this bit is untested, but I think it will work.	Don't see why
				;MIN_VOLUME would ever be != 0, though
				add.l				#(MIN_VOLUME * 256 * sizeof(short)),Temp	;table starts at 0, so add this in
#endif
				moveq				#9,Temp2
				lsl.l				Temp2,Temp	;multiplied by 256*sizeof(short) to obtain offset
				move.l			SubVolumeTable,VolumeMap	;get base address
				add.l				Temp,VolumeMap	;add offset
				move.l			Buffer,LocalBuffer
				subq.w			#1,LocalCount
				/* loading channel things */
				move.l			OFFSET(struct audio_channel,pointer)(Channel),Pointer
				move.l			OFFSET(struct audio_channel,step)(Channel),Step
				/* loading sample things */
				move.l			OFFSET(struct audio_channel,samp)(Channel),SampPtr
				move.l			OFFSET(struct sample_info,fix_length)(SampPtr),FixLength
				move.l			OFFSET(struct sample_info,start)(SampPtr),SampleData
				/* clearing high bits of certain registers */

				cmp.l				#DO_NOTHING,OFFSET(struct audio_channel,mode)(Channel)
				beq.s				@OutPoint

		@3: cmp.l				Pointer,FixLength
				bhi.s				@1
				tst.l				OFFSET(struct sample_info,rp_start)(SampPtr)
				bne.s				@2
				move.l			#DO_NOTHING,OFFSET(struct audio_channel,mode)(Channel)
				bra.s				@OutPoint
		@2: sub.l				OFFSET(struct sample_info,fix_rp_length)(SampPtr),Pointer
				move.l			#REPLAY,OFFSET(struct audio_channel,mode)(Channel)
				bra.s				@3

		@1:
				move.l			Pointer,Temp
				moveq				#ACCURACY,Temp2
				lsr.l				Temp2,Temp	;now Temp is the data index
				moveq.l			#0,Temp2
				move.b			0(SampleData,Temp.L),Temp2	;get byte
#if __option(mc68020)
				move.w			0(VolumeMap,Temp2.L*2),Temp	;getting volume corrected value
#else
				lsl.l				#1,Temp2
				move.w			0(VolumeMap,Temp2.L),Temp
#endif
				add.w				Temp,(LocalBuffer)+	;adding data into temporary buffer

				add.l				Step,Pointer	;incrementing pointer

				dbf					LocalCount,@3

		@OutPoint:
				move.l			Pointer,OFFSET(struct audio_channel,pointer)(Channel)
				movem.l			(A7)+,D3-D5/A2-A4
			}

		#undef Pointer
		#undef Step
		#undef FixLength
		#undef LocalCount
		#undef Temp
		#undef Temp2

		#undef SampleData
		#undef VolumeMap
		#undef Channel
		#undef SampPtr
		#undef LocalBuffer
	}
#endif


#if !USE_ASSEMBLY_CODE
void			SampleAntiAliased8(struct audio_channel* ch, short* Buffer, short Count)
	{
		long							Pointer;
		long							FixLength;
		char*							SampleData;
		long							Step;
		long							LoopLength;
		short							Volume;
		short							LeftWeight;
		short							RightWeight;

		if (ch->mode != DO_NOTHING)
			{
				Pointer = ch->pointer;
				FixLength = ch->samp->fix_length;
				SampleData = ch->samp->start;
				Step = ch->step;
				LoopLength = ch->samp->fix_rp_length;
				Volume = CONSTRAIN(ch->volume,MIN_VOLUME,MAX_VOLUME) - MIN_VOLUME;
			 LoopPoint:
				if (Pointer >= FixLength)
					{
						/* is there a replay ? */
						if (!ch->samp->rp_start)
							{
								ch->mode = DO_NOTHING;
								goto OutPoint;
							}
						ch->mode = REPLAY;
						Pointer -= LoopLength;
						goto LoopPoint;
					}
				RightWeight = Pointer & ((1 << ACCURACY) - 1);
				LeftWeight = (1 << ACCURACY) - RightWeight;
				*(Buffer++) +=
					(Volume * ((LeftWeight * SampleData[fix_to_int(Pointer)])
					+ (RightWeight * SampleData[fix_to_int(Pointer) + 1])) >> (ACCURACY + 1 + 6));
				Pointer += Step;
				Count -= 1;
				if (Count > 0)
					{
						goto LoopPoint;
					}
			 OutPoint:
				ch->pointer = Pointer;
			}
	}
#else
void			SampleAntiAliased8(struct audio_channel* ch, short* Buffer, short Count)
	{
		#define Pointer	D0
		#define Mask	D1
		#define Step	D2
		#define FixLength	D3
		#define LocalCount	D4
		#define Temp	D5
		#define Temp2	D6

		#define SampleData	A0
		#define AliasMap	A1
		#define VolumeMap	A2
		#define Channel	A3
		#define SampPtr	A4
		#define LocalBuffer	A5

		asm
			{
				movem.l			D3-D6/A2-A5,-(A7)

				/* loading global variables & parameters:	Must NOT disturb A5 and A6 here */
				move.l			AliasTable,AliasMap
				move.l			ch,Channel
				move.w			Count,LocalCount
				move.l			OFFSET(struct audio_channel,volume)(Channel),Temp	;got volume
				cmp.l				#MIN_VOLUME,Temp
				bge.s				@8
				moveq.l			#0,Temp	;if volume < 0, constrain it to 0
		@8: cmp.l				#MAX_VOLUME,Temp
				bmi.s				@7
				moveq.l			#MAX_VOLUME,Temp	;if volume > max volume, constain it to max
		@7:
#if 0 != MIN_VOLUME
				;this bit is untested, but I think it will work.	Don't see why
				;MIN_VOLUME would ever be != 0, though
				add.l				#(MIN_VOLUME * 256 * sizeof(short)),Temp	;table starts at 0, so add this in
#endif
				moveq				#9,Temp2
				lsl.l				Temp2,Temp	;multiplied by 256*sizeof(short) to obtain offset
				move.l			SubVolumeTable,VolumeMap	;get base address
				add.l				Temp,VolumeMap	;add offset
				move.l			Buffer,LocalBuffer
				subq.w			#1,LocalCount
				/* loading channel things */
				move.l			OFFSET(struct audio_channel,pointer)(Channel),Pointer
				move.l			OFFSET(struct audio_channel,step)(Channel),Step
				/* loading sample things */
				move.l			OFFSET(struct audio_channel,samp)(Channel),SampPtr
				move.l			OFFSET(struct sample_info,fix_length)(SampPtr),FixLength
				move.l			OFFSET(struct sample_info,start)(SampPtr),SampleData

				/* clearing high bits of certain registers */
#if __option(mc68020)
				;this is an invariant optimization which is only meaningful on the 68020, so
				;we don't need it if we are compiling for 68000
				clr.l				Mask
#endif

				cmp.l				#DO_NOTHING,OFFSET(struct audio_channel,mode)(Channel)
				beq.s				@OutPoint

		@3: cmp.l				Pointer,FixLength
				bhi.s				@1
				tst.l				OFFSET(struct sample_info,rp_start)(SampPtr)
				bne.s				@2
				move.l			#DO_NOTHING,OFFSET(struct audio_channel,mode)(Channel)
				bra.s				@OutPoint
		@2: sub.l				OFFSET(struct sample_info,fix_rp_length)(SampPtr),Pointer
				move.l			#REPLAY,OFFSET(struct audio_channel,mode)(Channel)
				bra.s				@3

		@1:
				;calculating anti-aliasing mask
#if !__option(mc68020)
				moveq.l			#0,Mask	;make sure to clear the high bits each time for 68000
#endif
				move.w			Pointer,Mask	;get lower 16 bits of pointer
				and.w				#0x0f00,Mask	;keep only upper 4 bits of fraction
				;this conveniently works out to leave 8 bits just below the accuracy
				;into which we can stuff bytes. */
#if ACCURACY != 12
#error "ACCURACY == 12 is hardwired!"
#endif

				;fetching data bytes
				move.l			Pointer,Temp
				moveq				#ACCURACY,Temp2
				lsr.l				Temp2,Temp	;now Temp is the data index
				moveq.l			#0,Temp2
				move.b			0(SampleData,Temp.L),Mask	;or in left byte
#if __option(mc68020)
				move.b			0(AliasMap,Mask.L*2),Temp2	;got left product
				move.b			1(SampleData,Temp.L),Mask	;or in right byte
				add.b				1(AliasMap,Mask.L*2),Temp2	;added in right product
				move.w			0(VolumeMap,Temp2.L*2),Temp	;getting volume corrected value
#else
				lsl.l				#1,Mask
				move.b			0(AliasMap,Mask.L),Temp2	;got left product
				lsr.l				#1,Mask	;must shift back, since we OR in a byte next
				move.b			1(SampleData,Temp.L),Mask	;or in right byte
				lsl.l				#1,Mask
				add.b				1(AliasMap,Mask.L),Temp2	;added in right product
				lsl.l				#1,Temp2
				move.w			0(VolumeMap,Temp2.L),Temp	;getting volume corrected value
#endif
				add.w				Temp,(LocalBuffer)+	;adding data into temporary buffer

				add.l				Step,Pointer	;incrementing pointer

				dbf					LocalCount,@3

		@OutPoint:
				move.l			Pointer,OFFSET(struct audio_channel,pointer)(Channel)
				movem.l			(A7)+,D3-D6/A2-A5
			}

		#undef Pointer
		#undef Mask
		#undef Step
		#undef FixLength
		#undef LocalCount
		#undef Temp
		#undef Temp2

		#undef SampleData
		#undef AliasMap
		#undef VolumeMap
		#undef Channel
		#undef SampPtr
		#undef LocalBuffer
	}
#endif


/* Same as SampleAliased8 except it doesn't divide out the volume */
void			SampleAliased16(struct audio_channel* ch, short* Buffer, short Count)
	{
		long				Pointer;
		long				FixLength;
		char*				SampleData;
		long				Step;
		long				LoopLength;
		short				Volume;

		if (ch->mode != DO_NOTHING)
			{
				Pointer = ch->pointer;
				FixLength = ch->samp->fix_length;
				SampleData = ch->samp->start;
				Step = ch->step;
				LoopLength = ch->samp->fix_rp_length;
				Volume = CONSTRAIN(ch->volume,MIN_VOLUME,MAX_VOLUME) - MIN_VOLUME;
			 LoopPoint:
				if (Pointer >= FixLength)
					{
						/* is there a replay ? */
						if (!ch->samp->rp_start)
							{
								ch->mode = DO_NOTHING;
								goto OutPoint;
							}
						ch->mode = REPLAY;
						Pointer -= LoopLength;
						goto LoopPoint;
					}
				/* placing result in buffer -- ranges between 64*-128 .. 64*128. */
				/* that way we can add two channels to get 15 bits worth of stuff */
				/* or 4 channels for 16 bits worth of stuff */
				*(Buffer++) += Volume * SampleData[fix_to_int(Pointer)];
				Pointer += Step;
				Count -= 1;
				if (Count > 0)
					{
						goto LoopPoint;
					}
			 OutPoint:
				ch->pointer = Pointer;
			}
	}


/* Same as SampleAntiAliased8 except it doesn't divide out the volume */
void			SampleAntiAliased16(struct audio_channel* ch, short* Buffer, short Count)
	{
		long							Pointer;
		long							FixLength;
		char*							SampleData;
		long							Step;
		long							LoopLength;
		short							Volume;
		short							LeftWeight;
		short							RightWeight;

		if (ch->mode != DO_NOTHING)
			{
				Pointer = ch->pointer;
				FixLength = ch->samp->fix_length;
				SampleData = ch->samp->start;
				Step = ch->step;
				LoopLength = ch->samp->fix_rp_length;
				Volume = CONSTRAIN(ch->volume,MIN_VOLUME,MAX_VOLUME) - MIN_VOLUME;
			 LoopPoint:
				if (Pointer >= FixLength)
					{
						/* is there a replay ? */
						if (!ch->samp->rp_start)
							{
								ch->mode = DO_NOTHING;
								goto OutPoint;
							}
						ch->mode = REPLAY;
						Pointer -= LoopLength;
						goto LoopPoint;
					}
				RightWeight = Pointer & ((1 << ACCURACY) - 1);
				LeftWeight = (1 << ACCURACY) - RightWeight;
				/* placing result in buffer -- ranges between 64*-128 .. 64*128. */
				/* that way we can add two channels to get 15 bits worth of stuff */
				/* or 4 channels for 16 bits worth of stuff */
				*(Buffer++) +=
					(Volume * (((LeftWeight * SampleData[fix_to_int(Pointer)])
					+ (RightWeight * SampleData[fix_to_int(Pointer) + 1])) >> (ACCURACY + 1)));
				Pointer += Step;
				Count -= 1;
				if (Count > 0)
					{
						goto LoopPoint;
					}
			 OutPoint:
				ch->pointer = Pointer;
			}
	}
