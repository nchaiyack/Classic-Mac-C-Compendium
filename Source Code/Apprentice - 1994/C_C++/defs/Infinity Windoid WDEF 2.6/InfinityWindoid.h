// *****************************************************************************
//
//	FILE:
//		InfinityWindoid.h
//
//	DESCRIPTION:
//		This file contains headers that can be used by an application using
//		the Infinity Windoid WDEF.
//
//		In order to create a window using the Infinity Windoid WDEF, the
//		following constants are useful:
//			kInfinityWindoidProc
//			kVerticalTitlebarProc
//			zoomDocProc				(defined in Windows.h)
//
//		For instance, if you want a new windoid with the titlebar down the
//		left side of the window and no zoom box:
//
//		  theWindow = NewWindow(&storage, bounds, title, visible, 
//								kInfinityWindoidProc + kVerticalTitlebarProc, 
//								behind, goAwayFlag, refCon);
//
//		The goAwayFlag will determine if the windoid has a close box.
//		If a zoom box is desired, just add zoomDocProc to theProc.
//
// *****************************************************************************

#define kInfinityWindoidID		128
	//	If you change the ID of the WDEF that is created, put that ID here.
	
#define kInfinityWindoidProc	(kInfinityWindoidID * 16)
	//	This is the base number to use for the procID of a window that uses
	//	the Infinity Windoid WDEF.

#define kVerticalTitlebarProc	2
	//	To have the titlebar be down the side of the window, add this constant.

// *****************************************************************************
