/*
	Preferences.c
	
	A resource-based preferences class — automatically handles all resource manipulations.
	Very few applications should need to subclass this class.

	Copyright © 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.
		
	This class was created as part of the Dragonsmith drag-and-drop application development kit,
	but may be used as is by any application that uses THINK C's OOP extensions — although you
	will need to add more #include's to get it to compile correctly (or you can use the same precompiled
	headers that Dragonsmith does)
	
	See the file "Preferences class notes" for a brief description of this class
	
*/

#include	"Preferences.h"

Preferences::Preferences (void)
{
	appResFork = kInvalidRefNum;
	fileCreator = '????';							// These two will be set
	fileType = '????';							//	in the Init method
	prefsRoster = NULL;
	numPrefs = 0;
	file.name[0] = 0;
	fileResFork = kInvalidRefNum;
	fileExists = FALSE;
	canSaveFile = FALSE;
	usingAppResources = TRUE;					// This is the safer default
}

Boolean Preferences::Init (short appRF, OSType creator, OSType type)
{
	// Initialize preferences using the resources in the application's file.  This method's caller should treat a return
	//	value of FALSE as a fatal error.  The only thing that might cause this is if the application's file is messed up
	//	(i.e., it's lacking some vital resources or the 'PrRo' resource has been corrupted)
	
	fileCreator = creator;
	fileType = type;
	appResFork = appRF;						// We can use the application's resources if we have to
	if (appResFork == kInvalidRefNum)
		return FALSE;
	prefsRoster = InitPrefsRoster (appResFork);
	if (prefsRoster != NULL && VerifyResources (prefsRoster)) {
		// Now initialize instance variables for the newly opened application file and return successfully
		numPrefs = (*prefsRoster)->numRsrcs;
		fileExists = FALSE;
		usingAppResources = TRUE;
		canSaveFile = FALSE;
		return TRUE;
	} else
		return FALSE;
}

Preferences::~Preferences (void)
{
	// Requires:	fileExists, fileResFork, canSaveFile
	// Changes:	<not applicable>

	CloseFile ();		// This will automatically save any changes if appropriate
}

Boolean Preferences::UseFile (FSSpec *fss)
{
	// Switch to another preferences file (if fss != NULL)
	
	// Requires:	prefsRoster, numPrefs, fileExists, fileResFork, canSaveFile
	// Changes:	<if it succeeds, all but appResFork, fileCreator, and fileType — otherwise, none>

	char				perm = fsRdWrPerm;
	short			refNum = kInvalidRefNum;
	PrefsRosterList	**roster;
	OSErr			err;
	
	if (fss == NULL)
		return FALSE;
	
	// Open the file with either fsRdWrPerm or fsRdPerm — on return, perm will contain the permission that was granted
	err = FSpOpenResFork (fss, &refNum, &perm);
	if (err == noErr && refNum != kInvalidRefNum) {
		roster = InitPrefsRoster (refNum);
		if (roster == NULL && perm == fsRdWrPerm && err == noErr) {
		
			// If we couldn't initialize the preferences roster from the file we want to use, then we copy everything from the
			//	current preference file's resource fork to the newly opened file (so we have to have write access!)
			err = CopyPrefs (refNum);			// Ensure that any prefs resources not already in the file will be copied
											//	from the current prefs file (or the application)
			roster = InitPrefsRoster (refNum);		// Try to initialize the preferences roster for the new file
		}
		if (roster != NULL && err == noErr && VerifyResources (roster)) {
		
			// Save and close the file we were using (CloseFile does both, plus it disposes of the old prefs resources roster)
			CloseFile ();
			
			// Now initialize instance variables for the newly opened file and return successfully
			prefsRoster = roster;
			numPrefs = (*prefsRoster)->numRsrcs;
			file = *fss;
			fileResFork = refNum;
			fileExists = TRUE;
			usingAppResources = FALSE;
			canSaveFile = (perm == fsRdWrPerm);
			return TRUE;
		} else if (refNum != appResFork)
			CloseResFile (refNum);
	}
	return FALSE;
}

