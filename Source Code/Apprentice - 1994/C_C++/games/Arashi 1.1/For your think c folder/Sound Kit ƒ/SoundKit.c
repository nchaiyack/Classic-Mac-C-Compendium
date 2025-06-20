/*/
     Project Arashi: SoundKit.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, May 26, 1993, 18:49
     Created: Saturday, March 18, 1989, 0:55

     Copyright � 1989-1993, Juri Munkki
/*/

#define	MAKE_SOUNDs
#define	SOUNDKIT_MAIN
#include <Sound.h>
#include <Retrace.h>
#include "Shuddup.h"
#include <GestaltEqu.h>

SndDoubleBufferHeader	SKDouble;
SndChannelPtr			SKChannel;

#define	SKBUFFERSIZE	(1024)	/*	Size of double buffering data area.		*/
#define	SKBUFFERBYTES	(1024)	/*	Actual amount of sound data in buffer.	*/
#define	KHZ11			(0x2B7745D1L)
#define	KHZ22			(0x56EE8BA3L)

int			OldVol;				/*	Sound volume at InitSoundKit time.	*/
int			NumSounds;			/*	Number of packed sounds.			*/
int			OldSound=0;			/*	Change to 1 to use old sound driver.*/

/*	Note: OldSound works only if you change it before InitSoundKit.		*/

Ptr			SKPtr;				/*	Pointer for Sound Kit data.			*/
SoundStuff	*Sounds;			/*	Storage for sound info table.		*/
FFSynthPtr	SoundBuf;			/*	Our one and only FFSynthRec			*/
VVars		Vv;					/*	Vertical blanking variables			*/
long		SKTicks;			/*	Tickcount from Sound Kit VBL.		*/

/*
>>	If A5 points to a VVars structure, the following macro will become
>>	xx(A5), where xx is the offset into that structure element.
*/
#define VBV(field)		((int) &((VVars *) 0)->field)(A5)

/*
>>	SKWorkHorse is used by the SoundManager compatible routines to
>>	fill up a new buffer of 512 sound bytes. It mixes two channels
>>	into one by adding them up.
*/
void	SKWorkHorse()
{
asm	{
		move.l	A2,-(sp)
		move.l	VBV(SoundB),A0			;	Get address of sound buffer

		move.w	VBV(CountA),D0			;	Is channel A active?
		beq.s	@silentone				;	No
										;	Yes:
		sub.w	#1,VBV(CountA)			;	Decrement counter
		move.l	VBV(ChanA),A1			;	Get sound data address

		move.w	VBV(CountB),D0			;	Is channel B active?
		bne.s	@twosounds				;	Yes
										;	No:
		move.l	#0x40404040,D1			;	Other channel is silent
	
		moveq.l	#63,D0					;	Loop counter.
@vbAloop
		move.l	D1,D2					;	Copy silence
		add.l	(A1)+,D2				;	Add sound to silence
		move.l	D2,(A0)+				;	Store sound+silence

		move.l	D1,D2					;	...
		add.l	(A1)+,D2
		move.l	D2,(A0)+

		dbra	D0,@vbAloop
		move.l	A1,VBV(ChanA)			;	Update sound data pointer
		bra.s	@endvbl					;	Return from vbl routine

@silentone
		move.w	VBV(CountB),D0			;	Are both channels silent?
		beq.s	@nosound				;	Yes
										;	No
		sub.w	#1,VBV(CountB)			;	Play only channel B.
		move.l	VBV(ChanB),A1			;	comments same as for
		move.l	#0x40404040,D1			;	channel A above.
		moveq.l	#63,D0

@vbBloop
		move.l	D1,D2
		add.l	(A1)+,D2
		move.l	D2,(A0)+

		move.l	D1,D2
		add.l	(A1)+,D2
		move.l	D2,(A0)+
		dbra	D0,@vbBloop

		move.l	A1,VBV(ChanB)
		bra.s	@endvbl					;	Return from vbl routine
@twosounds
		sub.w	#1,VBV(CountB)			;	Decrement channel B count
		move.l	VBV(ChanB),A2			;	A2 = channel B sound data pointer
		move.w	#63,D0					;	

@vbABloop
		move.l	(A1)+,D1				;	Get Sound channel A
		add.l	(A2)+,D1				;	Add channel B
		move.l	D1,(A0)+				;	Store sound

		move.l	(A1)+,D1				;	Get Sound channel A
		add.l	(A2)+,D1				;	Add channel B
		move.l	D1,(A0)+				;	Store sound

		dbra	D0,@vbABloop			;	Loop
		move.l	A1,VBV(ChanA)			;	Update sound channel A data pointer
		move.l	A2,VBV(ChanB)			;	Update sound channel B data pointer
		bra.s	@endvbl					;	Return from vbl routine
@nosound
		move.l	#0x80808080,D1			;	Silence
		move.w	#127,D0					;	512 bytes of silence.
@silencer
		move.l	D1,(A0)+				;	Store silence
		dbra	D0,@silencer			;	loop
@endvbl
		move.l	(sp)+,A2
		return							;	Return from vbl routine
	}
}

