/*
	___________________________________________________________________________
NAME:
	StandardGetAnything.c 1.0.2

WRITTEN BY:
	Ammon Skidmore <ammon@cs.byu.edu> May-June 1995

BASED UPON:
	StandardGetFolder.p that Peter N Lewis <peter@cujo.curtin.edu.au> converted
	from Ken Kirksey's <kkirksey1@aol.com> StandardGetFolder.c (made with help
	from Paul Forrester <paulf@apple.com> and others.  Note though that my code
	is only broadly based upon this because I've cleaned up a lot of the code and
	removed a couple bugs in StandardGetFolder.c.
			
DESCRIPTION:
	Displays an open dialog that returns both files and folders.  Unlike its
	predecessors, StandardGetAnything.c does not use any global variables and
	the code does not rely upon any external, non OS libraries (stdio.h).

	This source is in the public domain so you are free to change it as you
	need for your own programs.  If you make any useful updates to this file
	that others could benifit from I encourage you to release it to the public
	as I did via alt.sources.mac.  If you do not have access to this newsgroup
	I would be happy to place it there if you email me the source.
	
VERSION INFO:
  1.0.2
	- Made the prompt border ditl item small so that it only underlines the prompt
	words.  This is to provide compatibility with the OpenWide control panel and
	I think that it looks better this way.
	- Added comments in the sfHookFirstCall section allowing you to set a custom
	prompt string if so desired.  All you have to do is create a global variable
	containing this string.
  1.0.1
	- Got rid of the prompt string parameter because Super Bomerang doesn't like
	it.  Oh well, it will make the code smaller anyway, but less flexable.
	- I also made the prompt border (the double red lines) stretch out extreemly far
	so that Dialog View's "Stretch" option looked aesthetically better.
	- A THINK C 7.0 project file is now included in the StandardGetAnything.c package.
	___________________________________________________________________________
*/

#include "StandardGetAnything.h"
#include <Aliases.h>

//=============================================================================
//		Private defines															 
//-----------------------------------------------------------------------------
#define rGetFolderButton		10
#define rGetFolderMessage		11
#define rOldItemName			13	// my invisible string storage spot
#define SFSaveDisk				-*(short*)0x214
#define CurDirStore				*(long*)0x398

//=============================================================================
//		Private Function Prototypes														 
//-----------------------------------------------------------------------------

pascal Boolean MyCustomGetDirectoryFileFilter(CInfoPBPtr myPB, Ptr myDataPtr);

pascal short MyCustomGetDirectoryDlogHook( short        item, 
                                           DialogPtr    theDialog,
                                           Ptr          myDataPtr);

void SetButtonTitle (Handle ButtonHdl, Str255 name, Rect *ButtonRect);


//=============================================================================
//		StandardGetAnything
//-----------------------------------------------------------------------------

StandardFileReply	StandardGetAnything(short dlogID)
{	StandardFileReply	reply;
	Point				where;
	SFTypeList			typeList;
	
		// Center the dialog
	SetPt(&where, -1, -1);
	
		// Call CustomGetFile. Pass it a pointer to the file filter and dialog
		// hook functions. Also pass a pointer to mySFReply in the user data field.
	CustomGetFile(	(FileFilterYDUPP)MyCustomGetDirectoryFileFilter,
					-1,			// -1 so show (pass) all files
					typeList,	// dummy in this case
					&reply,
					dlogID,
					where,
					(DlgHookYDUPP)MyCustomGetDirectoryDlogHook,
					nil,
					nil,
					nil,
					(void *)&reply);
	
	return(reply);
}

