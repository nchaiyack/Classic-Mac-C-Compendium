/*	
	This fader shell serves to dispatch requests from the main DarkSide code
	into the appropriate entry points.
	
	Written by Tom Downdy.
	
	Modified for THINK C by Fran�ois Pottier, October 1993.
	If you should find any errors in this code, please drop me a mail at pottier@dmi.ens.fr
*/

#include <StdArg.h>
#include <Memory.h>
#include <Packages.h>
#include <Errors.h>

#include "Fader.h"
#include <SetupA4.h>


OSErr	main(long selector, Ptr *a5World, MachineInfoPtr machineInfo, ...)
{
	OSErr	anErr;
	va_list	nextArg;
		
	// Set up A4 so that we can use global variables...
	RememberA0();
	SetUpA4();
		
	// start stripping optional arguments
	va_start(nextArg, machineInfo);
	
	switch(selector)
		{
		case preflightFader:
			{
			long	*minTicks, *maxTicks;
			
			minTicks = va_arg(nextArg, long*);
			maxTicks = va_arg(nextArg, long*);
			
			anErr = PreflightFader(machineInfo, minTicks, maxTicks);
			break;
			}			
		case initializeFader:
			anErr = InitializeFader(machineInfo);
			break;
			
		case idleFader:
			anErr = IdleFader(machineInfo);
			break;
			
		case disposeFader:
			anErr = DisposeFader(machineInfo);
			break;

		case updateFader:
			anErr = UpdateFader(machineInfo);
			break;
			
		case hitFader:
			{
			DialogPtr	dPtr;
			long		itemHit;
			long		itemOffset;
						
			dPtr 		= va_arg(nextArg, DialogPtr);
			itemHit 	= va_arg(nextArg, long);
			itemOffset 	= va_arg(nextArg, long);

			anErr = HitFader(machineInfo, dPtr, itemHit, itemOffset);			
			}
			
			break;
			
		default:
			// function not found error
			anErr = fnfErr;
			break;
		}
		
	va_end(nextArg);
	RestoreA4();
	return(anErr);	
	
} // FaderEntry

/* ------------------------------------------------------------------------	*/
/* FADER UTILS									 							*/
/* ------------------------------------------------------------------------	*/

Handle	BestNewHandle(Size theSize)
/*
	Tries to get the handle from the temp memory first, if that fails, it goes
	to the application.
*/
{
	Handle theHandle;
	OSErr	anErr;
	
	theHandle = TempNewHandle(theSize, &anErr);
	if (theHandle == nil)
		theHandle = NewHandle(theSize);
		
	return(theHandle);
	
} // BestNewHandle

/* ------------------------------------------------------------------------	*/

RgnHandle	BestNewRgn()
/*
	Tries to get a rgn handle from the temp memory first, if that fails, it goes
	to the application.  Needs enough room in the app heap to create the region
	in the first place.
*/
{
	RgnHandle 	theRgn;
	OSErr		anErr;
	
	// make a region
	theRgn = NewRgn();
	if (theRgn != nil)
		{
		RgnHandle	theHandle;
		short		regionSize;
		
		// try to make something the same size in the temp memory
		regionSize = GetHandleSize((Handle) theRgn);
		theHandle = (RgnHandle) TempNewHandle(regionSize, &anErr);
		if (anErr == noErr)
			{
			// if we get it, use that one for our region
			BlockMove(*theRgn, *theHandle, regionSize);
			DisposeRgn(theRgn);
			theRgn = theHandle;
			}
		}
		
	return(theRgn);
	
} // BestNewRgn



/* ------------------------------------------------------------------------	*/

short	Rnd(long max)
/*
	Returns a number > 0 and < max
*/
{
unsigned long value;

	value = (unsigned short)max * (unsigned short)Random();
	value >>= 16;
	return(value);
	
} // Rnd

