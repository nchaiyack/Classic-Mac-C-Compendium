////////
/////// Select Multiple Files Demo 0.99 � By Eddy J. Gurney / �1994 The Eccentricity Group
//////
///// The code below demonstrates how to present a dialog box that allows the user to
//// select multiple files, ala THINK C's "Add Files..." option. This version is better, however,
/// since the user can easily select a single file without clicking two buttons, and because
// the user can "Tab" between the two lists.
//
// This is NOT a trivial task, as you'll see from the amount of code necessary to support
// this feature. I worked on this every night for over a week trying to get things to work
// the way I wanted them to for an application I'm writing. After I got it working, I saw
// several requests in comp.sys.mac.programmer for this type of functionality, so I put
// together this demo app. My application did NOT require all the features I demonstrate
// here -- my application uses files in the order the user clicked them, not alphabetical
// order like shown here (alpha order is better for most apps, though) and my app allows
// the user to select a particular file as many times as they'd like, not once like shown
// here (I didn't need the "file filter" code, but provided one in the demo since others will).
// Since removing features is easier than adding them, I added those features in hope that
// they would prove useful to someone.
//
// That said, let me say that there are probably still bugs in this code, and that this is only
// meant to be a "starting point" for you! Neither Hewlett-Packard nor myself are responsible
// for anything that happens from the use of this code. That is, #include <std/disclaimer.h>
//
// Very special thanks to Ari Halberstadt, for his "WinterShell", which got me started
// down the right path for this endeavour, and to Scott Knaster / Keith Rollin, authors of
// "Macintosh Programming Secrets" for lots of insight when I was learning to program
// the Mac. Extra special thanks to Lloyd Lim of Lim Unlimited for allowing me to use his
// wonderful method of drawing gray outlines. The original source for this material was
// his "Default" CDEF, version 2.4.
//
// If you use this code, or any code derived from it, in your application, you should include
// my name in your "About" box and/or documentation. I'd like to hear about it via e-mail
// as well. And if you want to give me a registered copy, that would be fine with me, too!
// Other than that, there are no restrictions on the use or distribution of this code, as
// long as this notice, my name and copyright remains intact when redistributing it.
//
// Oh yeah, if you do find any bugs, you improve the code, find this useful, or have any
// suggestions, want to give me money, etc., please let me know!
//
// Eddy J. Gurney <egurney@vcd.hp.com>
// v0.97 April 26, 1994 (My initial working version of this demo. Never released.)
// v0.98 November 30, 1994 (Cleaned up code for release on the Apprentice CD-ROM.)
//
// To do:
//   * Add support for keyboard in "selected files" list
//   * Clean up Balloon Help
//
// PS: I write code in Geneva 9 with tabs every 4 spaces, so you may have to adjust
// your editor to view this like I do. :-)

#include "Select Multiple Files.h"
#include "ArrayListLib.h"
#include <GestaltEqu.h>	/* For Gestalt constants */
#include <Packages.h>	/* For IUMagString */
#include <Palettes.h>	/* For GetGray() */

/* Local types */
typedef struct {
	StandardFileReply	*theReply;	/* the reply record */
	ListHandle			selectedList;	/* handle to the select file list */
	ArrayListHandle	files;		/* list of selected files */
	short			numTypes;
	SFTypeList		typeList;
	Boolean			active;		/* true if our file selection list is active */
	Boolean			findNextFile;
	short			nextCharacter;
	short			extraEvent;
} SFMultipleFileData;

/* For FrameItemInGray() */
typedef struct {
	PenState		penState;
	RGBColor		backColor;
	RGBColor		frameColor;
	Rect			bounds;
	Boolean		inactive;
	Boolean		roundFrame;
	Boolean		erasingOld;
} GrayFrameData, *GrayFrameDataPtr;

