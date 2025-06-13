/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:		PopupMenu.h

Description:
Routines to handle pop-up menus in dialogs.

------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	930228	[esp]	Created
	930611	[esp]	Radical rewrite to allow multiple popups in a dialog
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================*/

#if !defined(POPUPMENU_H)
#define POPUPMENU_H


#include <menus.h>		// MenuHandle
#include <dialogs.h>	// DialogPtr


// =====================================================================

typedef struct
{
	Handle			fNext;			// next item on list
	// You must fill these in...
	DialogPtr		fParentDialog;	// our dialog
	short			fMenuID;		// our popUp menu ID
	short			fPopupItemID;	// our popUp dialog user item ID
	short			fLastChoice;	// the last-chosen item from the pop-up menu
	// AddPopupToList will fill in the rest
	Rect			fTitleBounds;	// bounding box of popup menu title
	Rect			fPopupBounds;	// bounding box of popup menu itself
	MenuHandle		fMenuHandle;	// our popUp menu
} popupRec_t, *popupRecPtr_t, **popupRecHdl_t;


// =====================================================================

void InitPopups(void);

Boolean PopupsExist(void);

void AddPopupToList(popupRecPtr_t newPopupRecP);

short GetPopupValue(short pPopupItemID);

void KillPopups(void);

pascal Boolean PopupMouseDnDlgFilterProc(DialogPtr pDialogPtr, EventRecord *pEventPtr, short *pItemHitPtr);


#endif // POPUPMENU_H