/* ------------------------------------------------------------------------	*/
void PlaceRectOnScreen(
	MachineInfoPtr machineInfo,	// give info about the machine here
	short width,				// width of rect, can be 0
	short height,				// height of rect, can be 0
	Rect * placedRect,			// Placed rect is returned here
	Rect * margins,				// margins around screen, can be nil
	short * whichScreen)		// screen index returned here, can be nil
{
	Rect		screenRect;
	short		pickScreen;

	// pick a random screen	
	pickScreen = Rnd(machineInfo->numScreens);
	screenRect = machineInfo->theScreens[pickScreen].bounds;
	if (whichScreen != nil)
		*whichScreen = pickScreen;
		
	if (margins != nil)
		{
		screenRect.top += margins->top;
		screenRect.left += margins->left;
		screenRect.bottom -= margins->bottom;
		screenRect.right -= margins->right;
		}
		
	screenRect.right -= width;
	screenRect.bottom -= height;
	
	if (placedRect != nil)
		{
		placedRect->top = screenRect.top + Rnd(screenRect.bottom - screenRect.top);
		placedRect->left = screenRect.left + Rnd(screenRect.right - screenRect.left);
		placedRect->bottom = placedRect->top + height;
		placedRect->right = placedRect->left + width;
		}
		
} // PlaceRectOnScreen

/* ------------------------------------------------------------------------	*/
/* CALLBACK WRAPPERS
/* ------------------------------------------------------------------------	*/
OSErr	WritePreferencesHandle(MachineInfoPtr machineInfo, Handle h, ResType theType)
{
	OSErr	anErr;
	long	curA5;
	long	loader;
	
	curA5 = SetA5(machineInfo->applicationA5);
	loader = (long) machineInfo->callbackLoader;
	asm {
		move.l theType, -(SP)
		move.l h, -(SP)
		move.l loader, a0
		moveq #0, d0
		pea @returnAddress
		move.l #0x08000004, -(SP)
		jmp (a0)
	@returnAddress:
		move.w d0, anErr
		add #8, SP
	}
	SetA5(curA5);
	return(anErr);
	
} // WritePreferencesHandle

/* ------------------------------------------------------------------------	*/
OSErr	ReadPreferencesHandle(MachineInfoPtr machineInfo, Handle *h, ResType theType)
{
	OSErr	anErr;
	long	curA5;
	long	loader;
	
	curA5 = SetA5(machineInfo->applicationA5);
	loader = (long) machineInfo->callbackLoader;
	asm {
		move.l theType, -(SP)
		move.l h, -(SP)
		move.l loader, a0
		moveq #0, d0
		pea @returnAddress
		move.l #0x08000008, -(SP)
		jmp (a0)
	@returnAddress:
		move.w d0, anErr
		add #8, SP
	}
	SetA5(curA5);
	return(anErr);
	
} // ReadPreferencesHandle

/* ------------------------------------------------------------------------	*/
// While translating this code from MPW C to THINK C I found out that the calling conventions
// of these languages seem to be different. If we have a short and a Boolean as below, THINK C
// pushes them both into a longword, while MPW C uses a longword for each. Very confusing!

OSErr	PlayResourceSnd(MachineInfoPtr machineInfo, short theID, Boolean async)
{
	OSErr	anErr;
	long	loader;
	
	loader = (long) machineInfo->callbackLoader;
	asm {
		moveq #0, d0
		move.b async, d0								// convert async to longword
		move.l d0, -(SP)								// and push it

		moveq #0, d0									// convert theID to longword
		move.w theID, d0								// and push it
		move.l d0, -(SP)
		
		move.l loader, a0
		moveq #0, d0
		pea @returnAddress								// push return address
		move.l #0x0800000C, -(SP)						// push selector code
		jmp (a0)										// jump to callback loader
	@returnAddress:
		move.w d0, anErr								// save result code								
		add #8, SP										// adjust stack
	}
	return anErr;
	
} // PlayResourceSnd

