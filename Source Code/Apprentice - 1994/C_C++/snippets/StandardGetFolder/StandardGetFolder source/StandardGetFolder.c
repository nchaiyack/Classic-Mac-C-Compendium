/*************************************************************************************************

StandardGetFolder.c -- Copyright Chris Larson, 1993 -- All rights reserved.
                       Based partly upon StandardGetFolder example by Steve Falkenburg (MacDTS)
                       and partly on the code in Inside Macintosh: Files.
                       
                       Dialog box layouts taken from Inside Macintosh: Files.
                       
  To Do:	* Better method of replacing Pop-up directory hierarchy help strings.
			* Don’t show the temporary items folder(s).
			* Allow selection of folder with only write privileges.

	Bug Reports/Comments to cklarson@engr.ucdavis.edu

	Version 1.0

*************************************************************************************************/

// ----------
// Define this symbol to exclude various unneeded glue routines.
// ----------

#ifdef SystemSevenOrLater
#undef SystemSevenOrLater
#endif
#define SystemSevenOrLater 1

// ----------
// Include Files
// ----------

#include "StandardGetFolder.h"
#include "SGF.h"

// ----------
// Global Variables
// ----------

static StringHandle				gLeftString;		// Left portion of the select button title.
static StringHandle				gRightString;		// Right portion of the select button title.
static StringHandle				gDesktopName;		// Replacement name for the desktop folder.
static short					gDeskVRefNum;		// Volume reference number of the desktop folder.
static long						gDeskDirID;			// Directory ID of the desktop folder.
static short					gSaveSFSaveDisk;	// Temporarily holds the value of SFSaveDisk.
static long						gSaveCurDirStore;	// Temporarily holds the value of CurDirStore.
static char						gSelectKey;			// Holds the character code of the select key.
static Str255					gPreviousName;		// Holds the name last used in the button title.
static StandardFolderReplyPtr	gFolderReply;		// Pointer to the standard folder reply.
static WindowFunc				gWindowProc;		// Application window event handler function.
static long						gTrapAddress;		// Holds the true address of _GetResource.
static RgnHandle				gSaveRgn;			// Temporarily saves the main dialog’s ClipRgn.
static Boolean					gSelectButtonHit;	// Indicates that select button was just hit.
static Boolean					gCompareNames;		// Flag indicating the need to reexamine the
													//    current title of the select button.

/****************************************************************************************************

StandardGetFolder -- The 'main' routine

****************************************************************************************************/

