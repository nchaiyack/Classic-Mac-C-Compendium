/*______________________________________________________________________

	sfu.c - Standard File Utilities.
	
	Copyright � 1988-1991 Northwestern University.
	
	This module exports reusable standard file utility routines.
	
	The primary purpose of this module is to permit the use of the new
	System 7 standard file routines, even on pre-System 7 sytems.
	
	This reusable module presents a modified standard open file dialog.
	There are two extra buttons which can be used to pick a folder instead
	of a file.  
_____________________________________________________________________*/

#pragma load "precompile"
#include "sfu.h"
#include "utl.h"

#pragma segment sfu

/*______________________________________________________________________

	Constant Definitions.
_____________________________________________________________________*/

#define sfItemMyPromptTest7		10		/* item number of 7.0 prompt string
														static text field */
#define sfItemMyDoitButton7		11		/* item number of 7.0 "Doit" button */

#define sfItemMyPromptTest6		10		/* item number of pre-7.0 prompt string
														static text field */
#define sfItemMyDoitButton6		11		/* item number of pre-7.0 "Doit" button */

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static short					Item;					/* item number of selected item */
static Str255					DoitButtonName;	/* name of "Doit" button */

/* The following globals are used only on 7.0 systems. */

static StandardFileReply	*Reply7;				/* pointer to reply record *.

/* The following globals are used only on pre-7.0 systems. */

static FileFilterYDProcPtr	FileFilter;			/* pointer to file filter proc */
static void						*YourDataPtr;		/* pointer to application data */
static SFReply					Reply6;				/* pre-7.0 reply record */

/*______________________________________________________________________

	sfu_StandardGetFile - Standard Get File Dialog.
	
	Entry:	fileFilter = pointer to file filter function, or nil if none.
				numTypes = number of file types in typeList, or -1 if none.
				typeList = file type list, or nil if none.
				reply = pointer to reply record.
								
	Exit:		reply record completed. Only the following fields are valid:
				sfGood and sfFile.
_____________________________________________________________________*/

void sfu_StandardGetFile (FileFilterProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply)
	
{
	SFReply			reply6;		/* System 6 reply */
	Handle			h;				/* handle to System 6 dialog */
	Point				where;		/* location of centered System 6 dialog */

	if (utl_GestaltFlag(gestaltStandardFileAttr, gestaltStandardFile58)) {
	
		/* System 7. */
		
		StandardGetFile(fileFilter, numTypes, typeList, reply);
		
	} else {
		
		/* Pre-System 7. */
		
		h = GetResource('DLOG', getDlgID);
		utl_CenterDlogRect(*(Rect**)h, true);
		where = **(Point**)h;
		SFGetFile(where, "\p", fileFilter, numTypes, typeList, nil, &reply6);
		if (!(reply->sfGood = reply6.good)) return;
		reply->sfFile.vRefNum = -*(short*)SFSaveDisk;
		reply->sfFile.parID = *(long*)CurDirStore;
		utl_CopyPString(reply->sfFile.name, reply6.fName);
		
	}
}

/*______________________________________________________________________

	sfu_StandardPutFile - Standard Put File Dialog.
	
	Entry:	prompt = prompt string.
				defaultName = default file name.
				reply = pointer to reply record.
								
	Exit:		reply record completed. Only the following fields are valid:
				sfGood and sfFile.
_____________________________________________________________________*/

void sfu_StandardPutFile (Str255 prompt, Str255 defaultName,
	StandardFileReply *reply)
	
{
	SFReply			reply6;		/* System 6 reply */
	Handle			h;				/* handle to System 6 dialog */
	Point				where;		/* location of centered System 6 dialog */

	if (utl_GestaltFlag(gestaltStandardFileAttr, gestaltStandardFile58)) {
	
		/* System 7. */
		
		StandardPutFile(prompt, defaultName, reply);
		
	} else {
		
		/* Pre-System 7. */
		
		h = GetResource('DLOG', putDlgID);
		utl_CenterDlogRect(*(Rect**)h, true);
		where = **(Point**)h;
		SFPutFile(where, prompt, defaultName, nil, &reply6);
		if (!(reply->sfGood = reply6.good)) return;
		reply->sfFile.vRefNum = -*(short*)SFSaveDisk;
		reply->sfFile.parID = *(long*)CurDirStore;
		utl_CopyPString(reply->sfFile.name, reply6.fName);
		
	}
}