/* Function prototypes */
static pascal Boolean SFMultipleFileModalFilter(DialogPtr, EventRecord *, short *, void *);
static pascal short SFMultipleFileDialogHook(short, DialogPtr, void *);
static void DeselectListItems(ListHandle);
void AddFileToList(DialogPtr, SFMultipleFileData *, FSSpec *, Boolean);
pascal short ListMgrCompString(Ptr, Ptr, short, short);
Point InsertInOrder(StringPtr, ListHandle);
Boolean IsValidFileType(short, SFTypeList, OSType);
Boolean FileAlreadyAdded(short, long, StringPtr, SFMultipleFileData *);
static pascal void SFMultipleFileActivate(DialogPtr, short, Boolean, void *);
static pascal Boolean SFMultipleFileFilter(ParamBlockRec *, void *);
void ToolBoxInit(void);
void FlashDialogItem(DialogPtr, short);
void GetDItemRect(DialogPtr, short, Rect *);
void HiliteDialogItem(DialogPtr, short, short);
void MySetDialogDefaultItem(DialogPtr, short, Boolean);
pascal void FrameItemInGray(WindowPtr, short);
static pascal void GrayFrameDrawingProc(short, short, GDHandle, GrayFrameDataPtr);
void SetGrayFrameDialogItem(DialogPtr, short);
void SetDItemTitle(DialogPtr, short, StringPtr);

/* Global variables */
long				gQuickDrawVersion;

main()
{
	long				gestaltResult;
	StandardFileReply	theSFReply;
	Point				where = { -1, -1 };
	short			sendFileRefNum;
	short			activeInputItemsList[] = { 2, sfItemFileListUser, dSFMultipleSelectedFilesList };
	SFMultipleFileData	fileData;
	OSErr			result;

	ToolBoxInit();
	
	if (Gestalt(gestaltQuickdrawVersion, &gQuickDrawVersion) != noErr)
		ErrorHandler("\pGestalt error!");
	if (Gestalt(gestaltStandardFileAttr, &gestaltResult) != noErr)
		ErrorHandler("\pGestalt error!");
	if (!(gestaltResult & (1 << gestaltStandardFile58)))
		ErrorHandler("\pThe new Standard File routines are missing! Upgrade your system software!");

	/* Initialize the multiple file data structure before we use it */
	fileData.theReply = &theSFReply;
	fileData.selectedList = nil;
	fileData.files = ArrayListBegin(sizeof(FSSpec)); /* Start our list of FSSpec's */
	fileData.numTypes = 1;
	fileData.typeList[0] = 'TEXT';
	fileData.active = false;
	fileData.findNextFile = false;
	fileData.extraEvent = 0;

	/* Present the multiple file dialog */
	CustomGetFile((FileFilterYDUPP)SFMultipleFileFilter, fileData.numTypes, fileData.typeList, &theSFReply, 
		rSFMultipleFileDialog, where, SFMultipleFileDialogHook, SFMultipleFileModalFilter,
		activeInputItemsList, SFMultipleFileActivate, &fileData);

	if (theSFReply.sfGood) { /* The user clicked the "Open All" ("Open File") button */
		long			totalSizeOfSelectedFiles;
		FSSpec		fileSpec;
		CInfoPBRec	pb;
		DialogPtr		theDialog;
		short		index, itemHit;
		Str31		theNumber;

		/* Handle the case where the user clicked on the "Open File" button without adding any */
		if (ArrayListCount(fileData.files) == 0) {
			ArrayListInsert(fileData.files, 0);
			ArrayListSet(fileData.files, 0, &theSFReply.sfFile);
		}

		/* Now do something interesting with the selected files.
		    This shows how to access the files the user has selected. */

		/* Get the size of each file and add it to the total... */
		totalSizeOfSelectedFiles = 0;
		for (index = 0; index < ArrayListCount(fileData.files); index++) {
			ArrayListGet(fileData.files, index, &fileSpec);
			pb.hFileInfo.ioCompletion = nil;
			pb.hFileInfo.ioNamePtr = fileSpec.name;
			pb.hFileInfo.ioVRefNum = fileSpec.vRefNum;
			pb.hFileInfo.ioFDirIndex = 0;
			pb.hFileInfo.ioDirID = fileSpec.parID;
			if (PBGetCatInfo(&pb, false) == noErr)
				totalSizeOfSelectedFiles += pb.hFileInfo.ioFlLgLen;
			else
				break;
		}		

		/* Present a simple dialog to the user */
		NumToString((long)totalSizeOfSelectedFiles, theNumber);
		ParamText(theNumber, nil, nil, nil);
		theDialog = GetNewDialog(rTotalDialog, nil, (WindowPtr)-1);
		ShowWindow(theDialog);
		do {
			ModalDialog(nil, &itemHit);
		} while (!itemHit);
		DisposeDialog(theDialog);
	}

	ArrayListEnd(fileData.files); /* Dispose of the memory we used for the list of files */
}

/******************************************************************/

