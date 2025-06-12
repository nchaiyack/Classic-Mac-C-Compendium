/*			SpinCursors by
 *			America Online: LISPer
 *		 	Internet: tree@uvm.edu
*/
#ifndef _FOLDERS_
#include <Folders.h>
#endif
#include <Script.h>

#include "thePrefs.h"
#include "MacErrors.h"


/************************************************************************************/

#define kPrefsDialogID		130
#define kSaveButtonItem		1
#define kCancelButtonItem	2

#define kBeepWD				14
#define kQuitWD				5

#define kForce				7

#define kTEXTsuffix			3
#define kTEXTcreator		4
#define kTEXTlevel			10

#define kUpArrow			11
#define kDownArrow			12

/* prefs file constants */
#define kPrefsNameStrID		128
#define kPrefsCreatorType	'\?\?\?\?'
#define kPrefsFileType		'pref'
#define kPrefsResourceType	'Pref'
#define kPrefsResourceID	128

#define kFinderMessageStrID	-16397 /* ID of STR for default finder message */
                                   /* see Inside Mac VI, page 9-22         */
#define kStrType			'STR '

#define CmdKeyMask			0x0100

/*
 * From DialogBits,C.K. Haun
 * Apple Developer Tech Support
 */
 
 /* key equates */
enum  {
    kEnterKey		= 0x03, 
	kTabKey			= 9, 
	kReturnKey		= 0x0D, 
	kBackSpace		= 8, 
	kEscKey			= 0x1B, 
	kLeftArrowKey	= 0x1C, 
	kRightArrowKey, 
	kUpArrowKey,
    kDownArrowKey, 
	kDeleteKey		= 0x7F
};

pascal Boolean filterIt(DialogPtr inputDialog, EventRecord *myDialogEvent, short *theDialogItem);
 
pascal OSErr GetStdFilterProc(ModalFilterProcPtr *theProc) = 
{
    0x303C, 0x0203, 0xAA68
};

/* Indicates to the dialog manager which item is default.  Will then alias the return key */
/* to this item, and also bold border it for you (yaaaaa!) */
pascal OSErr SetDialogDefaultItem(DialogPtr theDialog, short newItem)
= 
{
    0x303C, 0x0304, 0xAA68
};

/* Indicates which item should be aliased to escape or Command - . */
pascal OSErr SetDialogCancelItem(DialogPtr theDialog, short newItem)
= 
{
    0x303C, 0x0305, 0xAA68
};
/* End From DialogBits */

/* these constants may be stored in the fields of the prefs struct */

OSErr GetPrefsName (StringPtr prefName);

PrefsType currPrefs;
Boolean prefsChangedFlag;

short OpenPrefsResFile(SignedByte prefsPerm, Boolean createFlag)
/* open the preferences file with the given permission; if createFlag is set,
   create a preferences file if necessary */
{
	OSErr retCode;
	short prefsVRefNum;
	long prefsDirID;
	Str255 prefsNameStr;
	FSSpec prefsFSSpec;
	short prefsResRefNum = -1;
	
	/* get the name of the prefs file */
	retCode = GetPrefsName(prefsNameStr);
	if (retCode == noErr) {
	
		/* Use FindFolder to locate the Preferences folder.
		
		   MPW 3.2 and some other compilers provide the
		   glue necessary to use FindFolder (and Gestalt) safely
		   under System 6 and earlier.  Under System 6, the dirID
		   returned for the Preferences folder request will be
		   the System folder's dirID */

		retCode = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
			&prefsVRefNum, &prefsDirID);
		if (retCode == noErr) {
		
			/* make a file spec for the prefs file */
			
			retCode = FSMakeFSSpec(prefsVRefNum, prefsDirID, prefsNameStr,
				&prefsFSSpec);
				
			if (retCode == fnfErr && createFlag) {
				/* prefs file doesn't already exist, so create it */
				FSpCreateResFile(&prefsFSSpec, kPrefsCreatorType, kPrefsFileType,
					smSystemScript);
				retCode = ResError();
			}
			
			/* open the prefs file */
			if (retCode == noErr) {
				prefsResRefNum = FSpOpenResFile(&prefsFSSpec, prefsPerm);
			}
		}
	}
	return prefsResRefNum;
}

OSErr GetPrefsName (StringPtr prefName)
/* get the name of the preferences file from a resource in the application */
{
	StringHandle prefStrHandle;
	OSErr retCode = noErr;
	
	prefStrHandle = GetString(kPrefsNameStrID);
	if (prefStrHandle != nil) 
		BlockMove(*prefStrHandle, prefName, (Size) (**prefStrHandle) + 1);
	else retCode = resNotFound;
	
	return retCode;
}