/*______________________________________________________________________

	FileFilter6 - Pre-7.0 File Filter Proc.
	
	Entry:	pBlock = pointer to file info param block.
_____________________________________________________________________*/

static pascal Boolean FileFilter6 (ParmBlkPtr pBlock)

{
	return (*FileFilter)(pBlock, YourDataPtr);
}

/*______________________________________________________________________

	MyHook7 - 7.0 CustomGetFile Dialog Hook.
	
	Entry:	item = item number of selected item.
				theDialog = pointer to dialog.
				yourDataPtr = pointer to application data (unused).
				
	Exit:		function result = item number of selected item.
_____________________________________________________________________*/

static pascal short MyHook7 (short item, DialogPtr theDialog, void *yourDataPtr)

{
#pragma unused (yourDataPtr)

	short					itemType;			/* item type */
	ControlHandle		doit;					/* handle to doit button */
	ControlHandle		open;					/* handle to open button */
	Rect					box;					/* item rect */
	
	/* The following line of code fixes a mysterious problem in CustomGetFile.
		This problem showed up when I tried to do an option-Scan and select
		one of my Disinfectant test files (the Red Ryder 10.0 Scores-infected
		acid test). This file has two CODE id=35 resources, but is otherwise
		normal.
		
		Without the following line of code, CustomGetFile bombed!
		
		The line of code makes a click on the Open button (or an equivalent
		press of the Return or Enter key) when a file is
		selected be treated the same as a click on the "Doit" button. I have
		no idea why this works and fixes the problem, but it does. */
	
	if (item == sfItemOpenButton) item = sfItemMyDoitButton7;

	if (item != sfHookLastCall) Item = item;
	GetDItem(theDialog, sfItemMyDoitButton7, &itemType, (Handle*)&doit, &box);
	GetDItem(theDialog, sfItemOpenButton, &itemType, (Handle*)&open, &box);
	
	/* Set name of "Doit" button if first call. */
	
	if (item == sfHookFirstCall) SetCTitle(doit, DoitButtonName);
	
	/* Hilite the "Doit" button iff the Open button is hilited. */
	
	HiliteControl(doit, (**open).contrlHilite); 
	
	/* If the "Doit" button was selected, pretend the Cancel button was 
		selected to force CustomGetFile to return. */
	
	if (item == sfItemMyDoitButton7) return sfItemCancelButton;
	
	/* For all other items do nothing. */
	
	return item;
}

/*______________________________________________________________________

	MyHook6 - Pre-7.0 SFPGetFile Dialog Hook.
	
	Entry:	item = item number of selected item.
				theDialog = pointer to dialog.
				
	Exit:		function result = item number of selected item.
_____________________________________________________________________*/

static pascal short MyHook6 (short item, DialogPtr theDialog)

{
	short					itemType;			/* item type */
	ControlHandle		doit;					/* handle to doit button */
	ControlHandle		open;					/* handle to open button */
	Rect					box;					/* item rect */
	
	Item = item;
	GetDItem(theDialog, sfItemMyDoitButton6, &itemType, (Handle*)&doit, &box);
	GetDItem(theDialog, getOpen, &itemType, (Handle*)&open, &box);
	
	/* Set name of "Doit" button if first call. */
	
	if (item == sfHookFirstCall) SetCTitle(doit, DoitButtonName);
	
	/* Hilite the "Doit" button iff the Open button is hilited. */
	
	HiliteControl(doit, (**open).contrlHilite); 
	
	/* If the "Doit" button was selected, pretend the Cancel button was 
		selected to force SFPGetFile to return. */
	
	if (item == sfItemMyDoitButton6) return getCancel;
	
	/* For all other items do nothing. */
	
	return item;
}