/* Handle the dialog events */
static pascal Boolean SFMultipleFileModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *itemHit, void *myDataPtr)
{
	GrafPtr			savePort;
	short			iType;
	Handle			iHandle;
	Rect				iRect;
	SFMultipleFileData	*fileData = myDataPtr;
	Boolean			result = false;

	GetPort(&savePort);

	switch (theEvent->what) {
		case updateEvt: {
			PenState	savePen;

			if (/* IsAppWindow((WindowPtr)theEvent->message) */ 0) {
				/* IsAppWindow is a function you provide that returns true if the passed WindowPtr
				   belongs to your application. If it does, then you need to call your application's update
				   function to redraw any windows which have Balloon Help holes in them, etc. */
				/* ProcessUpdateEvent((WindowPtr)theEvent->message); */
			 } else {
				/* It wasn't one of our application windows, so it must be our dialog */
				SetPort(theDialog);
				GetPenState(&savePen);
				PenNormal();
				LUpdate(theDialog->visRgn, fileData->selectedList);
				GetDItem(theDialog, dSFMultipleSelectedFilesList, &iType, &iHandle, &iRect);
				FrameRect(&iRect);
				if (fileData->active) {
					InsetRect(&iRect, -3, -3);
					PenSize(2, 2);
					FrameRect(&iRect);
				}
				SetPenState(&savePen);
			}
			break;
		}
		case activateEvt:
			if (/* IsAppWindow((WindowPtr)theEvent->message) */ 0) {
				/* IsAppWindow is a function you provide that returns true if the passed WindowPtr
				   belongs to your application. If it does, then you need to call your application's activate
				   function to activate/deactivate the specified window, etc. */
				/* ProcessActivateEvent((WindowPtr)theEvent->message, theEvent->modifiers & activeFlag); */
			} else {
				/* It wasn't one of our application windows, so it must be our dialog */
				SetPort(theDialog);
				SFMultipleFileActivate(theDialog, dSFMultipleSelectedFilesList, 
					fileData->active && (theEvent->modifiers & activeFlag) != 0, fileData);
				LActivate((theEvent->modifiers & activeFlag) != 0, fileData->selectedList);
			}
			break;
		case mouseDown:
			if (GetWRefCon(theDialog) == sfMainDialogRefCon) {
				Point		where = theEvent->where;

				SetPort(theDialog);				
				GlobalToLocal(&where);

				GetDItemRect(theDialog, dSFMultipleSelectedFilesList, &iRect);
				if (PtInRect(where, &iRect)) {
					/* A double click in the "selected files" list is the same as clicking "Remove"... */
					if (LClick(where, theEvent->modifiers, fileData->selectedList)) {
						*itemHit = dSFMultipleRemoveButton;
						FlashDialogItem(theDialog, *itemHit);
						result = true;
					}
				}
			}
			break;
		case keyDown:
		case autoKey:
			if (GetWRefCon(theDialog) == sfMainDialogRefCon) {
				char		key = theEvent->message;
				
				SetPort(theDialog);
				if ((theEvent->modifiers & cmdKey) != 0) {
					switch (key) {
						/* Handle any dialog Command-key equivalents for the dialog here, 
						   like Cmd-A for Add All, or something? */ 
					}
				} else if (fileData->active && key != tabKey) {
					/* Ooops. This isn't quite done yet! You need to handle the up/down keys
					here, add the ability for the user to type the first few characters of an
					entry and jump to it, etc. See Ari's "WinterShell" if you need help! */
				}
			}
			break;
	}
	
	SetPort(savePort);
	return result;
}

