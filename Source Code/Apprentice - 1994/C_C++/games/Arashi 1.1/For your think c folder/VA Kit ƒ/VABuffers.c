/*/
     Project Arashi: VABuffers.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:41
     Created: Thursday, February 9, 1989, 15:21

     Copyright � 1989-1992, Juri Munkki
/*/

/*
**	In addition to the VAInit and VAClose calls,
**	this files contains routines that manage the
**	double buffering of the vector animation toolkit.
*/

#define VECTOR_ANIMATION_MAIN
#define	 _VABUFFERS_
#include <Palette.h>
#include <Retrace.h>
#include <Slots.h>
#include "VA.h"
#include "VAInternal.h"
#include "Video.h"

#define	NO_VIDEO_BLANK_ROUTINE

extern	ColorSpec	*VASpec[2];		/*	VA color tables.	*/

/*
>>	If A5 points to a VideoVars structure, the following macro will become
>>	xx(A5), where xx is the offset into that structure element.
*/
#define VIDB(field)		((int) &((VideoVars *) 0)->field)(A5)

typedef	struct
{
	VBLTask				VBL;		/*	VBLTask entry								*/
	int					theslot;	/*	Slot number									*/
	int					called;		/*	A flag										*/
	long				RealA5;		/*	A5 saved here.								*/
	int					changecolors;
	VDSetEntryRecord	colorange;
	CntrlParam			coloparam;

}	VideoVars;

static	VideoVars	VideoBlank;

#ifdef _UNUSED_CODE
/*
>>	SetUpVBLTask contains and sets up a VBL task that
>>	changes the color lookup table to contain the current
>>	set of colors. This code was used only for testing
>>	a theory of mine and it caused more trouble than
>>	it was good for.
*/
void		VASetUpVideoTask(Display)
GDHandle	Display;
{
	AuxDCEHandle	DisplayDriver;
	
	long	vbltask;

	asm	{
		lea		@myvbltask,A0		;	Get address of vbltask
		move.l	A0,vbltask			;	Store in local variable
		lea		@mybase,A0			;	Get addr of variable base storage
		lea		VideoBlank,A1			;	Get addr of VideoBlank record
		move.l	A1,(A0)				;	Store base in base storage
		move.l	A5,VideoBlank.RealA5
		}

	VideoBlank.VBL.qType=vType;				/*	Vertical blanking queue.	*/
	VideoBlank.VBL.vblAddr=(ProcPtr)vbltask;/*	Address of task				*/
	VideoBlank.VBL.vblCount=1;				/*	Every tick					*/
	VideoBlank.VBL.vblPhase=0;				/*	0 is ok..					*/

	DisplayDriver=(void *)GetDCtlEntry((*Display)->gdRefNum);
	VideoBlank.theslot=(*DisplayDriver)->dCtlSlot;
	SlotVInstall(&VideoBlank.VBL,VideoBlank.theslot);	/*	Use main display VBL queue.	*/
	return;
	
/*
>>	This is the actual VBL task that gets executed.
>>	It just increments a counter and sets a flag
>>	every time it's called.
*/
asm	{
@myvbltask
		move.l	A5,-(SP)				;	Save A5
		move.l	@mybase,A5				;	Get VideoBlank address into A5
		move.w	#1,VIDB(VBL.vblCount)	;	Call again.
		move.w	#1,VIDB(called)			;	Flag screen blank
		
		move.w	VIDB(changecolors),D0
		beq.s	@donothing

		clr.w	VIDB(changecolors)		
		lea		VIDB(coloparam),A0
		_PBControl	IMMED
		
@donothing
		move.l	(sp)+,A5
		rts
@mybase
		dc.l	0						;	VideoBlank record address stored here.

	}
}
#endif