Boolean Preferences::VerifyResources (PrefsRosterList **roster)
{
	return TRUE;

	// Override this method to provide verification of the specified roster — your method shouldn't rely on any of this class's
	//	instance variables being valid, nor should it change any of them (though the same does not necessarily apply to
	//	the instance variables which your subclass adds — that's up to you)
	
	// This method is called by Init (to test the resources in the application file) and UseFile (when we're trying to switch to
	//	another file).   At the time this method is called, roster is a valid prefs resources roster containing handles to all the
	//	preferences resources that could be found.  Also, (*roster)->refNum is a valid refNum to the file in which the resources
	//	reside (we may or may not have write access to this file)
	
	// If there are any preference resources that absolutely MUST exist, then you can either try to add them by hand
	//	(which will only work if you have write access to the file), or you can just do like this —
	
	/*
		return (	(*roster)->rsrc[prefVital1].resHndl != NULL
			&&	(*roster)->rsrc[prefVital2].resHndl != NULL
			&&	(*roster)->rsrc[prefVital3].resHndl != NULL
			&&	(*roster)->rsrc[prefVital4].resHndl != NULL);
	*/
	
	// Of course, you might also need to check for empty handles…
	
	// If VerifyResources returns FALSE, then the file being verified (i.e., (*roster)->refNum) will not be used.  If it's the app file,
	//	then this is a fatal error and the app should quit.  Otherwise, remember — THEY'RE JUST PREFERENCES!!  Your app
	//	can still function without them
}

OSErr Preferences::CopyPrefs (short destRF)
{
	// Requires:	prefsRoster, numPrefs
	// Changes:	<none>

	short			saveRF, i;
	Handle			h;
	OSErr			err = noErr;
	PrefsRosterList	**tempRoster = NULL;
	
	if (destRF == kInvalidRefNum)
		return rfNumErr;
	
	saveRF = CurResFile ();
	if (saveRF != destRF)
		UseResFile (destRF);
	
	h = Get1Resource (rPrefsRosterType, rPrefsRosterID);	// Look to see if this file already has a 'PrRo' resource
	if (h != NULL) {										//	— if it does, get rid of it
		RmveResource (h);
		DisposHandle (h);
	}

	// Make a copy of the current prefsRoster — it would turn into a resource handle if we AddResource'd it (ugh!)
	tempRoster = prefsRoster;
	err = HandToHand ((Handle *) &tempRoster);
	if (err == noErr) {
		(*tempRoster)->refNum = kInvalidRefNum;		// Wipe out any run-time specific
		for (i = 0; i < numPrefs; i++)					//	values in the refNum field
			(*tempRoster)->rsrc[i].resHndl = NULL;		//	and resHndl fields of the copy
		AddResource ((Handle) tempRoster, rPrefsRosterType, rPrefsRosterID, NULL);
		err = ResError ();
	}
	
	// This loop won't be executed if AddResource failed		
	for (i = 0; i < numPrefs && err == noErr; i++)
		err = CopyResHandle (GetPrefResource (i), destRF, FALSE);
	
	if (err == noErr)
		UpdateResFile (destRF);

	if (saveRF != destRF)
		UseResFile (saveRF);
	
	return err;
}

Handle Preferences::GetPrefResource (short index)
{
	// Requires:	prefsRoster, numPrefs
	// Changes:	<none>
	
	Handle	h;
	
	if (index < 0 || index >= numPrefs)
		return NULL;
	
	h = (*prefsRoster)->rsrc[index].resHndl;
	if (h == NULL)
		return ReadPrefResource (index);
	else if (*h == NULL)
		LoadResource (h);
	
	return h;
}

Handle Preferences::ReadPrefResource (short index)
{
	// Requires:	prefsRoster
	// Changes:	<none>

	short	useRefNum, saveRefNum;
	Handle	h;
	
	useRefNum = (*prefsRoster)->refNum;		// Use the file in which the resources in the current prefsRoster reside
	if (useRefNum != kInvalidRefNum) {
		saveRefNum = CurResFile ();
		if (saveRefNum != useRefNum)
			UseResFile (useRefNum);
		h = (*prefsRoster)->rsrc[index].resHndl = Get1Resource ((*prefsRoster)->rsrc[index].resType, (*prefsRoster)->rsrc[index].resID);
		if (saveRefNum != useRefNum)
			UseResFile (saveRefNum);
		return h;
	} else
		return NULL;
}