static pascal short SFMultipleFileDialogHook(short item, DialogPtr theDialog, void *myDataPtr)
{
	GrafPtr			savePort;
	SFMultipleFileData	*fileData = myDataPtr;
	Cell				cell;					/* for accessing cells in list */
	short			index;				/* index for inserting/removing from list */
	
	GetPort(&savePort);
	SetPort(theDialog);
	
	if (GetWRefCon(theDialog) == sfMainDialogRefCon) {
		Boolean	isFile = !fileData->theReply->sfIsVolume && !fileData->theReply->sfIsFolder && *fileData->theReply->sfFile.name;

		/* Select the next file in the list for the user OR adjust the dialog buttons. */
		if (fileData->selectedList) {
			/* Ok, this is a real wizzy feature. When you add a file, it is removed from the "top" list
			  of files, and the hilite for that list just "disappears". That means the user has to manually
			  select the next file to add. (If you really want to see what I'm talking about, just comment
			  out the whole if statement below and click the "Add" button.) Since this code hilites the last file
			  added in the "bottom" list, we can get its name... and simulate the user typing in each character
			  of the name! This means the hilite moves to the file (or directory) AFTER the file the user just
			  added. Cool, huh? It IS a little sluggish right now, probably because of the two List Manager
			  calls. It would be faster to keep a copy of the filename we just added in the fileData structure,
			  but this was a quick-and-dirty last minute hack, so I'll leave performance optimizations as an
			  exercise for the programmer. :-) */
			if (fileData->findNextFile && !fileData->active) {
				Str63	fileName;
				short	length = sizeof(fileName);
				
				cell.h = cell.v = 0;
				if (LGetSelect(true, &cell, fileData->selectedList)) {
					LGetCell(fileName, &length, cell, fileData->selectedList);
					if (fileData->nextCharacter < length) {
						item = sfHookCharOffset + *(fileName + fileData->nextCharacter);
						fileData->nextCharacter++;
						return item;
					}
					fileData->findNextFile = false;
				}
			}

			/* See the comments at the end of the "Remove" button code for why we have this */
			if (fileData->extraEvent) {
				item = fileData->extraEvent;
				fileData->extraEvent = 0;
				return item;
			}
			
			cell.h = cell.v = 0;
			/* If a file in the "selected" list is selected, enable the Remove button */
			HiliteDialogItem(theDialog, dSFMultipleRemoveButton, LGetSelect(true, &cell, fileData->selectedList) ? kControlActive : kControlInactive);
			/* If the focus is in the list of files, enable the Add All button */
			HiliteDialogItem(theDialog, dSFMultipleAddAllButton, !fileData->active ? kControlActive : kControlInactive);
			/* Enable our "Open" button if a file is selected, or if files have been added */
			if ((ArrayListCount(fileData->files) > 0) || (!fileData->active && isFile))
				HiliteDialogItem(theDialog, dSFMultipleOpenButton, kControlActive);
			else
				HiliteDialogItem(theDialog, dSFMultipleOpenButton, kControlInactive); 
			/* Change the default button depending on where the input focus is */
			if (fileData->active) {
				if (ArrayListCount(fileData->files) == 0) {
					MySetDialogDefaultItem(theDialog, sfItemCancelButton, false);
				} else {
					MySetDialogDefaultItem(theDialog, dSFMultipleOpenButton, false);
				}
			} else {
				if (*fileData->theReply->sfFile.name) {
					MySetDialogDefaultItem(theDialog, sfItemOpenButton, true);
				} else {
					MySetDialogDefaultItem(theDialog, sfItemCancelButton, false);
				}
			}			
		}
		switch (item) {
			case sfHookFirstCall:
			{
				Rect		iRect;
				Rect		rDataBounds = { 0, 0, 0, 1 };	/* Data bounds of list */
			
				/* Create the list */
				cell.h = cell.v = 0;
				GetDItemRect(theDialog, dSFMultipleSelectedFilesList, &iRect);
				InsetRect(&iRect, -kListInset, -kListInset);
				iRect.right -= kScrollBarAdjust;
				fileData->selectedList = LNew(&iRect, &rDataBounds, cell, 0, theDialog, true, false, false, true);
				if (fileData->selectedList == nil)
					ErrorHandler("\pCouldn't create the selected file list!!");
				SetGrayFrameDialogItem(theDialog, dSFMultipleLine1);
				SetGrayFrameDialogItem(theDialog, dSFMultipleLine2);
				MySetDialogDefaultItem(theDialog, sfItemOpenButton, false);
				break;
			}
			case sfHookLastCall:
				/* Dispose of the list */
				LDispose(fileData->selectedList);
				fileData->selectedList = nil;
				break;
			case sfItemOpenButton:
				/* Add file to list */
				if (*fileData->theReply->sfFile.name && !fileData->active && isFile) {
					AddFileToList(theDialog, fileData, &fileData->theReply->sfFile, true);
					fileData->findNextFile = true;
					fileData->nextCharacter = 0;
					item = sfHookRebuildList;
				}
				break;
			case dSFMultipleOpenButton:
				if (isFile || (ArrayListCount(fileData->files) > 0))
					/* Fake the system into thinking we selected the file by clicking on its "Open" button */
					item = sfItemOpenButton;
				else
					item = 0;
				break;
			case dSFMultipleRemoveButton:
				cell.h = cell.v = 0;
				/* There can be more than one item selected in the "bottom" list, so remove all of 'em */
				while (LGetSelect(true, &cell, fileData->selectedList)) {
					index = cell.v;
					LDelRow(1, index, fileData->selectedList);
					ArrayListDelete(fileData->files, index);
					cell.h = cell.v = 0;
				}
				/* If there is only 1 item in the list, change the "Open List" button back to "Open File" */ 
				if (ArrayListCount(fileData->files) == 0) {
					Str31	openString;
					
					GetIndString(openString, rChangingButtonStrings, kOpenFileButton);
					SetDItemTitle(theDialog, dSFMultipleOpenButton, openString);
				}
				/* We want to be "cool"... so clicking on the remove button will select the "selected files"
				   (bottom) list, unless we're removing the last item, in which case we automatically switch
				   the input focus to the "file" list. We can only return one "item" (rebuildList in this case) so
				   we handle it "special"... */
				if (ArrayListCount(fileData->files) == 0) {
					/* If there are no items left in the list, switch the focus back to the file list */
					fileData->extraEvent = sfHookSetActiveOffset + sfItemFileListUser;
				} else {
					fileData->extraEvent = sfHookSetActiveOffset + dSFMultipleSelectedFilesList;
				}
				item = sfHookRebuildList;
				break;
			case dSFMultipleAddAllButton:
			{
				FSSpec		fsSpec;
				CInfoPBRec	pb;
				Str63		name;
				Boolean		alreadyDeselected = false;
				
				LDoDraw(false, fileData->selectedList);	/* Turn off list updating while we add files */
				pb.hFileInfo.ioVRefNum = -LMGetSFSaveDisk();	/* The current directory SF is in */
				pb.hFileInfo.ioNamePtr = name;	/* Where the name of each entry will be placed */
				index = 1;
				do {
					pb.hFileInfo.ioDirID = LMGetCurDirStore();	/* You need to set this each time through the loop! */
					pb.hFileInfo.ioFDirIndex = index++;
					PBGetCatInfo(&pb, false);	/* Get file info */
					/* If successful GetCatInfo, the entry is a file, the file type is OK, and it hasn't been added, add it */
					if (pb.hFileInfo.ioResult == noErr && (pb.hFileInfo.ioFlAttrib & 0x10) == 0 &&
					   IsValidFileType(fileData->numTypes, fileData->typeList, pb.hFileInfo.ioFlFndrInfo.fdType) &&
					   !FileAlreadyAdded(pb.hFileInfo.ioVRefNum, pb.hFileInfo.ioFlParID, pb.hFileInfo.ioNamePtr, fileData)) {
					   	if (!alreadyDeselected) {
							DeselectListItems(fileData->selectedList);
							alreadyDeselected = true;
						}
						/* Create an FSSpec, since that is what we store for each added entry */
						fsSpec.vRefNum = pb.hFileInfo.ioVRefNum;
						fsSpec.parID = pb.hFileInfo.ioFlParID;
						BlockMove(pb.hFileInfo.ioNamePtr, fsSpec.name, *pb.hFileInfo.ioNamePtr + 1);
						AddFileToList(theDialog, fileData, &fsSpec, false);
					}
				} while (pb.hFileInfo.ioResult == noErr);
				LDoDraw(true, fileData->selectedList);	/* Turn drawing back on */
				LUpdate(theDialog->visRgn, fileData->selectedList);	/* And update the list */
				item = sfHookRebuildList;
			}
			break;
		}
	}
	
	SetPort(savePort);
	return item;
}

