/* Preference Handling  */

#include "Prefs.h"

short	gVersNum;
short	gAppsResNum;
short	gPrefsResNum;
short	gPrefsRefNum;	// prefs file vRefNum

void DoReadPrefs()
{
	short	vRefNum;
	long	dirID;
	Str255	name;
	short	resNum;
	FSSpec	spec;
	OSErr	err;
	Handle	myPrefs;
	short	windTop, windLeft, tWindTop, tWindLeft;
	char	*s = "PPPop Prefs";
	
	gAppsResNum = CurResFile();

	GetIndString(name, rFileNames, sPrefsFile);		// get file name
	if (name[0] == 0) {
		BlockMove(s, name + 1, 11);
		name[0] = 11;
	}
	
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &vRefNum, &dirID);
	if (err != noErr) {
		vRefNum = 0;
		dirID = 0;
	}
	
	gPrefsRefNum = vRefNum;
	
	err = FSMakeFSSpec(vRefNum, dirID, name, &spec);
	resNum = FSpOpenResFile(&spec, fsRdWrPerm);
	
	if (resNum == -1) 
		resNum = DoCreatePrefsFile(&spec);
		
	if (resNum != -1) {
		UseResFile(resNum);
		myPrefs = Get1Resource(kPrefsType, rPrefsID);
		if (myPrefs == nil) {
			DoCopyResource(kPrefsType, rPrefsID, gAppsResNum, resNum);
			myPrefs = Get1Resource(kPrefsType, rPrefsID);
		}
	}
	
	if (myPrefs != nil && GetHandleSize((Handle)myPrefs) == sizeof(PrefsRec)) {
		gVersNum = (**(PrefsHnd)myPrefs).version;
		windTop = (**(PrefsHnd)myPrefs).top;
		windLeft = (**(PrefsHnd)myPrefs).left;
		gWillHardClose = (**(PrefsHnd)myPrefs).willHardClose;
		gSoundOn = (**(PrefsHnd)myPrefs).sound;
		gReturnToFinder = (**(PrefsHnd)myPrefs).finder;
		gTimerOn = (**(PrefsHnd)myPrefs).timer;
		tWindTop = (**(PrefsHnd)myPrefs).ttop;
		tWindLeft = (**(PrefsHnd)myPrefs).tleft;
		gCumulativeTime = (**(PrefsHnd)myPrefs).totalTime;
		gShowCurrent = (**(PrefsHnd)myPrefs).showCurrent;
		gAutoPositionTimer = (**(PrefsHnd)myPrefs).autoPosition;
		gResetTime = (**(PrefsHnd)myPrefs).resetTime;
		gCurrentRadio = (**(PrefsHnd)myPrefs).resetWhen;
		gResetDay = (**(PrefsHnd)myPrefs).resetDay;

		ReleaseResource(myPrefs);
	}
	else {
		gVersNum = 0x0000;
		windTop = defWindTop;
		windLeft = defWindLeft;
		gWillHardClose = TRUE;
		gSoundOn = TRUE;
		gReturnToFinder = FALSE;
		gTimerOn = TRUE;
		tWindTop = defTWindTop;
		tWindLeft = defTWindLeft;
		gCumulativeTime = 0;
		gShowCurrent = TRUE;
		gCurrentRadio = 0;
		gAutoPositionTimer = FALSE;
		gResetTime = 0;
		gResetDay = 1;
	}

	MoveWindow(gAppWindow, windLeft, windTop, FALSE);
	MoveWindow(gTimerWindow, tWindLeft, tWindTop, FALSE);
		
	UseResFile(gAppsResNum);
	gPrefsResNum = resNum;
}