/*
>>	VADoFrame is called when the vector animation
>>	kit is not "late" and should draw new lines and
>>	points on the screen. After drawing, it switches
>>	display buffers and then erases the now invisible
>>	old graphics.
*/
void	VADoFrame()
{
	register	int		i,erasedbuffer;
				int		oldcolor;
	register	Rect	*p;
	register	int		*c;
				int		mode;

	
	oldcolor=VA.color;

	VA.field=3;

	if(VA.visbuffer)	VA.offset=2;
		else			VA.offset=5;

	mode=QD32COMPATIBLE;

	erasedbuffer = VA.curbuffer-2;
	if(erasedbuffer<0) erasedbuffer+=3;

	VAInsertExplosions();

	SwapMMUMode(&mode);
	VA.SkipFrames = VA.numlines[erasedbuffer];
	VAEraseBufferedLines(	VA.numlines[erasedbuffer],	VA.lines[erasedbuffer]);
	VAErasePixels(			VA.numpix[erasedbuffer],	VA.pix[erasedbuffer]);
	VAEraseSpots(			VA.numspots[erasedbuffer],	VA.spots[erasedbuffer]);

	VAPlotSpots(VA.numspots[VA.curbuffer],
				VA.spots[VA.curbuffer],
				VA.spotcolors[VA.curbuffer]);

	VADrawBufferedLines(VA.numlines[VA.curbuffer],
						VA.lines[VA.curbuffer],
						VA.linecolors[VA.curbuffer]);

	VAPlotPixels(VA.numpix[VA.curbuffer],
				VA.pix[VA.curbuffer],
				VA.pixcolors[VA.curbuffer]);
	SwapMMUMode(&mode);

#ifdef	OLDSETENTRIES
	SetGDevice(VA.device);
	SetEntries(0,255,VASpec[VA.visbuffer]);
#else

	VideoBlank.colorange.csTable=VASpec[VA.visbuffer];
	VideoBlank.colorange.csStart=0;
	VideoBlank.colorange.csCount=255;

	*(void **)(&VideoBlank.coloparam.csParam)= &VideoBlank.colorange;
	VideoBlank.coloparam.ioCompletion=0;
	VideoBlank.coloparam.ioCRefNum=VA.refnum;
	VideoBlank.coloparam.csCode=3;
	
#ifdef _UNUSED_CODE
	VideoBlank.changecolors = 1;
	while(VideoBlank.changecolors);
#endif
	
	PBControl(&VideoBlank.coloparam,0);

#endif

	VA.numlines[erasedbuffer]=0;
	VA.numpix[erasedbuffer]=0;
	VA.numspots[erasedbuffer]=0;

	VA.curbuffer = erasedbuffer;
	VA.visbuffer = 1-VA.visbuffer;

	VA.color=oldcolor;
}

/*
>>	VACancelFrame is called when the kit is late
>>	and it has not time to draw new graphics. Any
>>	new buffered drawing commands are just flushed
>>	away and no buffer switching is done. Note that
>>	explosions have to be advanced one frame or they
>>	will slow down.
*/
void	VACancelFrame()
{
	VAInsertExplosions();
	VA.numlines[VA.curbuffer]=0;
	VA.numpix[VA.curbuffer]=0;
	VA.numspots[VA.curbuffer]=0;
}

CTabHandle	SystemCTab;	/*	Used for restoring colors when STORM is done.	*/