/* We deselect any items already selected before adding a new file */
static void DeselectListItems(ListHandle theList)
{
	Cell		theCell = {0, 0};
	
	while (theCell.v < (*theList)->dataBounds.bottom) {
		LSetSelect(false, theCell, theList);
		theCell.v++;
	}
}

/* Add the information passed to the specified list */
void AddFileToList(DialogPtr theDialog, SFMultipleFileData *fileData, FSSpec *theReply, Boolean doDeselect)
{
	Point		cell;
	short	index;
	
	/* Insert name into the list */
	cell = InsertInOrder(theReply->name, fileData->selectedList);
	index = cell.v;
	/* Insert into list of files, at index corresponding to item in display list, which makes it easy to
	   remove the item when the user hits Remove button */
	ArrayListInsert(fileData->files, index);
	ArrayListSet(fileData->files, index, theReply);
	if (doDeselect)
		/* Deselect any currently selected items... */
		DeselectListItems(fileData->selectedList);
	/* ...select the item we just added... */
	LSetSelect(true, cell, fileData->selectedList);
	/* ...and scroll it into view. */
	LAutoScroll(fileData->selectedList);
	/* If this is the first item we're adding to the list, change the "Open File" button to "Open List" */
	if (index == 0) {
		Str31	openString;
		
		GetIndString(openString, rChangingButtonStrings, kOpenListButton);
		SetDItemTitle(theDialog, dSFMultipleOpenButton, openString);
	}
}