OSErr StandardGetFolder (Str255 prompt, StandardFolderReplyPtr reply, ProcPtr windowProc)
{	
	StringHandle 		selectKeyString;
	Point				location = {-1,-1};
	StandardFileReply	newReply;

	// ----------
	// Initialize some variables.
	// ----------

	reply->sfGood = false;
	gWindowProc = windowProc;
	gCompareNames = true;
	gSelectButtonHit = false;
	gFolderReply = reply;
	gSaveRgn = NULL;
	newReply.sfIsVolume = 0;
	newReply.sfIsFolder = 0;
	newReply.sfFlags = 0;

	// ----------
	// Find and store the information about the desktop folder. Assume an error by FindFolder implies
	// that the desktop folder does not exist.
	// ----------

	if (FindFolder(kOnSystemDisk,kDesktopFolderType,kDontCreateFolder,&gDeskVRefNum,
			&gDeskDirID) != noErr)
		gDeskVRefNum = kNullVol;

	// ----------
	// Load and store the select key character code. If an error occurs, return the error code.
	// ----------

	if ((selectKeyString = GetString(rSelectKeyStringID)) != NULL)
		{
		gSelectKey = (*selectKeyString)[1];
		ReleaseResource ((Handle)selectKeyString);
		}
	else
		return (ResErr);

	// ----------
	// Load the replacement title for the desktop folder. If an error occurs, return the error code.
	// ----------

	if ((gDesktopName = GetString(rDesktopNameStringID)) == NULL)
		return (ResErr);

	// ----------
	// Load the wrapper strings for the select button title. If an error occurs, return the error
	// code.
	// ----------

	if ((gLeftString = GetString(rLeftStringID)) == NULL)
		return (ResErr);

	if ((gRightString = GetString(rRightStringID)) == NULL)
		return (ResErr);

	// ----------
	// Lock the strings. This is necessary since StringWidth() may move memory and I pass it
	// dereferenced handles. The strings are locked after all are successfully loaded to prevent
	// orphaning a locked block high in the heap if an error were to occur while loading the second
	// or third string.
	// ----------

	HLockHi ((Handle)gDesktopName);
	HLockHi ((Handle)gLeftString);
	HLockHi ((Handle)gRightString);

	// ----------
	// If we were given a prompt, store it in gPreviousName (on entry to CustomGetFile, there will be
	// no previous name so this space is usable).
	// ----------

	if (prompt)
		PStringCopy (gPreviousName,prompt);
	else
		*gPreviousName = 0;

	// ----------
	// Install the patch on _GetResource to allow our replacement help strings for the pop-up
	// directory hierarchy to be used (** If anyone knows of a better way to replace these strings I
	// would appreciate hearing about it **).
	// ----------

	gTrapAddress = GetToolTrapAddress(_GetResource);
	SetToolTrapAddress((long)Patch,_GetResource);

	// ----------
	// Call CustomGetFile with the appropriate parameters.
	// ----------

	CustomGetFile((ProcPtr)&GetFolderFileFilterYD,kAllFiles,NULL,&newReply,rGetFolderDialogID,
					location,(ProcPtr)&GetFolderDialogHookYD,(ProcPtr)&GetFolderModalFilterYD,
					NULL,NULL,&newReply);

	// ----------
	// Remove the trap patch immediately after exiting CustomGetFile.
	// ----------

	SetToolTrapAddress(gTrapAddress,_GetResource);

	// ----------
	// Get rid of the strings.
	// ----------

	ReleaseResource((Handle)gDesktopName);
	ReleaseResource((Handle)gLeftString);
	ReleaseResource((Handle)gRightString);

	// ----------
	// Everything’s OK, so return noErr.
	// ----------

	return (noErr);
}

/****************************************************************************************************

FileFilter -- Return false if the file is to be displayed (i.e. its directory bit is set and its
              invisible bit is not set).

****************************************************************************************************/

pascal Boolean GetFolderFileFilterYD (CInfoPBPtr paramBlock, StandardFileReply *myData)
{
	return(!((((paramBlock->hFileInfo).ioFlAttrib)&kIsFolderFlag)&&
			(!(((paramBlock->dirInfo).ioDrUsrWds.frFlags)&kInvisibleFlag))));
}

/****************************************************************************************************

ModalFilter -- Handle update and activate events for standard file dialogs, pass update and activate
			   events for application windows on to the application (via the function pointer passed
			   to StandardGetFolder) if a routine is given. Also, convert Command - Select Key to a
			   press of the select button and flag mouse clicks within the file list to cause the
			   select button to correct the name it displays.
               
****************************************************************************************************/

