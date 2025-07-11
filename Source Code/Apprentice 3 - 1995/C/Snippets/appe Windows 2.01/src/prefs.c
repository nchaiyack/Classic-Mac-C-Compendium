// File "prefs.c" - 

#include <Folders.h>

#include "prefs.h"

// * **************************************************************************** * //
// * **************************************************************************** * //

Handle GetPrefs(long type, short version) {
	short prefsRefNum, saveRefNum;
	Handle rsrc, pref=0;
	
	saveRefNum = CurResFile();
	if ((prefsRefNum = OpenPrefsFile()) == -1) return(0);
	
	if (pref = rsrc = Get1Resource(type, version))
		if (HandToHand(&pref)) pref = 0;
	
	if (rsrc) ReleaseResource(rsrc);
	CloseResFile(prefsRefNum);
	UseResFile(saveRefNum);
	return(pref);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void WritePrefs(Handle pref, long type, short version) {
	short prefsRefNum, saveRefNum;
	Handle rsrc;
	
	if (! pref) return;
	
	saveRefNum = CurResFile();
	if ((prefsRefNum = OpenPrefsFile()) == -1) return;
	
	if (rsrc = Get1Resource(type, version)) RmveResource(rsrc);
	
	rsrc = pref;
	if (! HandToHand(&rsrc)) {
		AddResource(rsrc, type, version, "\p");
		if (ResError()) DisposeHandle(rsrc);
		  else WriteResource(rsrc);
		}
	
	CloseResFile(prefsRefNum);
	UseResFile(saveRefNum);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

short OpenPrefsFile() {
	short err=0, prefsRefNum;
	FSSpec prefsFile;
	
	if (err = FindFolder(kOnSystemDisk, kPreferencesFolderType, -1,
			&prefsFile.vRefNum, &prefsFile.parID)) return(-1);
	BlockMove(kPrefsFileName, prefsFile.name, sizeof(prefsFile.name));
		
	prefsRefNum = FSpOpenResFile(&prefsFile, fsRdWrPerm);
	if (err = ResError()) {
		if ((err != fnfErr) && (err = FSpDelete(&prefsFile))) return(-1);
		if (err = CreatePrefsFile(&prefsFile)) return(-1);
	  	prefsRefNum = FSpOpenResFile(&prefsFile, fsRdWrPerm);
	  	}
	if (ResError() == fnfErr) return(-1);
	UseResFile(prefsRefNum);
	return(prefsRefNum);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

short CreatePrefsFile(FSSpecPtr prefsFile) {
	short err=0;
	
	FSpCreateResFile(prefsFile, kCreatorType, kPrefsFileType, 0);
	return(err = ResError());
	}