/* Compare string 'aPtr' and 'bPtr', and return 0 if the 'aPtr' string is
   greater than the 'bPtr' string, otherwise return 1. */
pascal short ListMgrCompString(Ptr aPtr, Ptr bPtr, short aLen, short bLen)
{
	if (IUMagString(aPtr, bPtr, aLen, bLen) == 1)
		return 0;
	else
		return 1;
}

/* Add the string 'stringToAdd' to the list 'theList', in alphabetical order
   using the search procedure ListMgrCompString() */
Point InsertInOrder(StringPtr stringToAdd, ListHandle theList)
{
	Point		cell = {0, 0};
	
	(void)LSearch((Ptr)stringToAdd + 1, (short)*stringToAdd, 
		(ListSearchUPP)&ListMgrCompString, &cell, theList);
	(void)LAddRow(1, cell.v, theList);
	LSetCell(stringToAdd + 1, (short)*stringToAdd, cell, theList);
	
	return cell;
}

/* Return 'true' if the specified file type is in the type list */
Boolean IsValidFileType(short numTypes, SFTypeList typeList, OSType fileType)
{
	Boolean	isValid = false;
	short	index;
	
	for (index = 0; !isValid && index < numTypes; index++)
		isValid = fileType == typeList[index];
		
	return isValid;
}

/* If the specified file parameters match a file in the list, return true since it has already been added */
Boolean FileAlreadyAdded(short ioVRefNum, long ioFlParID, StringPtr namePtr, SFMultipleFileData *fileData)
{
	FSSpec			**files;
	short			nfiles;
	SignedByte		state;
	Boolean			found;
	short			i;

	found = false;
	files = ArrayListGetHandle(fileData->files);
	nfiles = ArrayListCount(fileData->files);
	state = HGetState((Handle)files);
	HLock((Handle)files);

	for (i = 0; !found && i < nfiles; i++) {
		/* Check and see if a file is already in the list and return true if it is... */
		found = (ioVRefNum == (*files)[i].vRefNum &&
				ioFlParID == (*files)[i].parID &&
				(EqualString(namePtr, (*files)[i].name, true, true) == true));
	}

	HSetState((Handle)files, state);

	return found;
}

/* Change the hiliting of the list to indicate that it is active and receiving keyboard input */
static pascal void SFMultipleFileActivate(DialogPtr theDialog, short item, Boolean activating, void *myDataPtr)
{
	GrafPtr			savePort;
	PenState			savePen;
	SFMultipleFileData	*fileData = myDataPtr;
	Cell				cell;
	
	GetPort(&savePort);
	SetPort(theDialog);
	GetPenState(&savePen);
	
	activating = (activating != 0);	/* uses 0xFF for true, so convert to Boolean */

	if (item == dSFMultipleSelectedFilesList) {
		Rect		theRect;
		
		fileData->active = activating;
		if (!activating)
			PenPat(&qd.white);
		PenSize(2, 2);
		GetDItemRect(theDialog, item, &theRect);
		InsetRect(&theRect, -3, -3);
		FrameRect(&theRect);
	}

	SetPenState(&savePen);
	SetPort(savePort);
}

/* Filter files that have already been "added" */
static pascal Boolean SFMultipleFileFilter(ParamBlockRec *pb, void *myDataPtr)
{
	CInfoPBRec	*cat = (CInfoPBRec *)pb;		/* this isn't documented but seems to work */
	Boolean		hide;

	if ((pb->fileParam.ioFlAttrib & 0x10) == 0) {	/* param block describes a file */
		hide = FileAlreadyAdded(cat->hFileInfo.ioVRefNum, cat->hFileInfo.ioFlParID, 
							cat->hFileInfo.ioNamePtr, myDataPtr);
	} else {
		hide = false;
	}

	return hide;
}

