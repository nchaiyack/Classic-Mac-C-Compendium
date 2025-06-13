/*			SpinCursors by
 *			America Online: LISPer
 *		 	Internet: tree@uvm.edu
 */
#ifndef _FOLDERS_
#include <Folders.h>
#endif
#include <Script.h>
#include <Balloons.h>
/* 22may95,ejo: added these */
#include <Resources.h>
#include <SegLoad.h>
#include <TextUtils.h>
#include <Fonts.h>

#include "ICAPI.h"
#include "ICKeys.h"

#include "ThePrefs.h"
#include "MacErrors.h"
#include "PopUp.h"


/************************************************************************************/

#define kPrefsDialogID		130
#define kSaveButtonItem		1
#define kCancelButtonItem	2

#define	kKeepOrig			16
#define kBeepWD				14
#define kQuitWD				5

#define kForce				7
#define	kBHelp				8
#define kHelp				15

#define kCStKeys			26
#define kUStKeys			27
#define	kAltStKeys			30

#define kTEXTsuffix			3
#define kTEXTcreator		4
#define kTEXTbcreator		23
#define kTEXTbtype			28

#define kTEXTlevel			10

#define kUpArrow			11
#define kDownArrow			12

#define	kAsciiGzip			17
#define	kAsciiGunzip		18
#define	kGzip				19


#define	kAsciiGzipMenu		128
#define	kAsciiGunzipMenu	129
#define	kGzipMenu			130

/* prefs file constants */
#define kPrefsNameStrID			128
#define kPrefsCreatorType		'\?\?\?\?'
#define kPrefsFileType			'pref'
#define kPrefsResourceType		'Pref'
#define kPrefsResourceID		128

#define kFetchPrefsNameStrID	129
#define kFetchPrefsResourceType	'SUFX'
#define kFetchPrefsResourceID	1

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


/* End From DialogBits */

/* these constants may be stored in the fields of the prefs struct */

static OSErr GetPrefsName (StringPtr prefName, short StrID);
static ICError ICGetPrefHandle (ICInstance inst, ConstStr255Param key, 
						 ICAttr *attr, Handle *prefh);
						 
PrefsType	currPrefs;
Boolean		prefsChangedFlag;

Handle		FPrefs;
Size		FPrefsSize=0;

ICInstance	inst;
Handle		ICmappings = nil;

/* ejo: added this for PPC */
ModalFilterUPP gModalUPP;