pascal Boolean GetFolderModalFilterYD (DialogPtr theDialog, EventRecord *theEvent, short *itemHit,
										StandardFileReply *myData)
{
	Boolean		result = false;		// Initialize to indicate that the event was not handled.
	short		itemType;
	Handle		itemHandle;
	Rect		itemRect;
	Point		localPoint;
	GrafPtr		savePort;

	// ----------
	// Start with activate events. These must be processed no matter which dialog is in front, so this
	// block comes before the check to see which dialog is in front. Note that no matter to what
	// degree the activate event is handled, the modal filter must return false (as per IM: Macintosh
	// Toolbox Essentials).
	// ----------

	if (theEvent->what == activateEvt)
		{
		
		// ----------
		// If the main dialog is the target of the activate event, the select button must be either
		// activated or dimmed according to whether the dialog is being activated or deactivated.
		// ----------

		if (((WindowPeek)(theEvent->message))->refCon == sfMainDialogRefCon)
			{
			GetDItem ((DialogPtr)(theEvent->message),sfItemSelectFolderButton,&itemType,&itemHandle,
					&itemRect);
			if ((theEvent->modifiers)&activeFlag)
			
				// ----------
				// We are being activated, so un-dim the select button.
				// ----------

				HiliteControl((ControlHandle)itemHandle,kNormal);
			else
				{
				
				// ----------
				// The selection cannot change while the dialog is deactivated, so flag the select
				// button title as not needing examination.
				// ----------

				gCompareNames = false;

				// ----------
				// If the main dialog is being deactivated and the select button was just hit,
				// an error must have ocurred while trying to open the selected item. Therefore,
				// clear the select button press, restore the main dialog’s clip region and allow
				// standard file to display the error dialog. After the user dismisses the error
				// dialog, processing will continue as if the select button was never pressed.
				// ----------

				if (gSelectButtonHit)
					{
					gSelectButtonHit = false;
					GetPort(&savePort);
					SetPort((DialogPtr)(theEvent->message));
					SetClip(gSaveRgn);
					DisposeRgn(gSaveRgn);
					gSaveRgn = NULL;
					SetPort(savePort);
					}

				// ----------
				// We are being deactivated, so dim the select button.
				// ----------

				HiliteControl((ControlHandle)itemHandle,kInactive);
				}
			}

		// ----------
		// If the main dialog is not the target of the activate event, check to see if the error
		// dialog is the target. If the error dialog is the target, I have nothing to do. If the
		// error dialog is not the target, then the target must be an application window (since the
		// error dialog and the main dialog are the only two dialogs StandardGetFolder will ever
		// present) so call the application’s window event handler.
		// ----------

		else if (((WindowPeek)(theEvent->message))->refCon != sfErrorDialogRefCon)
			CallAppWindowFunction(theEvent);
		}

	// ----------
	// Now Update events. Much like activate events, these must be handled no matter which dialog is
	// in front and the modal filter must return false (again, as per IM: Mac Toolbox Essentials).
	// ----------

	if (theEvent->what == updateEvt)
		{
		
		// ----------
		// If the target of the update event is the dialog currently in front, standard file will
		// handle the event correctly, so return false immediately.
		// ----------

		if ((DialogPtr)(theEvent->message) == theDialog)
			return (false);
		
		// ----------
		// If the target of the update event is not the frontmost dialog, check to see if the target
		// is the main dialog. Since standard file will not update its own dialog boxes correctly if
		// they are not the frontmost window (Grrrr…), I must update the dialog myself. Note that the
		// only case where one standard file dialog is obscured by another is when the main dialog is
		// behind the error dialog. Thus this case is detectable simply by checking to see if the
		// target is the main dialog. (If the main dialog were the front dialog, the previous check
		// would have returned false, so implicit in this is that the main dialog is not in front.)
		// ----------

		if (((WindowPeek)(theEvent->message))->refCon == sfMainDialogRefCon)
			{
			BeginUpdate ((DialogPtr)(theEvent->message));
			UpdateDialog((DialogPtr)(theEvent->message),((WindowPtr)(theEvent->message))->visRgn);
			EndUpdate((DialogPtr)(theEvent->message));
			return (false);
			}
		
		// ----------
		// If the target is neither the frontmost dialog nor the main dialog, it must be a window
		// belonging to the application, so call the application’s window event handler.
		// ----------

		CallAppWindowFunction(theEvent);
		}

	// ----------
	// The remaining checks are only relevant if the main dialog is the frontmost, so return if the
	// frontmost dialog is not the main dialog.
	// ----------

	if (GetWRefCon((WindowPtr)theDialog) != sfMainDialogRefCon)
		return (false);

	// ----------
	// Check to see if the select key was pressed while the command key was held down. If so, fake a
	// press of the select button and return true, indicating that the event was handled.
	// ----------

	if ((theEvent->what == keyDown)||(theEvent->what == autoKey))
		{
		if (((theEvent->modifiers)&cmdKey)&&(((theEvent->message)&charCodeMask) == gSelectKey))
			{
			*itemHit = sfItemSelectFolderButton;
			HitButton (theDialog,sfItemSelectFolderButton);
			result = true;
			}
		}

	// ----------
	// Since pressing the mouse in an empty area of the file list changes the file selection (changes
	// it to no selection) but is passed to the dialog hook as a null event (Grrrr… part 2) I must
	// flag all clicks within the file list to cause examination of the select button title.
	// ----------

	if (theEvent->what == mouseDown)
		{
		localPoint = theEvent->where;
		GlobalToLocal (&localPoint);
		GetDItem (theDialog,sfItemFileListUser,&itemType,&itemHandle,&itemRect);
		if (PtInRect(localPoint,&itemRect))
			gCompareNames = true;
		}

	return (result);
}