//=============================================================================
//		MyCustomGetDirectoryDlogHook
// This function lets us process item hits in the GetFolderDialog.
//-----------------------------------------------------------------------------
pascal short MyCustomGetDirectoryDlogHook(  short       item, 
                                            DialogPtr   theDialog, 
                                            Ptr         myDataPtr )
{	CInfoPBRec			pb;
	short				itemType;
	Rect				itemRect;
	Handle				itemHandle;
	Boolean				isFolder, isAlias;
	Str255				oldItemName;
	StandardFileReply	*mySFRPtr = (StandardFileReply*)myDataPtr;

	// CustomGet calls dialog hook for both main and subsidiary dialog boxes.
	// Make sure that dialog record indicates that this is the main GetFolder dialog.
	if ((OSType)(((WindowPeek)theDialog)->refCon) == sfMainDialogRefCon) {		
		if (item == sfHookFirstCall) {
			// We have nothing to set up so do nothing.
			
			// If you want to display a custom prompt with a global var promptStr
//			GetDItem(theDialog, rGetFolderMessage, &itemType, &itemHandle, &itemRect);
//			SetIText(itemHandle, promptStr);
			
		} else {
			// If the selected folder is an alias, resolve it.
			ResolveAliasFile (&(mySFRPtr->sfFile), TRUE, &isFolder, &isAlias);
			
			if (!mySFRPtr->sfFile.name[0]) {	// if nothing is selected,
			// set the reply spec to point to the current folder from its parent...
				pb.hFileInfo.ioCompletion = nil;
				pb.hFileInfo.ioNamePtr = (StringPtr)mySFRPtr->sfFile.name;
				pb.hFileInfo.ioVRefNum = mySFRPtr->sfFile.vRefNum;
				pb.hFileInfo.ioDirID = mySFRPtr->sfFile.parID;
				pb.hFileInfo.ioFDirIndex = -1;
				PBGetCatInfo(&pb, FALSE);
				mySFRPtr->sfFile.parID = pb.dirInfo.ioDrParID;
			}
			
			// now that mySFRPtr->sfFile.name holds the correct, current, resolved
			// item name, compair it with the last item name (stored in rOldItemName.)
			GetDItem(theDialog, rOldItemName, &itemType, &itemHandle, &itemRect);
			GetIText(itemHandle, oldItemName);
			if (!EqualString(mySFRPtr->sfFile.name, oldItemName, FALSE, FALSE)) {
				SetIText(itemHandle, mySFRPtr->sfFile.name);
				GetDItem(theDialog, rGetFolderButton, &itemType, &itemHandle, &itemRect);
				SetButtonTitle(itemHandle, mySFRPtr->sfFile.name, &itemRect);
			}
			
			// If the user clicked the select folder button, force a cancel and
			// set the sfGood field of the Reply record to true.
			if (item == rGetFolderButton)
			{
				item = sfItemCancelButton;
				mySFRPtr->sfGood = TRUE;
			}
		}
	}
	return(item);
}

//=============================================================================
//		MyCustomGetDirectoryFileFilter
// This is the file filter passed to CustomGetFile. It passes visible items only.
//-----------------------------------------------------------------------------
pascal Boolean MyCustomGetDirectoryFileFilter(CInfoPBPtr myPB, Ptr myDataPtr)
{
    return( !! (myPB->hFileInfo.ioFlFndrInfo.fdFlags & fInvisible ) );
}

//=============================================================================
//		SetButtonTitle
// Called to redraw the get item button.
//-----------------------------------------------------------------------------
void SetButtonTitle (Handle ButtonHdl, Str255 name, Rect *ButtonRect)
{	short   width;
	Str255	TmpStr;
	short	i;

//	StrCpy( gCurrentSelectedFolder, (char*) name );
	
	// Find the width left over in the button after drawing the word 'Select'
	// the quotation marks. Truncate the new name to this length.
	width = (ButtonRect->right - ButtonRect->left) - CharWidth('W') - CharWidth('W');

	TruncString(width, name, smTruncEnd );
	
	// draw the surronding quotes
	TmpStr[1] = '�';
	for (i=2; i<=name[0]+1; i++)
		TmpStr[i] = name[i-1];
	TmpStr[i] = '�';
	TmpStr[0] = name[0] + 2;

	// Redraw the button.
	SetCTitle((ControlHandle)(ButtonHdl), (StringPtr)TmpStr);
	ValidRect(ButtonRect);
}