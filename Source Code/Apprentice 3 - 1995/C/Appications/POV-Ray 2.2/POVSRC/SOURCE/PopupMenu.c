/*==============================================================================
Project:	POV-Ray

Version:	2.2

File:		PopupMenu.h

Description:
Routines to handle pop-up menus in dialogs.  Long-hand way to do it,
so it works in System 6 and System 7.

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
	931119	[djh]	2.0.1 source conditionally compiles for PPC machines, keyword __powerc
==============================================================================*/

#define POPUPMENU_C

#include <types.h>
#include <stdlib.h>	// malloc
#include <memory.h>	// NewPtr

#if defined (__powerc)
#include "PovMac.h"	// for routine descriptor with global scope
#endif // __powerc
#include "PopupMenu.h"

#if defined(applec)
#include <Strings.h>		// p<->cstr
#endif // applec
#include <Fonts.h>			// checkMark
#include <Resources.h>		// ReleaseResource


// constants for positioning the default popup item within its box
#define	DOWNTRI_WIDTH	12		// room on right of popup for down triangle
#define	SLOP_LEFT		13		// leave this much space on left of title
	

// =====================================================================

// Currently active list of popup items in current dialog
static popupRecHdl_t	gCurrentListRoot;


// =====================================================================


//-----------------------------------------------------------
static popupRecHdl_t FindPopupRec(short popupItemID)
{
	popupRecHdl_t	foundItem = NULL;
	popupRecHdl_t	searchItem = gCurrentListRoot;

	// traverse the list, looking for a match of dlg item #s
	while (searchItem && !foundItem)
	{
		if ((**searchItem).fPopupItemID == popupItemID)
			foundItem = searchItem;
		else
			searchItem = (popupRecHdl_t)(**searchItem).fNext;
	}
	return(foundItem);
} // FindPopupRec


//-----------------------------------------------------------
static void DrawDownTriangle(register short h, register short v)
{
	register short i;

	for (i = 0; i < (DOWNTRI_WIDTH/2); ++i)	{
		MoveTo(h + (DOWNTRI_WIDTH/2)-1 - i, v - i);
		Line(2*i, 0);
	}
}


//-----------------------------------------------------------
// Draw the popup menu (called by Dialog Mgr for updates, and by our filterproc)
pascal void DrawPopupMenu(DialogPtr pDialogPtr, short pItem)
{
#pragma unused (pDialogPtr,pItem)
	popupRecHdl_t	popupItemH;
	popupRec_t		thePopupRec;
	short			itemWidth;
	short			itemWidthNew;
	short			itemLength;
	short			theItemType;
	Handle			theItemH;
	Rect			theItemRect;
	StringPtr		textPtr;
	PenState		savePen;
	FontInfo		fontInfo;
	Str255			theItemText;

	// find the appropriate popup item from list, given dialog item ID
	popupItemH = FindPopupRec(pItem);

	if (popupItemH)
	{
		// use a temporary to rid us of the floating handle problem
		thePopupRec = **popupItemH;

		GetPenState(&savePen);

		// get popup item box
		GetDItem(thePopupRec.fParentDialog, thePopupRec.fPopupItemID,
				&theItemType, &theItemH, &theItemRect);

		// get the menu title
		textPtr = (StringPtr)(**(thePopupRec.fMenuHandle)).menuData;	// Menu title

		// need to know font size for later
		GetFontInfo(&fontInfo);

		// Draw the menu title off the left of popup box
		MoveTo(theItemRect.left-StringWidth(textPtr)-2, theItemRect.top+fontInfo.ascent);
		DrawString(textPtr);

		// Get the currently chosen popup item text
		GetItem(thePopupRec.fMenuHandle, thePopupRec.fLastChoice, theItemText);

		// Insure that the item fits. Truncate it and add an ellipses (“…”) if it doesn’t
		itemWidth = (theItemRect.right-theItemRect.left) - (CharWidth(checkMark)+DOWNTRI_WIDTH+4); // available string area
		itemWidthNew = StringWidth(theItemText); // get current width
		if (itemWidthNew > itemWidth)
		{	// doesn't fit - truncate it
			itemLength = theItemText[0]; // current length in characters
			itemWidth = itemWidth - CharWidth('…'); // subtract width of ellipses
		
			do	{ // until it fits (or we run out of characters)
				// drop the last character and its width
				itemWidthNew -= CharWidth(theItemText[itemLength]);
				itemLength--;
			} while ((itemWidthNew > itemWidth) && (itemLength > 0));
		
			// add the ellipses character
			itemLength++; // add room for elipsis character
			theItemText[itemLength] = '…';
			theItemText[0] = itemLength; // set the new true length
		}

		// draw the box
		PenSize(1, 1);
		FrameRect(&theItemRect);
		// and its drop shadow
		MoveTo(theItemRect.right, theItemRect.top+2);
		LineTo(theItemRect.right, theItemRect.bottom);
		LineTo(theItemRect.left+2, theItemRect.bottom);

		// draw the string
		MoveTo(theItemRect.left+CharWidth(checkMark)+2, theItemRect.top+fontInfo.ascent);
		DrawString(theItemText);

		DrawDownTriangle(theItemRect.right-1-DOWNTRI_WIDTH, theItemRect.top+fontInfo.ascent);

		SetPenState(&savePen);
	} // if (popupItemH)

} // DrawPopupMenu