/****************************************************************************************************

DialogHook -- Set the select button title as appropriate, manage presses of the select button.

****************************************************************************************************/

pascal short GetFolderDialogHookYD (short itemHit, DialogPtr theDialog, StandardFileReply *myData)
{	
	CInfoPBRec	paramBlock;
	Str255		dirName;
	short		itemType;
	Handle		itemHandle;
	Rect		itemRect;
	GrafPtr		savePort;
	RgnHandle	nullRgn;

	// ----------
	// The dialog hook should only run if the main dialog is in front, so exit immediately if that is
	// not the case.
	// ----------

	if (GetWRefCon((WindowPtr)theDialog) != sfMainDialogRefCon)
		return (itemHit);

	// ----------
	// Repair what I consider to be a bug in standard file: if the file list is displaying the desktop
	// and the last thing that was selected (whether it is still selected or there is no selection) is
	// a volume other than the startup volume, the lo-mem globals hold the information describing the
	// root directory of the last selected volume _not_ the information describing the desktop folder,
	// which is the displayed directory. This bug can cause incorrect behavior: when a standard file
	// dialog is dismissed and immediately recalled, the second dialog should open displaying the same
	// directory as the first did when dismissed. Try this on your favorite standard file application:
	// call up the dialog, insert a floppy, go to the desktop and highlight the floppy. Then cancel
	// the dialog and immediately call it up again, *bam* it opens displaying the root directory of
	// the floppy _not_ the desktop (as it should). Thus, in an attempt to remedy this behavior (and
	// to make my module work correctly, since it relies on the values of the lo-mem globals) I reset
	// the globals to describe the desktop whenever the desktop button is dimmed.
	// ----------
	
	GetDItem (theDialog,sfItemDesktopButton,&itemType,&itemHandle,&itemRect);
	if ((**((ControlHandle)itemHandle)).contrlHilite == kInactive)
		{
		SFSaveDisk = -gDeskVRefNum;
		CurDirStore = gDeskDirID;
		}
	
	// ----------
	// If the select button was just pressed (and not cleared by the modal filter), the selected
	// item was opened without an error, so it is a valid selection. Clear the select button press
	// (so that the DisposeRgn() call is only made once), release the saved region, fill in the
	// reply record, restore the contents of the lo-mem globals, and fake a press of the cancel
	// button.
	// ----------
	
	if (gSelectButtonHit)
		{
		gSelectButtonHit = false;
		DisposeRgn(gSaveRgn);
		gFolderReply->sfVRefNum = -SFSaveDisk;
		gFolderReply->sfDirID = CurDirStore;
		gFolderReply->sfGood = true;
		
		SFSaveDisk = gSaveSFSaveDisk;
		CurDirStore = gSaveCurDirStore;
		
		return (sfItemCancelButton);
		}
	
	// ----------
	// Process events normally.
	// ----------

	switch (itemHit)
		{
		
		// ----------
		// If the select button was pressed, force the use of standard file’s error checking.
		// By instructing standard file to open the selected item, I can make use of the built-in
		// error mechanism of the standard file package and therefore achieve two goals: avoid
		// having to write one myself, and maintain absolute consistency over error messages.
		// This method does involve a little trickery however: since I force standard file to open
		// the selected item, the appearance of the main dialog would change in response to the
		// new directory information and the lo-memory globals will be changed to describe the
		// just-opened directory. No problem. To ensure that the display of the main dialog does not
		// change I save off its clip region (in case an error occurs and it must be restored) and
		// set the clip region to the empty region -- no drawing will occur. To make sure that the
		// lo-mem globals will describe the last seen directory (not necessarily the selected one)
		// upon exit of this routine (so that the next standard file dialog will open in the
		// directory last displayed by this one, barring munging by SuperBoomerang, etc.), I save 
		// off and restore their values.
		// ----------

		case sfItemSelectFolderButton:

			gSelectButtonHit = true;
			GetPort(&savePort);
			SetPort(theDialog);
			
			gSaveRgn = NewRgn();
			nullRgn = NewRgn();
			
			GetClip(gSaveRgn);
			SetClip(nullRgn);
			
			DisposeRgn(nullRgn);
			
			SetPort(savePort);

			gSaveSFSaveDisk = SFSaveDisk;
			gSaveCurDirStore = CurDirStore;

			// ----------
			// If the selected item is a folder, volume, or alias, open it. If there is no selection,
			// the selected directory is already displayed, so do nothing.
			// ----------

			if ( (myData->sfIsFolder) || (myData->sfIsVolume) )
				return (sfHookOpenFolder);
			else if ( (myData->sfFlags) & kIsAliasFlag)
				return (sfHookOpenAlias);
			else
				return (sfHookNullEvent);

		// ----------
		// If this is the first time through, set the prompt to the given text, if any was given.
		// ----------

		case sfHookFirstCall:
			if (*gPreviousName != 0)
				{
				GetDItem(theDialog,sfItemPromptStatText,&itemType,&itemHandle,&itemRect);
				SetIText(itemHandle,gPreviousName);
				*gPreviousName = 0;
				}

		// ----------
		// If this is a null event and the select button title does not need to be examined, exit the
		// switch statement.
		// ----------

		case sfHookNullEvent:
			if (!gCompareNames)
				break;

		// ----------
		// Otherwise, the select button title needs to be examined (either the event is not a null
		// event or we have flagged the title as needing examination).
		// ----------

		default:
		
			// ----------
			// If the selection is an alias, a folder, or a volume, use the name of the selected
			// item.
			// ----------

			if ((myData->sfIsFolder)||(myData->sfIsVolume)||((myData->sfFlags)&kIsAliasFlag))
				PStringCopy (dirName,(myData->sfFile).name);
		
			// ----------
			// Otherwise, use the name of the currently displayed directory.
			// ----------
			
			else
				{
				
				// ----------
				// If the currently displayed directory is the desktop, substitute our string for the
				// name of the desktop folder.
				// ----------
	
				if ((gDeskVRefNum == -SFSaveDisk)&&(gDeskDirID == CurDirStore))
					PStringCopy(dirName,*gDesktopName);
				
				// ----------
				// Otherwise, query the volume catalog to get the name of the currently displayed
				// directory.
				// ----------

				else
					{
					paramBlock.dirInfo.ioCompletion = NULL;
					paramBlock.dirInfo.ioNamePtr = dirName;
					paramBlock.dirInfo.ioFDirIndex = -1;
					paramBlock.dirInfo.ioVRefNum = -SFSaveDisk;
					paramBlock.dirInfo.ioDrDirID = CurDirStore;
					
					if (PBGetCatInfoSync(&paramBlock) != noErr)
						return(itemHit);
					}
				}

			// ----------
			// Compare the current name with the name last placed into the select button. If they
			// differ, set the select button title to reflect the current name. EqualString() is
			// used to perform the name comparison since it follows the comparison rules of the
			// Macintosh file system.
			// ----------

			if (EqualString(dirName,gPreviousName,false,false) == false)
				{
				GetDItem(theDialog,sfItemSelectFolderButton,&itemType,&itemHandle,&itemRect);
				SetButtonTitle((ControlHandle)itemHandle,dirName,&itemRect,theDialog);
				}

			// ----------
			// If this was not a null event, flag the select button title as needing examination
			// (since the selection can change after a call to the dialog hook with a non-null event
			// this will force those cases to be caught).
			// ----------

			gCompareNames = (itemHit != sfHookNullEvent);
			
			// ----------
			// If a press of the open button (meaning open a file -- opens associated with aliases or
			// directories are mapped to pseudo-items) happens to creep through (yes, it’s possible:
			// it can happen if return or enter is pressed soon after inserting a floppy when there is
			// a folder or alias highlighted prior to inserting the floppy) invalidate it by mapping it
			// to a null event.
			// ----------
			
			if (itemHit == sfItemOpenButton)
				itemHit = sfHookNullEvent;
		}

	return (itemHit);
}