/*
>>	See IM-VI to find out what a DoubleBackRoutine is.
>>	Basically it's just a routine that feeds data to the
>>	sound manager.
*/
pascal	void	SKDoubleBackRoutine(chan,buf)
SndChannelPtr	chan;
SndDoubleBuffer	*buf;
{
	register	char	*sdata;
	register	VVars	*VVFrame;
				char	sounder[512];
	
	buf->dbFlags=dbBufferReady;
	
	VVFrame=(VVars *)buf->dbUserInfo[0];
	VVFrame->SoundB=sounder;

	sdata=buf->dbSoundData;

asm	{	move.l	A5,-(sp)
		move.l	VVFrame,A5
		bsr		SKWorkHorse
		move.l	(sp)+,A5

		lea		sounder,A0
		moveq.l	#127,D0
@loop
		move.b	(A0)+,D1
		move.b	D1,(sdata)+
		move.b	D1,(sdata)+

		move.b	(A0)+,D1
		move.b	D1,(sdata)+
		move.b	D1,(sdata)+

		move.b	(A0)+,D1
		move.b	D1,(sdata)+
		move.b	D1,(sdata)+
		
		move.b	(A0)+,D1
		move.b	D1,(sdata)+
		move.b	D1,(sdata)+
		
		dbra	D0,@loop
	}
}