Boolean GetOrGeneratePrefs( PrefsType * thePrefsTypePtr, long wantedVers )
/* fill in the prefs structure from the resource file or from scratch */
{
	short prefsResRefNum;
	Handle tempHandle;
	extern	Boolean		gSavedPos;
	extern	Point		gSavedPoint;
	Str255 defaultStr = "\pttxt";
	
	/* initialize prefs structure in case we can't get a valid set */
	thePrefsTypePtr->prefsTypeVers = wantedVers;
	thePrefsTypePtr->compress = false;
	thePrefsTypePtr->ascii = false;
	thePrefsTypePtr->level = 6;
	thePrefsTypePtr->force = false;
	thePrefsTypePtr->suffix[0]=0x00;
	BlockMove(defaultStr, thePrefsTypePtr->textcreator, *defaultStr+1);
	thePrefsTypePtr->QuitWhenDone = true;
	thePrefsTypePtr->BeepWhenDone = false;
	thePrefsTypePtr->SavedPos = false;
	thePrefsTypePtr->SavedPoint.h = 0;
	thePrefsTypePtr->SavedPoint.v = 0;
	
	
	/* open (but don't create) the prefs file */
	prefsResRefNum = OpenPrefsResFile(fsRdPerm, false);
	if (prefsResRefNum != -1) {
	
		/* file opened successfully, get the prefs resource */
		tempHandle = Get1Resource(kPrefsResourceType, kPrefsResourceID);
		
		/* if the resource is there and it's the right size and version, copy it */
		/* (in C these can be combined with &&, but don't use AND in Pascal)     */
		
		if (tempHandle != nil)
			if (GetHandleSize(tempHandle) == sizeof(PrefsType)) 
				if ((*(PrefsType *)*tempHandle).prefsTypeVers == wantedVers) {
				
					/* copy the prefs struct */
					*thePrefsTypePtr = *(PrefsType *)*tempHandle;
					
				}
		/* release the pref resource and close the file */
		CloseResFile(prefsResRefNum);

		gSavedPos = thePrefsTypePtr->SavedPos;
		gSavedPoint.h = thePrefsTypePtr->SavedPoint.h;
		gSavedPoint.v = thePrefsTypePtr->SavedPoint.v;

	return true;
	
	}
	else
		return false;
}
OSErr SavePrefs(PrefsType * thePrefsTypePtr )
/* save the prefs structure in the prefs resource file */
{
	OSErr retCode;
	short prefsResRefNum;
	Handle prefHandle, finderMessageHandle;
	
	/* open (and, if necessary, create) the prefs file */
	prefsResRefNum = OpenPrefsResFile(fsRdWrPerm, true);
	if (prefsResRefNum != -1) {
	
		/* file opened successfully, get the prefs resource */
		prefHandle = Get1Resource(kPrefsResourceType, kPrefsResourceID);
		
		if (prefHandle == nil) {
		
			/* create a new resource */
			prefHandle = NewHandle(sizeof(PrefsType));
			if (prefHandle != nil) {
			
				/* copy the prefs struct into the handle
				   and make it into a resource */
				
				*(PrefsType *)*prefHandle = *thePrefsTypePtr;
				AddResource(prefHandle, kPrefsResourceType, kPrefsResourceID, 
					"\pPrefsType");
				retCode = ResError();
				if (retCode != noErr) DisposHandle(prefHandle);
			} 
			
			else retCode = MemError(); /* NewHandle failed */
		}
		
		else {  /* prefHandle != nil */
		
			/* update the existing resource */
			SetHandleSize(prefHandle, sizeof(PrefsType));
			retCode = MemError();
			if (retCode == noErr) {

				/* copy the prefs struct into the handle and tell the rsrc manager */
				*(PrefsType *)*prefHandle = *thePrefsTypePtr;
				ChangedResource(prefHandle);
			}
		}
		if (retCode == noErr) {
		
			/* add the message to be displayed if the user tries
			   to open the prefs file in the Finder (but don't add it
			   if it's already in the preferences file) */
			   
			finderMessageHandle = (Handle) GetString(kFinderMessageStrID);
			if (finderMessageHandle != nil &&
				HomeResFile((Handle) finderMessageHandle) != prefsResRefNum) {
			
				/* copy the resource into the prefs file */
				DetachResource(finderMessageHandle);
				AddResource(finderMessageHandle, kStrType, kFinderMessageStrID,
					"\pFinder message");
					
				/* if AddResource failed, dispose of the handle */
				retCode = ResError();
				if (retCode != noErr) DisposHandle(finderMessageHandle);
			}
		}
		
		/* update and close the preference resource file, 
		   releasing its resources from memory */
		CloseResFile(prefsResRefNum);
	}
	
	else {
		/* couldn't open the res file */
		retCode = ResError();
		if (retCode == noErr) retCode = resFNotFound;
	}
	
	return retCode;
}
/***********************Dialog*************/
Boolean DoPrefsDialog(PrefsType * thePrefsTypePtr)
/* display the prefs dialog; returns true if change made to prefs record */
{
	GrafPtr savePort;
	DialogPtr prefDialogPtr;
	short item;
	Boolean doneFlag = false;
	OSErr retCode;

	/* keep the new preference settings only in local variables
	   in case the user cancels */
	   
	PrefsType newPrefsType;

	short		itemType;
	Handle		itemHandle;
	Rect		itemRect;
	Str255		sTemp;
	
	Boolean changedFlag;


	/* the old settings are our starting point, so copy them */
	newPrefsType = *thePrefsTypePtr;
	
	/* throw up the dialog */
	prefDialogPtr = GetNewDialog(kPrefsDialogID, nil, (WindowPtr) -1);
	if (prefDialogPtr == nil) ExitToShell();  /* hurtinÕ bad */
	
	SetDialogDefaultItem( prefDialogPtr, kSaveButtonItem);
	SetDialogCancelItem( prefDialogPtr, kCancelButtonItem);

	/* set up the dialog to the initial values */

	GetDItem(prefDialogPtr, kForce, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.force ? 1 : 0));

	GetDItem(prefDialogPtr, kBeepWD, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.BeepWhenDone ? 1 : 0));

	GetDItem(prefDialogPtr, kQuitWD, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.QuitWhenDone ? 1 : 0));