void ToolBoxInit()
{
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	MaxApplZone();
	InitCursor();
}

/* Quickly flash a button in the dialog */
void FlashDialogItem(DialogPtr theDialog, short theItem)
{
	short	iType;
	Handle	iHandle;
	Rect		iRect;
	long		ignored;

	GetDItem(theDialog, theItem, &iType, &iHandle, &iRect);
	HiliteControl((ControlHandle)iHandle, 1);
	Delay(8, &ignored);
	HiliteControl((ControlHandle)iHandle, 0);
}

/* Return the enclosing rectangle of the specified dialog item, in coordinates
   local to the specified dialog. */
void GetDItemRect(DialogPtr theDialog, short theItem, Rect *iRect)
{
	Handle	iHandle;
	short	iType;
	
	GetDItem(theDialog, theItem, &iType, &iHandle, iRect);
}

/* Make a control active/inactive, or highlight/dim the specified part */
void HiliteDialogItem(DialogPtr theDialog, short theItem, short code)
{
	short	iType; 
	Handle	iHandle; 
	Rect		iRect;

	GetDItem(theDialog, theItem, &iType, &iHandle, &iRect);
	HiliteControl((ControlHandle)iHandle, code);
}

/****************************************************
	SetDialogItemTitle

	Change the text associated with a particular dialog item. This is a little
	tricky since there are two ways to change the text. If you are dealing
	with an EditText or StatText (static text) item, you must call SetIText.
	If you are dealing with a dialog item that is backed up by a Control
	Manager control (like a simple button, radio button, or checkbox), you
	must call SetCTitle.

	We determine what kind of dialog item we are dealing with by calling
	GetDItem. Returned in the �kind� parameter is a number that identifies
	what sort of item we are handling. First, we strip off the upper bit,
	which identifies the item as being enabled or disabled. Once that bit is
	removed, we can examine the kind of the item and act accordingly.

	Notice the special handling we give to controls when we call SetCTitle.
	This is to take care of �excessive flashing� as Online Companion puts it.
	When you call SetCTitle, the control manager first calls HideControl to
	remove the control with its old text from the screen. It then changes the
	control�s title in the ControlRecord, and reshows the control by calling
	ShowControl. At this point, the control is properly shown on the screen
	with its correct, new title.

	However, there�s a little time bomb lurking in the works. When HideControl
	was called, the Control Manager called InvalRect on the area the control
	occupied. Even though ShowControl was later called on the same area and
	everything is drawn correctly, that rectangle is still marked as invalid
	and is incorporated into the update region for the dialog. The event loop
	at the heart of ModalDialog will then get an update event for that area
	and redraw the button _again_! This can cause the button to flicker and
	flash more than we would like. We already know that that area is
	adequately drawn, so we tell the Event Manager to hoof it by validating it
	with a call to ValidRect.
*****************************************************/
void SetDItemTitle(DialogPtr theDialog, short theItem, StringPtr newTitle)
{
	short	iType;
	Handle	iHandle;
	Rect		iRect;

	GetDItem(theDialog, theItem, &iType, &iHandle, &iRect);
	iType &= ~itemDisable;	/* Strip off the enable/disable bit */
	if ((iType == statText) || (iType == editText)) {
		SetIText(iHandle, newTitle);
	} else {
		SetCTitle((ControlHandle)iHandle, newTitle);
		SetPort(theDialog);
		ValidRect(&iRect);
	}
}

/****************************************************
	SetGrayFrameDialogItem	Set the specified dialog item to have a special
						drawing procedure which will simply outline the
						item's rectangle in "gray". Dividing lines can be
						easily defined this way by setting the height or
						width of the user item to 1.
*****************************************************/
void SetGrayFrameDialogItem(DialogPtr theDialog, short theItem)
{
	Rect			iRect;
	Handle		iHandle;
	short		iType;

	GetDialogItem(theDialog, theItem, &iType, &iHandle, &iRect);
	SetDialogItem(theDialog, theItem, iType, (Handle)FrameItemInGray, &iRect);
}