/*
>>	Setup vertical blanking variables &
>>	install vertical blanking task.
*/
void	InstallMyVBL()
{
	register	long	vbltask;
	register	Handle 	SystemHand;
	
	asm	{
		lea		@myvbltask,A0		;	Get address of vbltask
		move.l	A0,vbltask			;	Store in local variable
		lea		@mybase,A0			;	Get addr of variable base storage
		lea		Vv,A1				;	Get addr of Vv record
		move.l	A1,(A0)				;	Store base in base storage
		}
	
	SystemHand=GetResource(SKRESTYPE,SKSYSJUMP);
	Vv.VBL.qType=vType;				/*	Vertical blanking queue.	*/
	Vv.VBL.vblAddr=(ProcPtr) *SystemHand;
	*(long *)(2+*SystemHand)=(long)vbltask;
									/*	Address of task				*/
	Vv.VBL.vblCount=1;				/*	Every 1/60 seconds			*/
	Vv.VBL.vblPhase=0;				/*	0 is ok..					*/
	Vv.TickPtr=&SKTicks;			/*	Faster than TickCount()		*/
	VInstall((void *)&Vv.VBL);		/*	Install task in queue		*/
	if(0)
asm	{	
@myvbltask
		move.l	A5,-(SP)				;	Save A5
		move.l	@mybase,A5				;	Get Vv address into A5
		move.w	#1,VBV(VBL.vblCount)	;	Call again in 1/60 seconds
		move.l	VBV(TickPtr),A0			;	Get pointer to private tickcount
		addq.l	#1,(A0)					;	Increment private tickcount

		clr.l	D0						;	Change sound buffer by
		move.w	#256,D0					;	setting:
		sub.w	VBV(SoundP),D0			;	SoundP = 256 - SoundP
		move.w	D0,VBV(SoundP)			;	Store SoundP for next time
		move.l	D0,VBV(SPar.ioActCount)	;	Store SoundP into ioActCount
		move.l	VBV(SoundB),A0			;	Get address of sound buffer
		add.l	D0,A0					;	Add SoundP to SoundB

		move.w	VBV(CountA),D0			;	Is channel A active?
		beq.s	@silentone				;	No
										;	Yes:
		sub.w	#1,VBV(CountA)			;	Decrement counter
		move.l	VBV(ChanA),A1			;	Get sound data address

		move.w	VBV(CountB),D0			;	Is channel B active?
		bne.s	@twosounds				;	Yes
										;	No:
		move.l	#0x40404040,D1			;	Other channel is silent
		move.l	D1,D2					;	Copy silence
		add.l	(A1)+,D2				;	Add sound to silence
		move.l	D2,(A0)+				;	Store sound+silence
	
		moveq.l	#22,D0					;	Loop counter.
@vbAloop
		move.l	D1,D2					;	Copy silence
		add.l	(A1)+,D2				;	Add sound to silence
		move.l	D2,(A0)+				;	Store sound+silence

		move.l	D1,D2					;	...
		add.l	(A1)+,D2
		move.l	D2,(A0)+

		dbra	D0,@vbAloop
		move.l	A1,VBV(ChanA)			;	Update sound data pointer
		move.l	(SP)+,A5				;	Restore A5
		rts								;	Return from vbl routine

@silentone
		move.w	VBV(CountB),D0			;	Are both channels silent?
		beq.s	@nosound				;	Yes
										;	No
		sub.w	#1,VBV(CountB)			;	Play only channel B.
		move.l	VBV(ChanB),A1			;	comments same as for
		move.l	#0x40404040,D1			;	channel A above.
		moveq.l	#22,D0

		move.l	D1,D2
		add.l	(A1)+,D2
		move.l	D2,(A0)+
@vbBloop
		move.l	D1,D2
		add.l	(A1)+,D2
		move.l	D2,(A0)+

		move.l	D1,D2
		add.l	(A1)+,D2
		move.l	D2,(A0)+
		dbra	D0,@vbBloop

		move.l	A1,VBV(ChanB)
		move.l	(SP)+,A5				;	Restore A5
		rts								;	Return from vbl routine
@twosounds
		sub.w	#1,VBV(CountB)			;	Decrement channel B count
		move.l	VBV(ChanB),A2			;	A2 = channel B sound data pointer
		move.w	#22,D0					;	((23 * 8) + 4) bytes = 188 bytes

		move.l	(A1)+,D1				;	Get Sound channel A
		add.l	(A2)+,D1				;	Add channel B
		move.l	D1,(A0)+				;	Store sound

@vbABloop
		move.l	(A1)+,D1				;	Get Sound channel A
		add.l	(A2)+,D1				;	Add channel B
		move.l	D1,(A0)+				;	Store sound

		move.l	(A1)+,D1				;	Get Sound channel A
		add.l	(A2)+,D1				;	Add channel B
		move.l	D1,(A0)+				;	Store sound

		dbra	D0,@vbABloop			;	Loop
		move.l	A1,VBV(ChanA)			;	Update sound channel A data pointer
		move.l	A2,VBV(ChanB)			;	Update sound channel B data pointer
		move.l	(SP)+,A5				;	Restore A5
		rts								;	Return from vbl routine
@nosound
		move.l	#0x80808080,D1			;	Silence
		move.w	#46,D0					;	(45+1)*4 bytes of silence
@silencer
		move.l	D1,(A0)+				;	Store silence
		dbra	D0,@silencer			;	loop
@endvbl
		move.l	(SP)+,A5				;	Restore A5
		rts								;	Return from vbl routine
@mybase
		dc.l	0						;	Vv record address stored here.
	}
}

/*
>>	You should call CloseSoundKit before exiting your program.
>>	The sound kit tries to make sure you do, but doing it yourself
>>	will be more reliable. It should be possible to reopen the
>>	sound kit after you have closed it. I haven't tried to do that.
*/

static	long	oldExit;

void	CloseSoundKit()
{
	if(Vv.SKOpen)
	{
		if(OldSound)
		{	VRemove((void *)&Vv.VBL);
			StopSound();
		}
		else
		{	SndDisposeChannel(SKChannel,-1);
		}

		SetSoundVol(OldVol);
		DisposPtr(SKPtr);
		NumSounds=0;
		Vv.SKOpen=0;
		NSetTrapAddress(oldExit,0x9F4,ToolTrap);
	}
}

/*
>>	ExitToShell is patched to call this routine
>>	to make sure that the kit is closed before the
>>	program quits. Without this patch, forgetting to
>>	close the sound kit and exiting the program generated
>>	some interesting and exiting side-effects like
>>	crashes and trashed memory locations.
*/