short OpenPrefsResFile(SignedByte prefsPerm, Boolean createFlag, short StrID)
/* open the preferences file with the given permission; if createFlag is set,
   create a preferences file if necessary */
{
	OSErr retCode;
	short prefsVRefNum;
	long prefsDirID;
	Str255 prefsNameStr;
	FSSpec prefsFSSpec;
	short prefsResRefNum;
	
	prefsResRefNum = -1;
	/* get the name of the prefs file */
	retCode = GetPrefsName(prefsNameStr, StrID);
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

static OSErr GetPrefsName (StringPtr prefName, short StrID)
/* get the name of the preferences file from a resource in the application */
{
	StringHandle prefStrHandle;
	OSErr retCode = noErr;
	
	prefStrHandle = GetString(StrID);
	if (prefStrHandle != nil) 
		BlockMove(*prefStrHandle, prefName, (Size) (**prefStrHandle) + 1);
	else retCode = resNotFound;
	
	return retCode;
}

Boolean GetOrGeneratePrefs( PrefsType * thePrefsTypePtr, long wantedVers )
/* fill in the prefs structure from the resource file or from scratch */
{
	short				prefsResRefNum;
	Handle				tempHandle;
	extern	Boolean		gSavedPos;
	extern	Point		gSavedPoint;
	
	/* initialize prefs structure in case we can't get a valid set */
	thePrefsTypePtr->prefsTypeVers =		wantedVers;
	thePrefsTypePtr->compress =				false;
	thePrefsTypePtr->ascii =				false;
	thePrefsTypePtr->force =				false;
	thePrefsTypePtr->level =				6;
	thePrefsTypePtr->suffix[0] =			0x00;
	BlockMove("\pttxt", thePrefsTypePtr->textcreator, 5);
	BlockMove("\p\?\?\?\?", thePrefsTypePtr->bincreator, 5);
	BlockMove("\p\?\?\?\?", thePrefsTypePtr->bintype, 5);
	thePrefsTypePtr->KeepOriginals =		false;
	thePrefsTypePtr->QuitWhenDone =			true;
	thePrefsTypePtr->BeepWhenDone =			false;

	thePrefsTypePtr->AsciiCompress =		OnlyText;
	thePrefsTypePtr->AsciiUnCompress =		UseMenu;
	thePrefsTypePtr->ResourceCompress =		Ask;
	thePrefsTypePtr->WhenCompress =			NonMacGzip;

	thePrefsTypePtr->DecodeHQX =	false;
	thePrefsTypePtr->StKeysComp =	true;
	thePrefsTypePtr->StKeysUnComp =	true;
	thePrefsTypePtr->StKeysAlt = 	true;

	thePrefsTypePtr->SavedPos = false;
	thePrefsTypePtr->SavedPoint.h = 0;
	thePrefsTypePtr->SavedPoint.v = 0;
	
	
	/* open (but don't create) the prefs file */
	prefsResRefNum = OpenPrefsResFile(fsRdPerm, false, kPrefsNameStrID);
	if (prefsResRefNum != -1) {
	
		/* file opened successfully, get the prefs resource */
		tempHandle = Get1Resource(kPrefsResourceType, kPrefsResourceID);
		
		/* 
		 * Assertion: left to right evaluation
		 */
		
		if (	(tempHandle != nil) &&
				(GetHandleSize(tempHandle) == sizeof(PrefsType)) &&
				((*(PrefsType *)*tempHandle).prefsTypeVers == wantedVers)
			)
		{
			/*
			 * copy the prefs struct and close file
			 */
			 
			*thePrefsTypePtr = *(PrefsType *)*tempHandle;
			CloseResFile(prefsResRefNum);
		}
		else
		{
			/* 06jun95,ejo: SPDsoft added this to prompt user for new prefs */
			/* when its clear that the current prefs can no longer be used */
			/* because they are incompatible with current prefs version */
			/* or otherwise malformed. */
			CloseResFile(prefsResRefNum);
			return false;
		}
		
	/*
	 * Get Fetch Prefs
	 */
	 
	if( thePrefsTypePtr->AsciiUnCompress == Fetch )
	{
	if((prefsResRefNum = OpenPrefsResFile(fsRdPerm, false, kFetchPrefsNameStrID))!=-1)
		{
			tempHandle = GetResource('SUFX',1);
			FPrefs = tempHandle;
			HandToHand( &FPrefs );
			FPrefsSize = GetHandleSize(FPrefs);
			ReleaseResource( tempHandle );
			CloseResFile(prefsResRefNum);
		}
		else
		{
			/* Fetch Prefs not found; set to menu */
			Calert("Warning: 'Fetch Prefs' not found. Check your Prefs. folder");
			thePrefsTypePtr->AsciiUnCompress = UseMenu;
		}
	}
	/*
	 * Get Internet Config Prefs
	 */
	else if (
		( thePrefsTypePtr->AsciiUnCompress == UseICinExp ) ||
		( thePrefsTypePtr->AsciiCompress == UseICinComp ) )
	{
		Boolean ICErr = true;
		
		if (!ICStart(&inst, 'ICMa'))
		{												// init the sucker
			if (!ICFindConfigFile(inst, 0, nil))
			{											// locate the config file
				if (!ICBegin(inst, icReadOnlyPerm))
				{										// open up the file for reading
					ICAttr	attr = 0;
					
					ICErr = false;
					
					if (!ICGetPrefHandle(inst, (ConstStr255Param)kICMapping, &attr, &ICmappings))
					{
						HUnlock(ICmappings);
					}
					else
					{
						/* Can't get ICmappings */
						ICmappings = nil;
						Calert("Warning: Can't get Mappings from IC. Check your IC Prefs");
						thePrefsTypePtr->AsciiUnCompress = UseMenu;
					}
					ICEnd(inst);							// all done
				}
				
			}
			ICStop(inst);
		}
		
		
		if(ICErr)
		{
			ICmappings = nil;
			Calert("Warning: IC Prefs not found. Check your Internet Prefs");
			thePrefsTypePtr->AsciiUnCompress = UseMenu;
		}
	}
	
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
	prefsResRefNum = OpenPrefsResFile(fsRdWrPerm, true, kPrefsNameStrID);
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
	Boolean	SaveBalloons;
	/* OSErr retCode; 22may5,ejo: this is not used */

	/* keep the new preference settings only in local variables
	   in case the user cancels */
	   
	PrefsType newPrefsType;

	short		itemType;
	Handle		itemHandle;
	Rect		itemRect;
	Str255		sTemp;
	
	/* Boolean changedFlag; 22may5,ejo: this is not used */
	UserItemUPP itemUPP;	/* 22may95,ejo: added this for PPC */

	/* the old settings are our starting point, so copy them */
	newPrefsType = *thePrefsTypePtr;
	
	/* throw up the dialog */
	prefDialogPtr = GetNewDialog(kPrefsDialogID, nil, (WindowPtr) -1);
	if (prefDialogPtr == nil) ExitToShell();  /* hurtinÕ bad */
	
	SetDialogDefaultItem( prefDialogPtr, kSaveButtonItem);
	SetDialogCancelItem( prefDialogPtr, kCancelButtonItem);

	/* set up the dialog to the initial values */
	SaveBalloons = HMGetBalloons();

	GetDItem(prefDialogPtr, kForce, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.force ? 1 : 0));

	GetDItem(prefDialogPtr,kTEXTlevel, &itemType, &itemHandle, &itemRect);
	NumToString( (long)thePrefsTypePtr->level, sTemp );
	SetIText(itemHandle,sTemp);

	GetDItem(prefDialogPtr, kTEXTsuffix, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->suffix);
	
	GetDItem(prefDialogPtr, kTEXTcreator, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->textcreator);
	
	GetDItem(prefDialogPtr, kTEXTbcreator, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->bincreator);

	GetDItem(prefDialogPtr, kTEXTbtype, &itemType, &itemHandle, &itemRect);
	SetIText(itemHandle,thePrefsTypePtr->bintype);

	SelIText(prefDialogPtr, kTEXTbcreator, 0, 32767);

	GetDItem(prefDialogPtr, kKeepOrig, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.KeepOriginals ? 1 : 0));

	GetDItem(prefDialogPtr, kBeepWD, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.BeepWhenDone ? 1 : 0));

	GetDItem(prefDialogPtr, kQuitWD, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.QuitWhenDone ? 1 : 0));


	GetDItem(prefDialogPtr, kAsciiGzip, &itemType, &itemHandle, &itemRect);
	PopUp[kAsciiGzip-PopUpBaseItem].MenuRect = itemRect;
	PopUp[kAsciiGzip-PopUpBaseItem].MenuID = kAsciiGzipMenu;
	PopUp[kAsciiGzip-PopUpBaseItem].Menu = GetMenu(kAsciiGzipMenu);
	PopUp[kAsciiGzip-PopUpBaseItem].Selection = newPrefsType.AsciiCompress;
	SetItemMark(PopUp[kAsciiGzip-PopUpBaseItem].Menu,PopUp[kAsciiGzip-PopUpBaseItem].Selection,checkMark);