/****************************************************************************************************

CallAppWindowFunction -- Passes an event to the application’s window function, if one was given. If
						 one was not given, the event is mapped to a null event.

****************************************************************************************************/

void CallAppWindowFunction (EventRecord *theEvent)
{
	GrafPtr	savePort;
	
	if (gWindowProc != NULL)
			{
			
			// ----------
			// Save and restore the current port in case the application leaves the port set to an
			// application window.
			// ----------

			GetPort(&savePort);
			(*gWindowProc)(theEvent);
			SetPort(savePort);
			}
	else 
		theEvent->what = nullEvent;
}

/****************************************************************************************************

SetButtonTitle -- Sets the given button's title to the concatenation (gLeftString + newTitle +
                  gRightString) where the newTitle string will be end-truncated to fit the
                  entire concatenation within the button's rectangle. End-truncation is used to
                  match the behavior of the pop-up menu atop the main standard file dialog.

****************************************************************************************************/

void SetButtonTitle (ControlHandle theButton, Str255 newTitle, Rect *buttonRect, DialogPtr theDialog)
{
	Str255		buttonTitle;
	short		width;

	// ----------
	// Place the name we are changing to into our comparison buffer
	// ----------

	PStringCopy (gPreviousName,newTitle);

	// ----------
	// Set width to the width of the select button less the lengths of the wrapper strings and less
	// a tad more (the title offset) to provide a little leeway.
	// ----------

	width = buttonRect->right - buttonRect->left - StringWidth(*gLeftString) -
			StringWidth (*gRightString) - kTitleOffset;

	// ----------
	// End-truncate the string
	// ----------

	TruncString(width,newTitle,smTruncEnd);
	
	// ----------
	// Only set a new title if the truncated filename has at least one character (yes it is possible
	// that this routine gets passed a null filename -- one instance in which it happens is right
	// after inserting a floppy, for one pass, standard file thinks the floppy is a volume with the
	// null string for a title).
	// ----------
	
	if (*newTitle != 0)
		{

		// ----------
		// Perform the concatenation.
		// ----------
	
		PStringCopy (buttonTitle,*gLeftString);
		PStringCat (buttonTitle,newTitle);
		PStringCat (buttonTitle,*gRightString);
	
		// ----------
		// Set the button’s title (note that SetCTitle causes the control to be drawn and then causes
		// the control’s rectangle to be marked invalid so immediately after changing the title, I
		// validate the rectangle to prevent the control from being drawn twice.
		// ----------
	
		SetCTitle (theButton,buttonTitle);
		SetPort (theDialog);
		ValidRect (buttonRect);
		}
}