void	SoundExitToShell()
{
	CloseSoundKit();
	asm	{	move.l	oldExit,A0
			jmp		(A0)
		}
}
/*
>>	Patch exittoshell to make sure that vbl routine is removed.
*/
void	ExitSoundPatch()
{	
	oldExit=NGetTrapAddress(0x9F4,ToolTrap);
	NSetTrapAddress((long)SoundExitToShell,0x9F4,ToolTrap);
}
/*
>>	Initialize FFSynthRec and start producing silence.
*/
void	StartNoise()
{
	register	long	*p;
	register	int		i;

	SoundBuf=(FFSynthPtr)NewPtr(700);	/*	Allocate buffer					*/
	
	p=(long *)SoundBuf;
	for(i=700/4;i;i--)	*p++=0x80808080;/*	Store silence in buffer			*/

	SoundBuf->mode=ffMode;				/*	Freeform (sampled) data			*/
	SoundBuf->count=FixRatio(1,2);		/*	11 Khz sampling rate			*/

	Vv.SPar.ioRefNum=-4;				/*	Sound driver.					*/
	Vv.SPar.ioBuffer=(Ptr)SoundBuf;		/*	FFSynthPtr stored here			*/
	Vv.SPar.ioReqCount=660;				/*	Play 660 bytes (At least)		*/
	Vv.SPar.ioCompletion=0;				/*	No ioCompletion routine.		*/
	Vv.SoundB=(void *)SoundBuf->waveBytes;/*	Store buffer start address		*/
	Vv.SoundP=0;						/*	Start with buffer at offset 0	*/

	PBWrite(&Vv.SPar,-1);				/*	Start playing					*/
}

/*
>>	SKVolume allows you to change the sound volume.
>>	Note that all Macs are capable of 9 distinct
>>	sound volumes. Volume 0 is usually understood
>>	as silence, but actually it is just a low volume
>>	setting. SKVolume reassigns these values so that
>>	0 is really silence and 8 is the maximum volume.
*/
void	SKVolume(vol)
int		vol;
{
	if(vol>8) vol=8;
	if(vol<0) vol=0;
	Vv.volume=vol;
	if(vol)
	{	SetSoundVol(vol-1);
		if(OldSound)
			PBWrite(&Vv.SPar,-1);
	}
	else
	{	Vv.CountA=0;
		Vv.CountB=0;
	}
}

/*
>>	Create a double[back] buffering data structure for
>>	the sound manager.
*/
SndDoubleBuffer	*SKCreateDBuffer()
{
	register	SndDoubleBuffer	*buf;
	register	int				i;
	register	char			*p;

	buf=(SndDoubleBuffer *)NewPtr(SKBUFFERSIZE+sizeof(SndDoubleBuffer));
	buf->dbNumFrames=SKBUFFERBYTES;
	buf->dbUserInfo[0]= (long)&Vv;
	buf->dbFlags=dbBufferReady;
	
	p= buf->dbSoundData;
	for(i=0;i<SKBUFFERSIZE;i++)
	{	*p++= 128;
	}
	
	return	buf;
}
void	InitSoundKit()
{
	int		err;
	long	response;

	GetSoundVol((void *)&OldVol);
	SKVolume(OldVol+1);
	DeCompress();
	DecodeSounds();

	ExitSoundPatch();

	if(Gestalt(gestaltSoundAttr, &response))
	{	OldSound = 1;
	}
	else
	{	if(response & (1 << (31 - gestaltSoundIOMgrPresent)))
		{	OldSound = 1;
		}
	}

	if(OldSound)
	{	StartNoise();
		InstallMyVBL();
	}
	else
	{
		SKDouble.dbhNumChannels		=1;
		SKDouble.dbhSampleSize		=8;
		SKDouble.dbhCompressionID	=0;
		SKDouble.dbhPacketSize		=0;
		SKDouble.dbhSampleRate		=KHZ22;
		
		SKDouble.dbhBufferPtr[0]	=SKCreateDBuffer();
		SKDouble.dbhBufferPtr[1]	=SKCreateDBuffer();
		SKDouble.dbhDoubleBack		=(void *)SKDoubleBackRoutine;

		SKChannel=0;
		err=SndNewChannel(&SKChannel,sampledSynth,4,0L);
		err=SndPlayDoubleBuffer(SKChannel,&SKDouble);
	}
	Vv.SKOpen=-1;
}

