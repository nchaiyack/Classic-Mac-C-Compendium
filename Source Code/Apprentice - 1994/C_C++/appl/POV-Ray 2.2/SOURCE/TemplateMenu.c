/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	TemplateMenu.c

Description:
	Template text insertion menus for POV-Ray.

	This is the main source file, containing the private definitions and
	code to implement all the needed external and internal support functions.

Related Files:
	TemplateMenu.h	- Header for these routines
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
	920901	[esp]	Created
	920906	[esp]	Added auto-scroll to new insertion point
	921226	[esp]	More error checking (& returning) in InitTemplateMenu
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931001	[esp]	Added code to delete current selection upon insert of new template
==============================================================================
*/

/*==== POV-Ray std headers ====*/
#include "povmac.h"


/*==== Macintosh-specific headers ====*/
// NOTE: _H_MacHeaders_ is defined by Think C if it is using
// precompiled headers.  This is only for compilation speed improvement.
#if !defined(_H_MacHeaders_)
#include <errors.h>
#include <resources.h>
#include <memory.h>
#include <menus.h>
#endif // _H_MacHeaders_


/*==== POV-Ray text editor header ====*/
#include "TextEditor.h"


#include "TemplateMenu.h"


/*==== definitions ====*/

#define	TEXT_RSRC_ID		200		// 200.., 300.., 400..

MenuHandle	gMacroMenus[NUM_MACRO_MENUS];


// ==============================================
OSErr InitTemplateMenu(void)
{
	Boolean		endOfList;
	short		anError = noErr;
	short		theMenuID;
	short		theItemID;
	Handle		theRsrcHandle;
	short		theRsrcID;
	ResType		theRsrcType;
	Str255		theRsrcName;

	// get submenus
	for (theMenuID=0; theMenuID<NUM_MACRO_MENUS; theMenuID++)
	{
		gMacroMenus[theMenuID] = GetMenu(macmn_sub_ID+theMenuID);
		anError = ResError();
		if (gMacroMenus[theMenuID] == NULL)
			anError = resNotFound;

		if (!anError)
		{
			// add TEXT items to this menu
			theItemID = TEXT_RSRC_ID + (100*theMenuID);
			endOfList = false;
			do {
				theRsrcHandle = GetResource('TEXT', theItemID);
				anError = ResError();
				if (theRsrcHandle == NULL)
					anError = resNotFound;

				// if we just hit end of list of resources, not really an error...
				if (anError == resNotFound)
				{
					anError = noErr;
					endOfList = true;
				}

				// Get its name
				if (!endOfList && !anError)
				{
					GetResInfo(theRsrcHandle, &theRsrcID, &theRsrcType, theRsrcName);
					anError = ResError();
				}

				// add it to the menu
				if (!endOfList && !anError)
				{
					AppendMenu(gMacroMenus[theMenuID], theRsrcName);
					ReleaseResource(theRsrcHandle);
					theItemID++;
				}
			} while (!endOfList && !anError);
	
			// insert submenu under rug
			if (!anError)
				InsertMenu(gMacroMenus[theMenuID], -1);

		}
	}
	return anError;
} // InitTemplateMenu


// ==============================================
void HandleTemplateMenu(short theMenuID, short theItem)
{
	short		anError = noErr;
	short		theOffset;
	short		theRsrcID;
	Handle		theRsrcHandle;

	theOffset = macmn_sub_ID;
	theOffset = theMenuID-macmn_sub_ID;
	if ((theOffset < NUM_MACRO_MENUS) && (gMacroMenus[theOffset] != NULL))
	{
		// get the text from the resource list
		theRsrcID = TEXT_RSRC_ID + (100*theOffset) + (theItem - 1);
		theRsrcHandle = GetResource('TEXT', theRsrcID);
		anError = ResError();
		if ((!anError) && (gSrcWind_TEH))
		{
			support_undo("Template", TRUE);
			HLock(theRsrcHandle);
			// Delete any selected text before the insertion
			TEDelete(gSrcWind_TEH);
			// stick in the text
			TEInsert(&(**theRsrcHandle), GetHandleSize(theRsrcHandle), gSrcWind_TEH);
			gSrcWind_dirty = true;
			ReleaseResource(theRsrcHandle);
			// scroll to new insertion point
			ShowSelect();
		}
	}
} // HandleTemplateMenu


// ==============================================
void KillTemplateMenu(void)
{
} // KillTemplateMenu