#if 1
	/* 22may95,ejo: fixed for PPC */
	itemUPP = NewUserItemProc (DrawPopUp);
	SetDialogItem (prefDialogPtr, kAsciiGzip, itemType, (Handle) itemUPP, &itemRect);
#else
	SetDItem(prefDialogPtr, kAsciiGzip, itemType, (ProcPtr) &DrawPopUp, &itemRect);

/*	WARNING!
 *	SetDialogItem( prefDialogPtr, kAsciiGzip,
 *				itemType, UserItemUPP((ProcPtr) &DrawPopUp), &itemRect); for PPC ?
 */
#endif

	GetDItem(prefDialogPtr, kAsciiGunzip, &itemType, &itemHandle, &itemRect);
	PopUp[kAsciiGunzip-PopUpBaseItem].MenuRect = itemRect;
	PopUp[kAsciiGunzip-PopUpBaseItem].MenuID = kAsciiGunzipMenu;
	PopUp[kAsciiGunzip-PopUpBaseItem].Menu = GetMenu(kAsciiGunzipMenu);
	PopUp[kAsciiGunzip-PopUpBaseItem].Selection = newPrefsType.AsciiUnCompress;
	SetItemMark(PopUp[kAsciiGunzip-PopUpBaseItem].Menu,PopUp[kAsciiGunzip-PopUpBaseItem].Selection,checkMark);