void	PlayA(num,priority)
register	int		num,priority;
{
	if(Vv.volume)			/*	If volume is not off.					*/
	if(num<NumSounds)		/*	And the sound number looks possible.	*/
	if(num>=0)
	if(priority >= Vv.PriA || Vv.CountA==0)	/*	Can channel be used?	*/
	{	Vv.CountA=0;						/*	Stop old sound.			*/
		Vv.ChanA=Sounds[num].Poin;			/*	Pointer to new sound.	*/
		Vv.CountA=Sounds[num].Count;		/*	Start new sound.		*/
		Vv.PriA=priority;					/*	Store priority.			*/
	}
	else					/*	Try if the other channel is available.	*/
	{	if(Vv.CountB==0)
		{	Vv.CountB=0;
			Vv.ChanB=Sounds[num].Poin;
			Vv.CountB=Sounds[num].Count;
			Vv.PriB=0;
		}
	}
}
void	PlayB(num,priority)
register	int		num,priority;
{
	if(Vv.volume)
	if(num<NumSounds)
	if(num>=0)
	if(priority >= Vv.PriB || Vv.CountB==0)
	{	Vv.CountB=0;
		Vv.ChanB=Sounds[num].Poin;
		Vv.CountB=Sounds[num].Count;
		Vv.PriB=priority;
	}
	else
	{	if(Vv.CountA==0)
		{	Vv.CountA=0;
			Vv.ChanA=Sounds[num].Poin;
			Vv.CountA=Sounds[num].Count;
			Vv.PriA=0;
		}
	}
}

/*
>>	SKProcessHandle allows normal sound sample handles
>>	to be converted to a form suitable for the sound kit.
*/
void	SKProcessHandle(thedata)
Handle	thedata;
{
	register	unsigned	char	*source,*dest;
	register	long				len,oldlen;
	register	int					counter;
	register	int					sampleframes,samplepad;
	
	if(OldSound)
	{	sampleframes = 185;
		samplepad = 188;
	}
	else
	{	sampleframes = 512;
		samplepad = 512;
	}
	
	oldlen = GetHandleSize(thedata);
	len = (oldlen + sampleframes - 1) / sampleframes * samplepad;
	
	SetHandleSize(thedata,len+sizeof(long));
	
	if(!MemErr)
	{	BlockMove(*thedata,*thedata + len - oldlen + sizeof(long), oldlen);
		dest = (unsigned char *)*thedata;
		*(long *)dest = (oldlen + sampleframes - 1) / sampleframes;
		dest += sizeof(long);
		source = dest + len - oldlen;
		counter = 0;
		
		while(oldlen--)
		{	*dest++ = (*source++)>>1;
			len--;
			counter++;
			
			if(counter == sampleframes)
			{	while(counter++ < samplepad)
				{	*dest++ = 64;
					len--;
				}
				counter = 0;
			}
		}
		while(len--)
		{	*dest++ = 64;
		}
	}
}

/*
>>	You can play a preprocessed handle with the
>>	following two routines. Read the manual for
>>	more information.
*/
void	SKPlayHandleA(thehand,priority)
register	Handle	thehand;
register	int		priority;
{
	if(Vv.volume)
	if(priority >= Vv.PriA || Vv.CountA==0)
	{	Vv.CountA=0;
		Vv.ChanA=*thehand+sizeof(long);
		Vv.CountA=*(long *)*thehand;
		Vv.PriA=priority;
	}
	else
	{	if(Vv.CountB==0)
		{	Vv.CountB=0;
			Vv.ChanB=*thehand+sizeof(long);
			Vv.CountB=*(long *)*thehand;
			Vv.PriB=0;
		}
	}
}
void	SKPlayHandleB(thehand,priority)
register	Handle	thehand;
register	int		priority;
{
	if(Vv.volume)
	if(priority >= Vv.PriB || Vv.CountB==0)
	{	Vv.CountB=0;
		Vv.ChanB=*thehand+sizeof(long);
		Vv.CountB=*(long *)*thehand;
		Vv.PriB=priority;
	}
	else
	{	if(Vv.CountA==0)
		{	Vv.CountA=0;
			Vv.ChanA=*thehand+sizeof(long);
			Vv.CountA=*(long *)*thehand;
			Vv.PriA=0;
		}
	}
}