//-----------------------------------------------------------
void InitPopups(void)
{
	gCurrentListRoot = NULL;
} // InitPopups


//-----------------------------------------------------------
Boolean PopupsExist(void)
{
	return(gCurrentListRoot != NULL);
} // PopupsExist


//-----------------------------------------------------------
static OSErr FillInPopupInfo(popupRecHdl_t newItemH)
{
	OSErr			anError = noErr;
	short			itemWidth;
	short			theItemType;
	Handle			theItemH;
	Rect			theItemRect;
	StringPtr		textPtr;
	FontInfo		fontInfo;

	// we're about to dereference the heck out of this...
	HLock((Handle)newItemH);

	SetPort((**newItemH).fParentDialog);

	// get the menu & attach to our rec
	(**newItemH).fMenuHandle = GetMenu((**newItemH).fMenuID); // our popUp menu
	if ((**newItemH).fMenuHandle == NULL)
		anError = ResError();
	else
	{
		// checkmark the current item
		SetItemMark((**newItemH).fMenuHandle, (**newItemH).fLastChoice, checkMark);

		// get dialog item & adjust its rectangle
		GetDItem((**newItemH).fParentDialog, (**newItemH).fPopupItemID,
				&theItemType, &theItemH, &theItemRect);
		CalcMenuSize((**newItemH).fMenuHandle);
		itemWidth = (**((**newItemH).fMenuHandle)).menuWidth;
		itemWidth += DOWNTRI_WIDTH + 4;
		if (itemWidth < (theItemRect.right - theItemRect.left))
			theItemRect.right = theItemRect.left + itemWidth;
		GetFontInfo(&fontInfo);
		theItemRect.bottom = theItemRect.top
							+ fontInfo.ascent
							+ fontInfo.descent
							+ fontInfo.leading
							+ 1;

		// remember popup bounds
		(**newItemH).fPopupBounds = theItemRect;

		// set items rectangle & install handler routine (DrawPopupMenu)
#if defined(__powerc)
		SetDItem((**newItemH).fParentDialog, (**newItemH).fPopupItemID,
				theItemType, (Handle)&gPopupMenuRD, &theItemRect);
#else
		SetDItem((**newItemH).fParentDialog, (**newItemH).fPopupItemID,
				theItemType, (Handle)DrawPopupMenu, &theItemRect);
#endif // __powerc
		// now figure out title bounds
		// get the menu title
		textPtr = (StringPtr)(**((**newItemH).fMenuHandle)).menuData;	// Menu title
		theItemRect.right = theItemRect.left - 1;
		theItemRect.left = theItemRect.right - StringWidth(textPtr) - 2;
		(**newItemH).fTitleBounds = theItemRect;
	}

	HUnlock((Handle)newItemH);

	return(anError);

} // FillInPopupInfo


//-----------------------------------------------------------
void AddPopupToList(popupRecPtr_t newPopupRecP)
{
	OSErr			anError = noErr;
	popupRecHdl_t	newItemH;
	popupRecHdl_t	nextItemH;

	// allocate new popup item
	newItemH = (popupRecHdl_t)NewHandle(sizeof(popupRec_t));

	if (newItemH)
	{
		// copy user info into it
		**newItemH = *newPopupRecP;
		// fill in additional info from menu etc.
		anError = FillInPopupInfo(newItemH);
		// remember list contents, append it later
		nextItemH = gCurrentListRoot;
		// point the head of list to our new item
		gCurrentListRoot = newItemH;
		// connect rest of list after our newly inserted item
		(**newItemH).fNext = (Handle)nextItemH;
	}
} // AddPopupToList