/*******************************************************************************/
void DoSavePrefs()
{
	Handle		myPrefData;
	Handle		rHandle;
	short		rID;
	ResType		rType;
	Str255		rName;
	short		rAttr;
	Point		theTopLeft;
	
	if (gPrefsResNum == -1)
		return;

	myPrefData = NewHandleClear(sizeof(PrefsRec));
	if (myPrefData == nil)
		DeathAlert(errNoMemory);
		
	HLockHi(myPrefData);
		
	(**(PrefsHnd)myPrefData).version = gVersNum;
	theTopLeft = GetWindowPosition(gAppWindow);
	(**(PrefsHnd)myPrefData).top = theTopLeft.v;
	(**(PrefsHnd)myPrefData).left = theTopLeft.h;
	(**(PrefsHnd)myPrefData).willHardClose = gWillHardClose;
	(**(PrefsHnd)myPrefData).sound = gSoundOn;
	(**(PrefsHnd)myPrefData).finder = gReturnToFinder;
	(**(PrefsHnd)myPrefData).timer = gTimerOn;
	(**(PrefsHnd)myPrefData).totalTime = gCumulativeTime;
	theTopLeft = GetWindowPosition(gTimerWindow);
	(**(PrefsHnd)myPrefData).showCurrent = gShowCurrent;
	(**(PrefsHnd)myPrefData).ttop = theTopLeft.v;
	(**(PrefsHnd)myPrefData).tleft = theTopLeft.h;
	(**(PrefsHnd)myPrefData).autoPosition = gAutoPositionTimer;
	(**(PrefsHnd)myPrefData).resetTime = gResetTime;
	(**(PrefsHnd)myPrefData).resetDay = gResetDay;
	(**(PrefsHnd)myPrefData).resetWhen = gCurrentRadio;

	UseResFile(gPrefsResNum);
	
	rHandle = Get1Resource(kPrefsType, rPrefsID);		// Delete current PRFN resource
	if (rHandle != nil) {
		GetResInfo(rHandle, &rID, &rType, rName);
		rAttr = GetResAttrs(rHandle);
		RmveResource(rHandle);
		DisposHandle(rHandle);
	}
	
	if (ResError() == noErr) {
		AddResource(myPrefData, kPrefsType, rPrefsID, rName);
		if (ResError() == noErr)
			SetResAttrs(myPrefData, rAttr);
		else {
			HUnlock(myPrefData);
			DisposeHandle(myPrefData);
		}
		if (ResError() == noErr)
			ChangedResource(myPrefData);
		if (ResError() == noErr)
			WriteResource(myPrefData);
		if (ResError() == noErr)
			UpdateResFile(gPrefsResNum);			// Write it out NOW
		if (ResError() == noErr)
			FlushVol(nil, gPrefsRefNum);
	}
	
	if (myPrefData != nil) {
		HUnlock(myPrefData);
		ReleaseResource(myPrefData);
	}
	UseResFile(gAppsResNum);
}

/*******************************************************************************/
OSErr DoCopyResource(ResType rType, short rID, short source, short dest)
{
	Handle	myHandle;	// handle to resource copy
	Str255	myName;		// name or resource to copy
	short	myAttr;		// resource attributes
	ResType	*myType;	// ignored
	short	*myID;		// ignored
	short	myCurrent;	// Res file on entry
	OSErr	err;
	
	myCurrent = CurResFile();
	UseResFile(source);
	
	myHandle = Get1Resource(rType, rID);
	
	if (myHandle != nil) {
		GetResInfo(myHandle, myID, myType, myName);
		myAttr = GetResAttrs(myHandle);
		DetachResource(myHandle);
		UseResFile(dest);
		if (ResError() == noErr)
			AddResource(myHandle, rType, rID, myName);
		if (ResError() == noErr)
			SetResAttrs(myHandle, myAttr);
		if (ResError() == noErr)
			ChangedResource(myHandle);
		if (ResError() == noErr)
			WriteResource(myHandle);
	}
	err = ResError();
	ReleaseResource(myHandle);		// does not respond if something wrong with handle
	UseResFile(myCurrent);
	return err;
}

/*******************************************************************************/
short DoCreatePrefsFile (FSSpec *spec)
{
	short 	resNum = -1;
	OSErr	err;
	short	rID;
	Handle	rHandle;
	ResType	rType;
	
	ResetCumulativeTime();		// Set last reset time to first use of program.
	
	FSpCreateResFile(spec, kCreator, kPrefsFile, 0);
	if (ResError() == noErr) {
		resNum = FSpOpenResFile(spec, fsRdWrPerm);
		if (resNum != -1) {
			err = DoCopyResource(kPrefsType, rPrefsID, gAppsResNum, resNum);
			if (err != noErr) {
				CloseResFile(resNum);
				err = FSpDelete(spec);
				resNum = -1;
			}
		}
	}
	return resNum;
}
	