/*
>>	Start up the vector animation kit with VAInit.
*/
void	VAInit(GDev)
GDHandle	GDev;
{
	register	int				i,j;
	register	PixMapPtr		Pix;
	register	Ptr				quickp;
	register	GDHandle		savedgd;
				int				reqlist[257];
				int				zero=0;

	while((*(*GDev)->gdPMap)->pixelSize != 8)
	{	i=Alert(200,0);
		if(i==2)	ExitToShell();
	}
	HideCursor();

	/*	Save color table for display device we are going to use:	*/
	savedgd=GetGDevice();
	SetGDevice(GDev);
	SystemCTab=(CTabHandle)NewHandle(0);
	for(i=0;i<256;i++)
	{	reqlist[i+1]=i;
	}
	reqlist[0]=255;
	SaveEntries(0,SystemCTab,reqlist);
	VAInstallQDSearch();
	SetGDevice(savedgd);

	/*	Open a window that covers our device as completely as possible:	*/
	VA.window=(WindowPtr)NewCWindow(0,&(*GDev)->gdRect,&zero,-1,plainDBox,(void *)-1,0,0);
	RectRgn(VA.window->visRgn,&VA.window->portRect);
	SetPort(VA.window);
	
	/*	Set up an explicit palette for direct color indexing:			*/
	VA.palette=(Handle)NewPalette(256,0,pmExplicit,0);
	SetPalette(VA.window,VApalette,0);

	/*	Initialize some important global variables:						*/
	Pix=*(*GDev)->gdPMap;
	VA.base=Pix->baseAddr;
	VA.row=Pix->rowBytes & 0x3FFF;
	VA.frame=VA.window->portRect;
	VA.DisplayFrame=(*GDev)->gdRect;
	VA.device=(Handle)GDev;
	VA.refnum=(*GDev)->gdRefNum;

	quickp=VA.base;
		
	VA.quickrow=(Ptr *)NewPtr(VA.frame.bottom*sizeof(Ptr));
	for(i=0;i<VA.frame.bottom;i++)
	{	VA.quickrow[i]=quickp;
		quickp+=VA.row;
	}

	VA.ticker=QuickTicks+3;	/*	QuickTicks is just an alias for TickCount()	*/
	VA.field=3;
	VA.offset=2;

	/*	Allocate space for display lists etc. (double buffered)			*/
	for(i=0;i<NUMDISPLISTS;i++)
	{	VASpec[i]		= (void *) NewPtr(sizeof(ColorSpec)	* 256);
		VA.lines[i]		= (void *) NewPtr(sizeof(Rect)		* MAXSAVED);
		VA.linecolors[i]= (void *) NewPtr(sizeof(int)		* MAXSAVED);
		VA.pixcolors[i] = (void *) NewPtr(sizeof(int)		* MAXSAVEDPIX);
		VA.pix[i]		= (void *) NewPtr(sizeof(Ptr)		* MAXSAVEDPIX);
		VA.spotcolors[i]= (void *) NewPtr(sizeof(int)		* MAXSAVEDPIX);
		VA.spots[i]		= (void *) NewPtr(sizeof(Ptr)		* MAXSAVEDPIX);
		VA.numlines[i]=0;
		VA.numpix[i]=0;
		VA.numspots[i]=0;
	}
	VACreateColorMapping();

	
	VAEraseBuffer();
	
#ifdef	VIDEO_BLANK_ROUTINE
	VASetUpVideoTask(GDev);
#endif
	VASetColors(GetResource('CLOT',1000));
	VA.curbuffer=0;
	VA.visbuffer=1;
	VA.FrameSpeed=DEFAULT_FRAMESPEED;
	VALoadExplosions();
	VAInitSins();
	VADoFrame();
}

/*
>>	Call VARestoreColors to temporarily
>>	restore the color environment that
>>	existed before the vector animation
>>	kit took control.
*/
void	VARestoreColors()
{
	register	GDHandle	savedgd;
	register	int			i;
				int			reqlist[257];

	for(i=0;i<256;i++)
	{	reqlist[i+1]=i;
	}
	reqlist[0]=255;

	savedgd=GetGDevice();
	SetGDevice(VA.device);

	RestoreEntries(SystemCTab,0,reqlist);
	VARemoveQDSearch();

	SetGDevice(savedgd);
}
/*
>>	VAClose restores the old color environment, but doesn't
>>	do much else. It doesn't free any of the storage that
>>	was previously aquired, although it probably should...
>>
>>	Maybe some day I'll bother to write a proper VAClose.
*/
void	VAClose()
{

#ifdef	VIDEO_BLANK_ROUTINE
	SlotVRemove(&VideoBlank.VBL,VideoBlank.theslot);
#endif

	VARestoreColors();
	ShowCursor();
}

/*
>>	VAStep performs one animation step.
>>	There's still a busy waiting loop in
>>	here, but usually it's not used much.
*/
void	VAStep()
{
	VA.FrameCounter++;
	
	if(VA.Late)
	{	VACancelFrame();
		VA.SkipFrames++;
		VA.SkipCounter++;
	}
	else
	{	VADoFrame();
		VA.SkipCounter=0;
	}

	/*	Determine if we are late:
	*/
	VA.ticker+=VA.FrameSpeed;
	VA.Late=VA.ticker<=QuickTicks;
	if(VA.SkipCounter>=4)
		VA.Late=0;	/*	Don't skip more than 4 frames in a row.	*/

	/*	If we are ahead more than 4 ticks, loop for a while.
	*/
	while(VA.ticker>QuickTicks+4);
}

/*
>>	Call VACatchUp when you have stopped
>>	using VAStep for a while. If you do
>>	a lengthy drawing without calling
>>	VAStep regularly, the animation kit
>>	will think that it is badly late and
>>	the animation will skip wildly. Calling
>>	VACatchUp before resuming the animation
>>	will solve this problem.
*/
void	VACatchUp()
{
	VA.SkipCounter=0;
	VA.Late=0;
	VA.ticker=QuickTicks+VA.FrameSpeed;
}