/****************************************************************************************************

HitButton -- Fake a button press by inverting the button for a given time interval.

****************************************************************************************************/

void HitButton (DialogPtr theDialog, short itemNumber)
{
	short	itemType;
	Handle	buttonHandle;
	Rect	buttonRect;
	long	ignored;

	GetDItem (theDialog,itemNumber,&itemType,&buttonHandle,&buttonRect);
	HiliteControl ((ControlHandle)buttonHandle,kInverted);
	Delay (kHitButtonDelay,&ignored);
	HiliteControl ((ControlHandle)buttonHandle,kNormal);
}

/****************************************************************************************************

Patch -- Allows the use of our replacement help strings for the pop-up directory list. In order to
         replace these help strings, _GetResource must be patched (** once again, if you know of a
         better way I’d love to hear it **). This patch is installed immediately before the call to
         CustomGetFile and removed immediately after. It simply watches for attempts to get a handle
         to the default help strings and alters the call to get the replacement strings.

****************************************************************************************************/

pascal Handle Patch (ResType type, short id)
{	
	// ----------
	// If the default string resource is being requested, alter the request to return our replacement
	// strings.
	// ----------

	if ((type == 'STR#')&&(id == kExistingPopUpStrings))
		id = kNewPopUpStrings;

	// ----------
	// When we are called, A5 may not be valid. Since patching a trap from an application only effects
	// the application, we can assume that CurrentA5 will hold the address of our A5 world. We need
	// access to the A5 world to get at the correct address of _GetResource, since it is stored as a
	// global variable and global variables are accessed as offsets from A5.
	// ----------

	asm 68000
		{
		move.l	a5,-(a7)		; push the value of a5
		move.l	CurrentA5,a5	; load the address of our a5 world
		move.l	gTrapAddress,a0	; load the address of _GetResource into a0
		move.l	(a7)+,a5		; restore the value of a5
		unlk	a6				; remove the a6 link
		jmp		(a0)			; jump to _GetResoruce
		}
}
