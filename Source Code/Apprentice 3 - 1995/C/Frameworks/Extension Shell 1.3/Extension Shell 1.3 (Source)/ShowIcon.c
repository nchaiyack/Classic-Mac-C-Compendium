/*	NAME:
		ShowIcon.c

	WRITTEN BY:
		Various... see description.

	MODIFIED BY:
		Dair Grant
				
	DESCRIPTION:
		This code is based variously on James Walker's ShowIcon7.c code, and Patrick
		Beard's ShowIconFamily code. ShowIconFamily was itself based on the original
		ShowInit - by Paul Mercer, Darin Adler, and Paul Snively, from an idea by
		Steve Capps.

		This is the de facto standard used by all Mac INITs.

	NOTES:
		¥	Compiled with THINK C 6.0.

		¥	We use the System 7 IconSuite calls to display the appropriate icon
			from our INIT code's icon family. This looks after bit depths, and
			removes the need for 'cicn's for colour displays.
		
		¥	If we're not running under System 7, we can only handle 'ICN#'
			resources.
			
		¥	Make sure you have to most up-to-date copy of <Icons.h>. The IconSuite
			routines were missing from some older versions of THINK C.
			
	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			¥	First publicly distributed version.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <Icons.h>
#include "ESConstants.h"
#include "ShowIcon.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
void		GetIconRect(Rect *theRect, Rect *thePortRect);
void		NextPosition(void);
short		CheckSum(register short x);
void		PlotBWIcon(Rect *iconRect, Handle theIcon);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
// ShowInit's information is nestled at the tail end of CurApName.
// It consists of a short which encodes the next horizontal offset,
// and another short which is that value checksummed.
extern  short  gCurrX		: 0x92C;			// CurApName + 28
extern  short  gTheCheckSum	: 0x92E;			// CurApName + 30





//=============================================================================
//		Private defines																	 
//-----------------------------------------------------------------------------
#define kCheckSumConst		0x1021			// Check-sum constant
#define kInitialXPosition	8				// First horizontal position
#define kYOffset			40				// Amount icons advance by vertically
#define kXOffset			40				// Amount icons advance by horizontally
#define kIconDimension		32				// Width/Height of an icon










//=============================================================================
//		PlotINITIcon : Plot a series of icons on the screen.																 
//-----------------------------------------------------------------------------
//		Note :	We are passed an array of icon resource IDs, the number of
//				valid IDs there are in the array, the number of ticks to wait
//				between showing each icon, and a status flag for the presence
//				of System 7.
//
//				Usually Extensions will pass in only one icon, but this allows
//				us to support animated icons as well.
//
//				We use System 7's IconSuite routines to plot the icons. These
//				look after all the details of deciding what icon to use,
//				depending on the depth of the display. They also work using
//				the same icon families that the Finder uses, which means
//				there's no need for a 'cicn' version.
//
//				If we don't have System 7, we don't have access to the
//				IconSuite routines, and so have to resort to plotting 'ICN#'
//				resources.
//-----------------------------------------------------------------------------
void PlotINITIcon(Boolean haveSys7, int animDelay, int numIcons, int (*theIcons)[])
{	CGrafPtr	oldPort;
	GrafPort	newPort;
	Handle		theIconHnds[kMaxNumIcons+1];
	Rect 		theIconPos;
	long		theTicks;
	int			i;




	// Save the port, and open a new one
	GetPort(&oldPort);
	OpenCPort(&newPort);
	SetPort(&newPort);



	// Work out where we should draw the icon
	GetIconRect(&theIconPos, &newPort.portRect);



	// Read in handles to as many icons as we have to/can. If we Get/Plot/Dispose
	// of each icon in turn, we can get jerky animation.
	for (i = 1; i <= numIcons && i <= kMaxNumIcons; i++)
		{
		if (haveSys7)
			GetIconSuite(&theIconHnds[i], (*theIcons)[i], svAllLargeData);
		else
			theIconHnds[i] = GetResource('ICN#', (*theIcons)[i]);
		}
	
		

	// Plot all the icons, with the right delay
	for (i = 1; i <= numIcons && i <= kMaxNumIcons; i++)
		{
		if (haveSys7)
			PlotIconSuite(&theIconPos, atNone, ttNone, theIconHnds[i]);
		else
			PlotBWIcon(&theIconPos, theIconHnds[i]);
		Delay(animDelay, &theTicks);
		}
	
	
	
	// Before releasing them again
	for (i = 1; i <= numIcons && i <= kMaxNumIcons; i++)
		{
		if (haveSys7)
			DisposeIconSuite(theIconHnds[i], true);
		else
			ReleaseResource(theIconHnds[i]);
		}


	
	// Set things up for the next INIT's icon - but only if we
	// actually showed anything
	if (numIcons > 0)
		NextPosition();



	// Dispose of our temporary port, and restore the old one
	CloseCPort(&newPort);
	SetPort(oldPort);
}










//=============================================================================
//		GetIconRect : Calculate the correct position for our INIT's icon.																 
//-----------------------------------------------------------------------------
//		Note :	We leave the correct position in theRect. We are given the
//				portRect field of the current graphics port in thePortRect.
//
//				*theRect is forced to be onscreen. This is done by taking
//				the horizontal offset modulo the screen width to generate the
//				horizontal position of the icon, and the offset divided by the
//				screen width to generate the proper row. This mechanism can get
//				messed up if people plot icons at non-standard offsets.
//
//				We are also responsible for initialising the ShowInitIcon
//				mechanism if we're the first INIT to load.
//-----------------------------------------------------------------------------
void GetIconRect(Rect *theRect, Rect *thePortRect)
{	short	screenWid;




	// If we're the first INIT to run we need to initialize the horizontal value
	if (((gCurrX << 1) ^ kCheckSumConst) != gTheCheckSum)
		gCurrX = kInitialXPosition;



	// Get the width of the screen, making sure we don't run off the edge
	screenWid  = thePortRect->right - thePortRect->left;
	screenWid -= screenWid % kXOffset;
	
	
	
	// Work out where the rectangle is going to be
	theRect->left	= (gCurrX % screenWid);
	theRect->right	= theRect->left + kIconDimension;
	theRect->top	= thePortRect->bottom - (kYOffset * (1 + gCurrX / screenWid));
	theRect->bottom	= theRect->top  + kIconDimension;
}










//=============================================================================
//		NextPosition : Advance the ShowIcon position for the next INIT.																 
//-----------------------------------------------------------------------------
//		Note :	In Patrick Beard's original version, this was done at the end
//				of GetIconRect. That caused incorrect behaviour when IconWrap
//				1.2 was used to wrap icons.
//
//				If an INIT using that version of ShowIconFamily was the first
//				in a row, then the second icon in that row would land on top of
//				it.
//-----------------------------------------------------------------------------
void NextPosition(void)
{




	// Advance the position for the next icon.
	gCurrX += kXOffset;
	

	
	// Recompute the checksum.
	gTheCheckSum = (gCurrX << 1) ^ kCheckSumConst;
}










//=============================================================================
//		PlotBWIcon : Plot an 'ICN#' icon.																 
//-----------------------------------------------------------------------------
//		Note :	We leave theIcon in the state we found it - unlocked.
//
//				This routine is based on Jim Friedlander's Technical Note,
//				ÒDrawing IconsÓ
//-----------------------------------------------------------------------------
void PlotBWIcon(Rect *iconRect, Handle theIcon)
{	BitMap		src, dest;
	GrafPtr		myPort;




	// If we don't have an icon, there's not much point in plotting it.
	// But, if we do have one, we've got to lock it down.
	if (theIcon == nil)
		return;
	else
		HLock(theIcon);

	
	
	// Prepare the source and destination bitmaps.
	src.baseAddr = *theIcon + 128;							// Offset to Mask.
	src.rowBytes = 4;
	SetRect(&src.bounds, 0, 0, 32, 32);
	GetPort(&myPort);
	dest = ((GrafPtr) myPort)->portBits;

	
	
	// Transfer the mask.
	CopyBits(&src, &dest, &src.bounds, iconRect, srcBic, nil);
	
	
	
	// Followed by the icon.
	src.baseAddr = *theIcon;								/// 0 offset to icon data.
	CopyBits(&src, &dest, &src.bounds, iconRect, srcOr, nil);



	// Unlock the icon.
	HUnlock(theIcon);
}

