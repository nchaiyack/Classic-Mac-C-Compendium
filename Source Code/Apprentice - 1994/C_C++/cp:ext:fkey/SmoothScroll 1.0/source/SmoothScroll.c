//////////////
//
//	SmoothScroll
//
//	by Steve Bushell - May 3, 1993
//	python@world.std.com
//
//	SmoothScroll is an INIT which patches the ScrollRect trap, changing
//	it from an instant, jumpy scroll, to a smooth, flowing scroll.
//	The degree of smoothness is based on a 'cnfg' resource stored in
//	the INIT file itself.  The higher the long int in the 'cnfg' resource,
//	the smoother the scrolling will be.  It is good not to set this number
//	too high because the scrolling, although very smooth, can get very
//	slow.  The patch is also optimized to adjust the scrolling based on
//	the number of pixels it will be moving, and their depth.  The
//	more pixels it will be moving, the more jumpy the scrolling becomes.
//
//	�1993 Steven J. Bushell
//
//////////////

#include <SetUpA4.h>
#include <Traps.h>

#define		Abs(x)			((x) < 0 ? -(x) : (x))

//	Prototype for ScrollRect patch function
static pascal void SmoothScroll(Rect *startRect,
								short hDist,
								short vDist,
								RgnHandle updateRgn);

//	typedef for ScrollRect function pointer
typedef pascal void (*ScrollRectProcPtrType)(const Rect *r,
											short dh,
											short dv,
											RgnHandle updateRgn);

//	prototype or ShowIcon function
void ShowIcon(short resID);

//	globals used in patch
static ScrollRectProcPtrType	origScrollRect;
static long						cnfgMaxBitsPerBlit;
static RgnHandle				clipRgn,tempRgn;
static Boolean					colorQDAvailable;

//	low-memory global which contains state of shift key
extern Byte shiftKeyDown:0x017b;

void main(void)
{
	short		trap,err;
	Handle		ourHandle,cnfgHandle;
	THz			savedZone;
	SysEnvRec	ser;

	//	prepare to use globals
	RememberA0();
	SetUpA4();

	//	if the shift key is down, we should not install ourselves
	if (shiftKeyDown & 0x01U)
		goto InstallAbort;

	//	get a grip on ourselves, and lock ourselves down
	asm {
		move.l a0,ourHandle
	}
	ourHandle = RecoverHandle((Ptr)ourHandle);
	if (!ourHandle)
		goto InstallAbort;
	DetachResource(ourHandle);
	HLock(ourHandle);
	HNoPurge(ourHandle);

	//	read in the configuration setting and store it in global
	cnfgHandle = GetResource('cnfg',128);
	if (!cnfgHandle)
		goto InstallAbort;
	cnfgMaxBitsPerBlit = (**(long **)cnfgHandle);
	ReleaseResource(cnfgHandle);

	//	install the patch
	trap = _ScrollRect & 0x3FF;
	origScrollRect = (ScrollRectProcPtrType)NGetTrapAddress( trap, ToolTrap);
	NSetTrapAddress((long)SmoothScroll, trap, ToolTrap);

	//	create two regions in the System heap for use later
	savedZone = GetZone();
	SetZone(SysZone);
	clipRgn = NewRgn();
	tempRgn = NewRgn();
	SetZone(savedZone);
	
	//	Is Color Quickdraw available?
	err = SysEnvirons(1, &ser);
	colorQDAvailable = err ? false : ser.hasColorQD;

	//	Success!  Show the SmoothScroll icon.
	ShowIcon(128);

	//	return to normal
	RestoreA4();
	return;

InstallAbort:
	//	Something went wrong and the patch installation was aborted!
	//	Show the SmoothScroll icon with the red 'X' through it.
	ShowIcon(129);
	RestoreA4();
}