void MySetDialogDefaultItem(DialogPtr theDialog, short theItem, Boolean forceActive)
{
	if (((DialogPeek)theDialog)->aDefItem != theItem) {
		GrafPtr		savePort;
		GrayFrameData	frameData;
	
		GetPort(&savePort);
		SetPort(theDialog);

		/* Erase the old default button */		
		GetPenState(&frameData.penState);
		GetDItemRect(theDialog, ((DialogPeek)theDialog)->aDefItem, &frameData.bounds);
		InsetRect(&frameData.bounds, -4, -4);
		frameData.roundFrame = true;
		frameData.inactive = false;
		frameData.erasingOld = true;
		GrayFrameDrawingProc(0, 0, nil, &frameData);

		((DialogPeek)theDialog)->aDefItem = theItem;
		if (forceActive)
			HiliteDialogItem(theDialog, theItem, kControlActive);
		FrameItemInGray(theDialog, theItem);
		
		SetPort(savePort);
	}
}		

/*******************************************************
	FrameItemInGray() and GrayFrameDrawingProc() are adapted routines based
	primarily on Lloyd Lim�s wonderful method of drawing things in "gray" correctly
	in his "Default" CDEF. He gave me permission to use them here, and for that I am
	indebted! This is the best method I�ve seen for drawing things across multiple
	monitors with different bit depths, etc. Very slick!
*******************************************************/
pascal void FrameItemInGray(WindowPtr theDialog, short theItem)
{
	short		iType;
	ControlHandle	theControl;
	GrayFrameData	frameData;
	RGBColor		foreColor;
	AuxCtlHandle	theAuxCtl;

	GetPenState(&frameData.penState);

	GetDItem(theDialog, theItem, &iType, (Handle *)&theControl, &frameData.bounds);
	if ((iType & ~itemDisable) == (ctrlItem | btnCtrl)) {
		frameData.roundFrame = true;
		InsetRect(&frameData.bounds, -4, -4);
		frameData.inactive = ((*theControl)->contrlHilite == kControlInactive);
	} else {
		frameData.roundFrame = false;
		frameData.inactive = true;	/* Since we want to draw userItem outlines in gray */
	}
	frameData.erasingOld = false;

	if (gQuickDrawVersion >= gestalt8BitQD) {
		GetForeColor(&foreColor);
		if (frameData.roundFrame) {
			(void)GetAuxCtl(theControl, &theAuxCtl);
			frameData.frameColor = (*(*theAuxCtl)->acCTable)->ctTable[cFrameColor].rgb;
		} else {
			frameData.frameColor = foreColor;
		}
	}
	if (gQuickDrawVersion >= gestalt32BitQD13 && theDialog->portBits.rowBytes & 0xC000) {
		GetBackColor(&frameData.backColor);
		DeviceLoop(theDialog->visRgn, (DeviceLoopDrawingProcPtr)GrayFrameDrawingProc,
			(long)&frameData, 0);
	} else {
		GrayFrameDrawingProc((gQuickDrawVersion >= gestalt8BitQD), 0, nil, &frameData);
	}
	if (gQuickDrawVersion >= gestalt8BitQD) {
		RGBForeColor(&foreColor);
	}
}

static pascal void GrayFrameDrawingProc(short depth, short deviceFlags, GDHandle targetDevice,
	 GrayFrameDataPtr frameData)
{
	RGBColor		outlineColor;
	short		radius;
	
	PenNormal();

	if (frameData->erasingOld) {
		PenPat(&qd.white);
	} else {
		if (depth)
			outlineColor = frameData->frameColor;
		if (frameData->inactive)
			if (targetDevice == nil || GetGray(targetDevice, &frameData->backColor, &outlineColor) == false)
				PenPat(&qd.gray);
		if (depth)
			RGBForeColor(&outlineColor);
	}
	if (frameData->roundFrame) {
		PenSize(3, 3);
		if ((radius = ((frameData->bounds.bottom - frameData->bounds.top) >> 1)) < 16)
			radius = 16;
		FrameRoundRect(&frameData->bounds, radius, radius);
	} else {
		FrameRect(&frameData->bounds);
	}

	SetPenState(&frameData->penState);
}

/* This is just a sample error handler... that is totally lame! You really need to look
   up the error messages in a string resource and stuff, not use string constants like
   I've shown here! (Hey, this wasn't a demo app showing how to handle errors! :-) */
void ErrorHandler(StringPtr errorString)
{
	short	itemHit;

	SetCursor(&qd.arrow);
	SetDAFont(systemFont);
	ParamText(errorString, nil, nil, nil);
	itemHit = StopAlert(rErrorAlert, nil);
	ExitToShell();
}