#if 1
	/* 22may95,ejo: fixed for PPC */
	SetDialogItem (prefDialogPtr, kAsciiGunzip, itemType, (Handle) itemUPP, &itemRect);
#else
	SetDItem(prefDialogPtr, kAsciiGunzip, itemType, (ProcPtr) &DrawPopUp, &itemRect);
#endif
	GetDItem(prefDialogPtr, kGzip, &itemType, &itemHandle, &itemRect);
	PopUp[kGzip-PopUpBaseItem].MenuRect = itemRect;
	PopUp[kGzip-PopUpBaseItem].MenuID = kGzipMenu;
	PopUp[kGzip-PopUpBaseItem].Menu = GetMenu(kGzipMenu);
	PopUp[kGzip-PopUpBaseItem].Selection = newPrefsType.WhenCompress;
	SetItemMark(PopUp[kGzip-PopUpBaseItem].Menu,PopUp[kGzip-PopUpBaseItem].Selection,checkMark);
#if 1
	/* 22may95,ejo: fixed for PPC */
	SetDialogItem (prefDialogPtr, kGzip, itemType, (Handle) itemUPP, &itemRect);
#else
	SetDItem(prefDialogPtr, kGzip, itemType, (ProcPtr) &DrawPopUp, &itemRect);
#endif
	GetDItem(prefDialogPtr, kCStKeys, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysComp ? 1 : 0));
	
	GetDItem(prefDialogPtr, kUStKeys, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysUnComp ? 1 : 0));
	
	GetDItem(prefDialogPtr, kAltStKeys, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysAlt ? 1 : 0));

	GetDItem(prefDialogPtr, kHelp, &itemType, &itemHandle, &itemRect);
	SetCtlValue((ControlHandle) itemHandle, SaveBalloons);

	gModalUPP = NewModalFilterProc (filterIt);	/* 22may95,ejo: added this for PPC */

	do {
		GetPort(&savePort);
		SetPort(prefDialogPtr);

		/* ejo: change for PPC */
		ModalDialog (gModalUPP, &item);
		/*ModalDialog((ModalFilterProcPtr)filterIt, &item);*/

		switch(item) {
		
			
			case kSaveButtonItem:
				/* user wants to save the current or new settings */
				
				/* copy the editText string into the new prefs record */
				GetDItem(prefDialogPtr, kTEXTcreator, &itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.textcreator);
				
				GetDItem(prefDialogPtr, kTEXTbcreator, &itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.bincreator);
				
				GetDItem(prefDialogPtr, kTEXTbtype, &itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.bintype);

				GetDItem(prefDialogPtr, kTEXTsuffix, &itemType, &itemHandle, &itemRect);
				GetIText(itemHandle, newPrefsType.suffix);
				
				if(newPrefsType.textcreator[0]!=0x04)
				{
					Calert( "Text creator not valid (Only 4 chars!)" );
					SelIText(prefDialogPtr, kTEXTcreator, 0, 32767);
				}
				else if (newPrefsType.bincreator[0]!=0x04)
				{
					Calert( "Binary creator not valid (Only 4 chars!)" );
					SelIText(prefDialogPtr, kTEXTbcreator, 0, 32767);
				}
				else if (newPrefsType.bintype[0]!=0x04)
				{
					Calert( "Binary type not valid (Only 4 chars!)" );
					SelIText(prefDialogPtr, kTEXTbtype, 0, 32767);
				}
				else
				{
						/* replace the old record with the new record */
						
						newPrefsType.AsciiCompress = PopUp[kAsciiGzip-PopUpBaseItem].Selection;
						newPrefsType.AsciiUnCompress = PopUp[kAsciiGunzip-PopUpBaseItem].Selection;
						newPrefsType.WhenCompress = PopUp[kGzip-PopUpBaseItem].Selection;
						
						*thePrefsTypePtr = newPrefsType;
						doneFlag = true;
				}
				
				if (
					(( FPrefsSize==0) && (newPrefsType.AsciiUnCompress==Fetch)) ||
					(( ICmappings==nil) && (newPrefsType.AsciiCompress==UseICinComp)) ||
					(( ICmappings==nil) && (newPrefsType.AsciiUnCompress==UseICinExp))
					)
					
				{
					Calert("You must restart MacGzip for your changes to take effect");
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
				
			case kKeepOrig:
			
				newPrefsType.KeepOriginals = !newPrefsType.KeepOriginals;
				GetDItem(prefDialogPtr, kKeepOrig, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.KeepOriginals ? 1 : 0));
				break;

			case kQuitWD:
			
				newPrefsType.QuitWhenDone = !newPrefsType.QuitWhenDone;
				GetDItem(prefDialogPtr, kQuitWD, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.QuitWhenDone ? 1 : 0));
				break;
				
			case kBeepWD:
			
				newPrefsType.BeepWhenDone = !newPrefsType.BeepWhenDone;
				GetDItem(prefDialogPtr, kBeepWD, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.BeepWhenDone ? 1 : 0));
				break;
				
			case kAsciiGzip:
			case kAsciiGunzip:
			case kGzip:
			
				DoPopUp(prefDialogPtr, item);
				break;
			
			case kCStKeys:
			
				newPrefsType.StKeysComp = !newPrefsType.StKeysComp;
				GetDItem(prefDialogPtr, kCStKeys, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysComp ? 1 : 0));
				break;
				
			case kUStKeys:
			
				newPrefsType.StKeysUnComp = !newPrefsType.StKeysUnComp;
				GetDItem(prefDialogPtr, kUStKeys, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysUnComp ? 1 : 0));
				break;

			case kAltStKeys:
			
				newPrefsType.StKeysAlt = !newPrefsType.StKeysAlt;
				GetDItem(prefDialogPtr, kAltStKeys, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, ( newPrefsType.StKeysAlt ? 1 : 0));
				break;

			case kHelp:
			case kBHelp:
			
				HMSetBalloons(!HMGetBalloons());
				GetDItem(prefDialogPtr, kHelp, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, HMGetBalloons());
				break;

				
		}
	} while (!doneFlag);
	
	HMSetBalloons(SaveBalloons);
	SetItemMark(PopUp[kAsciiGzip-PopUpBaseItem].Menu,PopUp[kAsciiGzip-PopUpBaseItem].Selection,noMark);
	SetItemMark(PopUp[kAsciiGunzip-PopUpBaseItem].Menu,PopUp[kAsciiGunzip-PopUpBaseItem].Selection,noMark);
	SetItemMark(PopUp[kGzip-PopUpBaseItem].Menu,PopUp[kGzip-PopUpBaseItem].Selection,noMark);

	DisposDialog(prefDialogPtr);
	
	SetPort(savePort);

	/* 22may95,ejo: added these (PPC) */
	DisposeRoutineDescriptor (itemUPP);
	DisposeRoutineDescriptor (gModalUPP);

	/* if the user clicked Save, return true */
	return (item == kSaveButtonItem);
}