/*	HiliteControl ((ControlHandle) itemHandle, 255 );*/

	GetDItem(prefDialogPtr, kTEXTsuffix, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->suffix);

	GetDItem(prefDialogPtr, kTEXTcreator, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->textcreator);
	SelIText(prefDialogPtr, kTEXTsuffix, 0, 32767);
	
	GetDItem(prefDialogPtr,kTEXTlevel, &itemType, &itemHandle, &itemRect);
	NumToString( (long)thePrefsTypePtr->level, sTemp );
	SetIText(itemHandle,sTemp);

	do {
		GetPort(&savePort);
		SetPort(prefDialogPtr);
		ModalDialog((ModalFilterProcPtr)filterIt, &item);
		switch(item) {
		
			case kSaveButtonItem:
				/* user wants to save the current or new settings */
				
				/* copy the editText string into the new prefs record */
				GetDItem(prefDialogPtr, kTEXTcreator, 
					&itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.textcreator);
				
				GetDItem(prefDialogPtr, kTEXTsuffix, 
					&itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.suffix);
				
				if(newPrefsType.textcreator[0]!=0x04)
				{
					Calert( "TEXT creator not valid" );
				}
				else
				{
						/* replace the old record with the new record */
						*thePrefsTypePtr = newPrefsType;
						doneFlag = true;
				}
				break;
				
			case kCancelButtonItem:
				/* abandon the new preferences structure */
				doneFlag = true;
				break;
				
			case kForce:
			
				newPrefsType.force = !newPrefsType.force;
				GetDItem(prefDialogPtr, kForce, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.force ? 1 : 0));
				break;
				
			case kBeepWD:
			
				newPrefsType.BeepWhenDone = !newPrefsType.BeepWhenDone;
				GetDItem(prefDialogPtr, kBeepWD, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.BeepWhenDone ? 1 : 0));
				break;
				
			case kQuitWD:
			
				newPrefsType.QuitWhenDone = !newPrefsType.QuitWhenDone;
				GetDItem(prefDialogPtr, kQuitWD, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.QuitWhenDone ? 1 : 0));
				break;
				
			case kUpArrow:
			
				if ( newPrefsType.level < 9 )
				{
					newPrefsType.level++;
					GetDItem(prefDialogPtr,kUpArrow, &itemType, &itemHandle, &itemRect);
					InvertRect(&itemRect);
					while (StillDown())
					;
					InvertRect(&itemRect);
					GetDItem(prefDialogPtr,kTEXTlevel, &itemType, &itemHandle, &itemRect);
					NumToString( (long)newPrefsType.level, sTemp );
					SetIText(itemHandle,sTemp);
				}
				break;
				
			case kDownArrow:
			
				if ( newPrefsType.level > 1 )
				{
					newPrefsType.level--;
					GetDItem(prefDialogPtr,kDownArrow, &itemType, &itemHandle, &itemRect);
					InvertRect(&itemRect);
					while (StillDown())
					;
					InvertRect(&itemRect);
					GetDItem(prefDialogPtr,kTEXTlevel, &itemType, &itemHandle, &itemRect);
					NumToString( (long)newPrefsType.level, sTemp );
					SetIText(itemHandle,sTemp);
				}
				break;
				
		}
	} while (!doneFlag);
	
	DisposDialog(prefDialogPtr);
	
	SetPort(savePort);

	/* if the user clicked Save, return true */
	return (item == kSaveButtonItem);
}

pascal Boolean filterIt(DialogPtr inputDialog, EventRecord *myDialogEvent, short *theDialogItem)
{
	ModalFilterProcPtr theModalProc;
	Boolean returnVal = false;
	char theKey;
	
	if (((myDialogEvent->what == keyDown) || (myDialogEvent->what == autoKey)) &&
		(myDialogEvent->modifiers & CmdKeyMask))
	{
        theKey = myDialogEvent->message & charCodeMask;
        switch (theKey)
        {
			case kUpArrowKey:
							*theDialogItem = kUpArrow;
							returnVal = true;
							break;
			case kDownArrowKey:
							*theDialogItem = kDownArrow;
							returnVal = true;
							break;
        }
	}
	
	if (!returnVal)
		if (GetStdFilterProc(&theModalProc) == noErr)
			returnVal=theModalProc(inputDialog, myDialogEvent, theDialogItem);

	return(returnVal);
}