/*______________________________________________________________________

	sfu_GetFileOrFolder - Get File or Folder.
	
	Entry:	fileFilter = file selection filter procedure.
				numTypes = number of file types.
				typeList = array of file types.
				reply = pointer to standard file package reply record.
				dlgID6 = pre-7.0 dialog resource id.
				dlgID7 = 7.0 dialog resource id.
				where = location of top left corner of dialog, in global coords,
					or (-1,-1) to center dialog on main screen.
				yourDataPtr = optional pointer to application data.
				doitButtonName = name of "Doit" button.
				prompt = prompt string.
								
	Exit:		reply record completed. Only the following fields are valid:
				sfGood, sfFile, sfIsFolder, and sfIsVolume.
_____________________________________________________________________*/

void sfu_GetFileOrFolder (FileFilterYDProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply,
	short dlgID6, short dlgID7, Point where, 
	void *yourDataPtr, Str255 doitButtonName, Str255 prompt)

{
	CInfoPBRec		catPBlock;		/* PBGetCatInfo param block */
	Handle			h;					/* handle to DLOG resource */
	Boolean			isFolderAlias;	/* true if item is folder or vol alias */
	Boolean			isAlias;			/* true if item is alias */
	OSErr				rCode;			/* result code */
	
	utl_CopyPString(DoitButtonName, doitButtonName);
	
	if (utl_GestaltFlag(gestaltStandardFileAttr, gestaltStandardFile58)) {
	
		/* System 7. */
		
		Reply7 = reply;
		ParamText(prompt, nil, nil, nil);
		
		while (true) {
		
			/* Call the standard file package. */
			
			CustomGetFile(fileFilter, numTypes, typeList, reply, dlgID7,
				where, MyHook7, nil, nil, nil, yourDataPtr);
				
			/* Return if dialog canceled. */
			
			if (Item == sfItemCancelButton) return;
			reply->sfGood = true;
			
			/* Resolve aliases. */
	
			if (Item == sfItemMyDoitButton7) {
				rCode = ResolveAliasFile(&reply->sfFile, true, 
					&isFolderAlias, &isAlias);
				if (rCode) continue;
				if (isFolderAlias) reply->sfIsFolder = true;
				break;
			}
		
		}
	
		/* If a folder was selected check to see if it's really a volume. */
		
		if (reply->sfIsFolder && reply->sfFile.parID == fsRtParID) {
			reply->sfIsFolder = false;
			reply->sfIsVolume = true;
		}
	
	} else {
	
		/* Pre-System 7. */
		
		FileFilter = fileFilter;
		YourDataPtr = yourDataPtr;
		
		/* Center the dialog if requested. */
			
		if (where.h == -1 && where.v == -1) {
			h = GetResource('DLOG', dlgID6);
			utl_CenterDlogRect(*(Rect**)h, true);
			where = **(Point**)h;
		}
		
		/* Call the standard file package. */
		
		SFPGetFile(where, prompt, fileFilter ? FileFilter6 : nil, 
			numTypes, typeList, MyHook6,
			&Reply6, dlgID6, nil);
		
		/* Return if dialog canceled. */
		
		if (Item == getCancel) {
			reply->sfGood = false;
			return;
		}
		
		/* Set reply fields. */
		
		reply->sfGood = true;
		reply->sfIsVolume = false;
		reply->sfFile.vRefNum = -*(short*)SFSaveDisk;
		if (Item == sfItemMyDoitButton6 && !*Reply6.fName) {
			reply->sfIsFolder = true;
			reply->sfFile.parID = *(long*)CurDirStore;
			catPBlock.dirInfo.ioNamePtr = reply->sfFile.name;
			catPBlock.dirInfo.ioVRefNum = reply->sfFile.vRefNum;
			catPBlock.dirInfo.ioFDirIndex = -1;
			catPBlock.dirInfo.ioDrDirID = Reply6.fType;
			PBGetCatInfo(&catPBlock, false);
		} else {
			reply->sfIsFolder = false;
			reply->sfFile.parID = *(long*)CurDirStore;
			utl_CopyPString(reply->sfFile.name, Reply6.fName);
		}
	}
}
		