pascal Boolean filterIt(DialogPtr inputDialog, EventRecord *myDialogEvent, short *theDialogItem)
{
#if 1
	/* 22may95,ejo */
	ModalFilterUPP theModalProc;
#else
	ModalFilterProcPtr theModalProc;
#endif
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
#if 1
	/* 22may95,ejo */
			returnVal=CallModalFilterProc(theModalProc, inputDialog, myDialogEvent, theDialogItem);
#else
			returnVal=theModalProc(inputDialog, myDialogEvent, theDialogItem);
#endif
	return(returnVal);
}

static ICError ICGetPrefHandle (ICInstance inst, ConstStr255Param key, 
						 ICAttr *attr, Handle *prefh)
{
	ICError	err = noErr;
	long	prefsize = 0;

	(*prefh) = 0L;
	err = ICGetPref(inst, key, attr, nil, &prefsize);
	if (err != noErr)	prefsize = 0;

	(*prefh) = NewHandle(prefsize);
	err = MemError();
	if (err == noErr) {
		HLock(*prefh);
		err = ICGetPref(inst, key, attr, *(*prefh), &prefsize);
		if (err == icPrefNotFoundErr) {
			attr = 0;
			err = noErr;
		}
		HUnlock(*prefh);
	}
	
	if (err != noErr) {
		if (*prefh) DisposeHandle(*prefh);
		(*prefh) = nil;
	}
	return(err);
}