/////////////
//
//	SmoothScroll
//
//	This is the actual patch to ScrollRect.
//	It works by dividing up a single call to ScrollRect
//	into multiple calls, giving the appearance of smooth
//	motion.
//	Note: this is a severe tail patch, as it calls the original
//	trap several times.  Anyone with any better ideas can go
//	and write their own patch (and of course tell me how they
//	did it)!
//
/////////////
static pascal void SmoothScroll(Rect *startRect,
								short hDist,
								short vDist,
								RgnHandle updateRgn)
{
	short		depth,divisor,hStep,vStep;
	long		totalBits;
	Rect		travelRect,destRect;
	GDHandle	theGDevice;
	GrafPtr		thisPort;

	//	prepare to access our globals
	SetUpA4();

	//	if there is no offset, there is nothing to do,
	//	so we just exit
	if (!hDist && !vDist)
		goto EXIT;

	//	if there is both a horizontal and a vertical offset, things
	//	are much too complicated for us, so we just call the
	//	original trap with the original parameters and get out
	if (hDist && vDist)
	{
	ORIG:
		(origScrollRect)(startRect,hDist,vDist,updateRgn);
		goto EXIT;
	}

	//	find our screen depth
	if (colorQDAvailable)
	{
		destRect = *startRect;
		LocalToGlobal(&(((Point *)&destRect)[0]));
		LocalToGlobal(&(((Point *)&destRect)[1]));
		theGDevice = GetMaxDevice(&destRect);
	
		//	if we can't get our screen depth, we call the
		//	original trap and get out
		if (!theGDevice)
			goto ORIG;
		
		depth = (*(*theGDevice)->gdPMap)->pixelSize;
	}
	else
		depth = 1;

	//	calculate the total number of bits that we're going
	//	to be dealing with in this scrolling sequence
	totalBits = ((long)startRect->bottom-startRect->top) *
				((long)startRect->right-startRect->left) *
				depth;

	//	if there are no bits, there's nothing to do; we just
	//	set update region to nothing and then exit
	if (!totalBits)
	{
		SetEmptyRgn(updateRgn);
		goto EXIT;
	}

	//	based on the configuration setting, we calculate how
	//	many steps we're going to divide the scrolling sequence into
	divisor = cnfgMaxBitsPerBlit / totalBits;

	//	if the result was zero, we must make it at least one step
	if (!divisor)
		divisor = 1;

	//	the the current port so we can access its fields
	GetPort(&thisPort);

	//	set up the destination rect for scrolling
	destRect = *startRect;
	OffsetRect(&destRect,hDist,vDist);

	//	set up clipping area, and calculate the region that will
	//	be returned as the last parameter to the function call
	GetClip(clipRgn);
	ClipRect(startRect);
	RectRgn(updateRgn,startRect);
	SectRgn(thisPort->visRgn,updateRgn,tempRgn);
	CopyRgn(tempRgn,updateRgn);
	OffsetRgn(tempRgn,hDist,vDist);
	XorRgn(tempRgn,updateRgn,updateRgn);
	SectRgn(thisPort->clipRgn,updateRgn,updateRgn);

	//	do horizontal scrolling
	if (hDist)
	{
		travelRect = *startRect;
		//	calculate number of pixels in each step
		hStep = hDist / divisor;
		//	if the result is zero, it is reset to a unit pixel
		if (!hStep)
			hStep = Abs(hDist) / hDist;
		
		//	are we done yet?
		while (travelRect.left != destRect.left)
		{
			//	are we close yet?
			if (Abs(destRect.left - travelRect.left) <= Abs(hStep))
				hStep = destRect.left - travelRect.left;
			//	call original trap with new parameters
			(origScrollRect)(&travelRect,hStep,0,tempRgn);
			//	move to next step
			OffsetRect(&travelRect,hStep,0);
		}
		goto DONE;
	}

	//	do vertical scrolling
	if (vDist)
	{
		travelRect = *startRect;
		//	calculate number of pixels in each step
		vStep = vDist / divisor;
		//	if the result is zero, it is reset to a unit pixel
		if (!vStep)
			vStep = Abs(vDist) / vDist;
		
		//	are we done yet?
		while (travelRect.top != destRect.top)
		{
			//	are we close yet?
			if (Abs(destRect.top - travelRect.top) <= Abs(vStep))
				vStep = destRect.top - travelRect.top;
			//	call original trap with new parameters
			(origScrollRect)(&travelRect,0,vStep,tempRgn);
			//	move to next step
			OffsetRect(&travelRect,0,vStep);
		}
		goto DONE;
	}
DONE:
	//	reset clipping region
	SetClip(clipRgn);
EXIT:
	//	return to normal
	RestoreA4();
}

////////////
//
//	ShowIcon
//
//	A handy little function which displays an icon at startup
//	time.  This is actually just a stub which calls the real code
//	which is kept in a code resource called 'Show'.  It takes
//	a single parameter, the resource ID of the icon family to
//	be displayed.
//
////////////
void ShowIcon(short resID)
{
	typedef void (*ShowCodeFunc)(short);
	Handle	ShowCode = 0L;

	ShowCode = GetResource('Show',-4064);
	if (!ShowCode)
		return;

	HLock(ShowCode);
	
	((ShowCodeFunc)*ShowCode)(resID);
	
	HUnlock(ShowCode);
	ReleaseResource(ShowCode);
}