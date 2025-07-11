/*______________________________________________________________________

	gff.c - Get File or Folder.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This reusable module presents a modified standard open file dialog.
	There are two extra buttons which can be used to pick a folder instead
	of a file.  
	
	The caller supplies the id of the dialog to be used.  It must be a 
	standard open file dialog with two additional buttons:
	
	item 11 = Select current folder button.
	item 12 = Folder button.
_____________________________________________________________________*/


#pragma load "precompile"
#include "gff.h"

#pragma segment gff

/*______________________________________________________________________

	Constant Definitions.
_____________________________________________________________________*/


#define getSCFold		11			/* item number of select current folder 
											button */
#define getFold		12			/* item number of folder button. */




/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static SFReply		*Reply;				/* pointer to reply record */
static Boolean		FolderActive;		/* true if folder button is active */
static short		Item;					/* item number of selected item */
static OSType		DirID;				/* dir id of selected folder */

/*______________________________________________________________________

	HiliteFolderButton - Hilite the Folder Button.
	
	Entry:	theDialog = pointer to dialog.
				active = true to make button active, false to make it inacative.
_____________________________________________________________________*/


static void HiliteFolderButton (DialogPtr theDialog, Boolean active)

{
	short				itemType;			/* item type */
	Handle			item;					/* handle to item */
	Rect				box;					/* item rectangle */

	GetDItem(theDialog, getFold, &itemType, &item, &box);
	HiliteControl((ControlHandle)item, active ? 0 : 255);
	FolderActive = active;
}

/*______________________________________________________________________

	MyHook - SFPGetFile Dialog Hook.
	
	Entry:	item = item number of selected item.
				theDialog = pointer to dialog.
				
	Exit:		function result = item number of selected item.
_____________________________________________________________________*/


pascal short MyHook (short item, DialogPtr theDialog)

{
	Item = item;

	/* Hilite the Folder button iff a folder is currently selected. */

	if (!*(Reply->fName) && Reply->fType) {
		if (!FolderActive) HiliteFolderButton(theDialog, true);
	} else {
		if (FolderActive) HiliteFolderButton(theDialog, false);
	};
	
	/* Check for our two additional buttons.  If either one was selected
		pretend the Cancel button was selected to force SFPGetFile to return. */
	
	if (item == getSCFold) return getCancel;
	if (item == getFold) {
		DirID = Reply->fType;
		return getCancel;
	};
	
	/* For all other buttons do nothing. */
	
	return item;
}

/*______________________________________________________________________

	gff_get - Get File or Folder
	
	Entry:	where = pointer to location of top left corner of dialog, in 
					global coords.
				prompt = prompt string.
				fileFilter = file selection filter procedure.
				numTypes = number of file types.
				typeList = array of file types.
				reply = pointer to standard file package reply record.
				dlgID = dialog resource id.
				
	Exit:		reply->good = true if something selected.
				
				if a file was selected:
				
				reply->vRefNum = vol ref num or working directory ref num of 
					volume or directory that contains the file.
				reply->version = vol ref num of volume that contains the file.
				reply->fName = name of the file.
				
				if a folder was selected:
				
				reply->vRefNum = vRefNum of volume containing folder.
				reply->fType = directory id of folder.
				reply->fName = empty string.
_____________________________________________________________________*/
		
		
void gff_Get (Point *where, Str255 prompt, FileFilterProcPtr fileFilter, 
	short numTypes, SFTypeList typeList, SFReply *reply, 
	short dlgID)

{
	/* Initialize global variables. */

	Reply = reply;
	FolderActive = true;
	
	/* Call the standard file package. */
	
	SFPGetFile(*where, prompt, fileFilter, numTypes, typeList, MyHook,
		reply, dlgID, nil);
	
	/* Return if dialog canceled. */
	
	if (Item == getCancel) return;
	
	/* If file selected set version field to vol ref num and return. */
	
	if (reply->good) {
		reply->version = - (* (short*) SFSaveDisk);
		return;
	};
	
	/* If a folder was selected clear the file name and set the vRefNum
		and directory id. */
	
	reply->good = true;
	*(reply->fName) = 0;
	reply->vRefNum = - (* (short*) SFSaveDisk);
	reply->fType = (Item == getFold) ? DirID : * (OSType*) CurDirStore;
	return;
}
		