//-----------------------------------------------------------
void KillPopups(void)
{
	popupRecHdl_t	trailerH = gCurrentListRoot;

	// walk the list of popup recs & dispose of each one
	while (gCurrentListRoot != NULL)
	{
		trailerH = gCurrentListRoot;
		gCurrentListRoot = (popupRecHdl_t)(**gCurrentListRoot).fNext;
		// release the attached menu resource
		if ((**trailerH).fMenuHandle)
			ReleaseResource((Handle)(**trailerH).fMenuHandle);
		DisposeHandle((Handle)trailerH);
	}
	gCurrentListRoot = NULL;
} // KillPopups


//-----------------------------------------------------------
// Filterproc for popup userItem hits on mouse down (call from your dialog filter proc)
pascal Boolean PopupMouseDnDlgFilterProc(DialogPtr pDialogPtr, EventRecord *pEventPtr, short *pItemHitPtr)
{
	Point		mouseLoc,
				popLoc;
	short		newChoice,
				theItem;
	long		chosen;
	Boolean		myFilter;
	popupRecHdl_t	popupItemH = NULL;

	// pre-initialize return values
	*pItemHitPtr = 0;
	myFilter = false; // haven't handled yet

	if (pEventPtr->what == mouseDown)
	{
		mouseLoc = pEventPtr->where; // copy the mouse position
		GlobalToLocal(&mouseLoc); // convert it to local dialog coordinates
		
		// Was the click in a popup item?  NOTE: FindDItem is zero-based!
		theItem = FindDItem(pDialogPtr, mouseLoc)+1; // FindDialogItem someday...
		if (theItem >= 0)
			popupItemH = FindPopupRec(theItem);

		if (popupItemH)
		{
			HLock((Handle)popupItemH);
			// It's time to pop up the menu
			// - Insert the menu into the menu list,
			// - call CalcMenuSize (to work around a bug in the Menu Manager)
			// - call popupRecSelect and let the user drag around
			// Note that the (top,left) parameters to popupRecSelect are
			// our item’s, converted to global coordinates.

			// hilight the title
			InvertRect(&(**popupItemH).fTitleBounds);

			// insert our menu in the menu list
			InsertMenu((**popupItemH).fMenuHandle, -1);

			// copy our item’s topleft
			popLoc = *(Point*)(&(**popupItemH).fPopupBounds.top);
			LocalToGlobal(&popLoc); // convert back to global coords
			CalcMenuSize((**popupItemH).fMenuHandle); // Work around Menu Mgr bug

			chosen = PopUpMenuSelect((**popupItemH).fMenuHandle,
							popLoc.v, popLoc.h, (**popupItemH).fLastChoice);

			// Remove our menu from the menu list
			DeleteMenu((**popupItemH).fMenuID);

			// unhilight the title
			InvertRect(&(**popupItemH).fTitleBounds);
			
			// Was something chosen?
			if (chosen)
			{
				// get the chosen item number
				newChoice = chosen & 0x0000ffff;
				if (newChoice != (**popupItemH).fLastChoice)
				{
					// the user chose an item other than the current one
					// unmark old choice
					SetItemMark((**popupItemH).fMenuHandle, (**popupItemH).fLastChoice, noMark);
					(**popupItemH).fLastChoice = newChoice; // update the current choice
					// mark the new choice
					SetItemMark((**popupItemH).fMenuHandle, (**popupItemH).fLastChoice, checkMark);
					
					// Draw the newly selected item
					EraseRect(&(**popupItemH).fPopupBounds);
					DrawPopupMenu(pDialogPtr, (**popupItemH).fPopupItemID);
					
					myFilter = true; // dialog is over
					// have ModalDialog return that the user changed items
					*pItemHitPtr = (**popupItemH).fPopupItemID;
				} // if new choice
			} // if chosen

			HUnlock((Handle)popupItemH);
		} // if our popup Item
	} // if mousedown

	return myFilter;

} // PopupMouseDnDlgFilterProc



//-----------------------------------------------------------
short GetPopupValue(short pPopupItemID)
{
	short			theValue = 0;
	popupRecHdl_t	popupItemH;

	// find the popup rec
	popupItemH = FindPopupRec(pPopupItemID);

	if (popupItemH)
		theValue = (**popupItemH).fLastChoice;

	return(theValue);
} // GetPopupValue