OSErr Preferences::DetachPrefResource (short index)
{
	OSErr	err = resNotFound;
	Handle	h;
	
	if (index >= 0 && index <= numPrefs) {
		h = (*prefsRoster)->rsrc[index].resHndl;
		if (h != NULL) {
			DetachResource (h);
			if ((err = ResError ()) == noErr)
				(*prefsRoster)->rsrc[index].resHndl = NULL;
		}
	}
	return err;
}

OSErr Preferences::ReleasePrefResource (short index)
{
	OSErr	err = resNotFound;
	Handle	h;
	
	if (index >= 0 && index <= numPrefs) {
		h = (*prefsRoster)->rsrc[index].resHndl;
		if (h != NULL) {
			ReleaseResource (h);
			if ((err = ResError ()) == noErr)
				(*prefsRoster)->rsrc[index].resHndl = NULL;
		}
	}
	return err;
}

void Preferences::CloseFile (void)
{
	// Close the current preferences file (this will save it if possible) and dispose of the preferences resources roster
	
	// Requires:	fileResFork
	// Changes:	fileResFork, fileExists, canSaveFile, usingAppResources
	// Disposes:	prefsRoster

	Handle	h;
	short	i;
	
	// If we're using a preferences file, close it (saving any resources which have been ChangedResource'd) — this
	//	automatically releases all the resources referred to in prefsRoster
	if (fileResFork != kInvalidRefNum) {
		CloseResFile (fileResFork);
		fileResFork = kInvalidRefNum;
 		if (fileExists)
			FlushVol (NULL, file.vRefNum);
		fileExists = FALSE;
		canSaveFile = FALSE;
		usingAppResources = TRUE;
	}
	
	// Now release the preferences resources roster itself
	if (prefsRoster != NULL) {
	
		// If the resources came from the application file, special care must be taken to release everything by hand,
		//	since the code above wasn't executed, so the application file wasn't closed (which it shouldn't be, of course)
		if ((*prefsRoster)->refNum == appResFork) {		// Could also say "if (usingAppResources)", but I prefer this
			for (i = 0; i < numPrefs; i++) {
				h = (*prefsRoster)->rsrc[i].resHndl;
				if (h != NULL)
					ReleaseResource (h);
			}
		}
		DisposHandle ((Handle) prefsRoster);
	}
}

OSErr Preferences::SavePrefResource (short index)
{
	// Mark the designated resource as changed, and (if possible) 
	// Requires:	prefsRoster, numPrefs
	// Changes:	<none>

	Handle	h;
	char		hState;
	OSErr	err;
	
	if (canSaveFile && index >= 0 && index < numPrefs) {
		h = (*prefsRoster)->rsrc[index].resHndl;
		if (h == NULL)
			return resNotFound;	// The resource doesn't exist
		if (*h == NULL)
			return noErr;			// It's already saved (and it's purged)
		hState = HGetState (h);
		HNoPurge (h);
		ChangedResource (h);
		err = ResError ();			// Inside Macintosh, Volume I (page 125) warns about not checking ResError here
		if (err == noErr)
			WriteResource (h);
		HSetState (h, hState);
		return ResError ();
	} else
		return fLckdErr;			// Return an error as if the file were locked (which in fact it may be…)
}

PrefsRosterList **Preferences::InitPrefsRoster (short refNum)
{
	// Requires:	<none>
	// Changes:	<none>

	short			saveRF, n;
	register short		i;
	PrefsRosterList	**roster, **prefsRosterWas;
	
	saveRF = CurResFile ();
	if (saveRF != refNum)
		UseResFile (refNum);
		
	roster = (PrefsRosterList **) Get1Resource (rPrefsRosterType, rPrefsRosterID);
	if (roster != NULL) {
		DetachResource ((Handle) roster);		// The prefs roster should never be changed except in memory
		HNoPurge ((Handle) roster);			// Just in case the 'PrRo' resource's attributes weren't set correctly
		
		(*roster)->refNum = refNum;				// Store a copy of the file's refNum for safekeeping
		n = (*roster)->numRsrcs;
		if (n < 1) {
			DisposHandle ((Handle) roster);
			roster = NULL;
		} else {
			prefsRosterWas = prefsRoster;		// Save the original value of prefsRoster
			prefsRoster = roster;
			for (i = 0; i < n; i++)
				(void) ReadPrefResource (i);		// Ignore any errors
			prefsRoster = prefsRosterWas;		// Restore prefsRoster
		}
	}
	
	if (saveRF != refNum)
		UseResFile (saveRF);
	
	return roster;
}

