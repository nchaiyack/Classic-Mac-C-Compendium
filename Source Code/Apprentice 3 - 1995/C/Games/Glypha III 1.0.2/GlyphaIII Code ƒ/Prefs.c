
//============================================================================
//----------------------------------------------------------------------------
//									Prefs.c
//----------------------------------------------------------------------------
//============================================================================

// This is a slick little file that I re-use and re-use.  I wrote it to�
// seemlessly handle System 6 or System 7 with but a single call.  You need�
// to define your own "prefs" struct, but these routines will read and write�
// it to the System folder.

#include "Externs.h"
#include <Folders.h>							// Needed for creating a folder.
#include <GestaltEqu.h>							// Needed for the Gestalt() call.
#include <Script.h>								// I can't remember why I needed this.


#define	kPrefCreatorType	'zade'				// Change this to reflect your apps creator.
#define	kPrefFileType		'zadP'				// Change this to reflect your prefs type.
#define	kPrefFileName		"\pGlypha Prefs"	// Change this to reflect the name for your prefs.
#define	kDefaultPrefFName	"\pPreferences"		// Name of prefs folder (System 6 only).
#define kPrefsStringsID		160					// For easy localization.
#define	kPrefsFNameIndex	1					// This one works with the previous constant.


Boolean CanUseFindFolder (void);
Boolean GetPrefsFPath (long *, short *);
Boolean CreatePrefsFolder (short *);
Boolean GetPrefsFPath6 (short *);
Boolean WritePrefs (long *, short *, prefsInfo *);
Boolean WritePrefs6 (short *, prefsInfo *);
OSErr ReadPrefs (long *, short *, prefsInfo *);
OSErr ReadPrefs6 (short *, prefsInfo *);
Boolean DeletePrefs (long *, short *);
Boolean DeletePrefs6 (short *);


//==============================================================  Functions
//--------------------------------------------------------------  CanUseFindFolder

// Returns TRUE if we can use the FindFolder() call (a System 7 nicety).

Boolean CanUseFindFolder (void)
{
	OSErr		theErr;
	long		theFeature;
	
	if (!DoWeHaveGestalt())		// Darn, have to check for Gestalt() first.
		return(FALSE);			// If no Gestalt(), probably don't have FindFolder().
	
	theErr = Gestalt(gestaltFindFolderAttr, &theFeature);
	if (theErr != noErr)		// Use selector for FindFolder() attribute.
		return(FALSE);
								// Now do a bit test specifically for FindFolder().
	if (!BitTst(&theFeature, 31 - gestaltFindFolderPresent))
		return(FALSE);
	else
		return(TRUE);
}

//--------------------------------------------------------------  GetPrefsFPath

// This function gets the file path to the Preferences folder (for System 7).
// It is called only if we can use FindFolder() (see previous function).

Boolean GetPrefsFPath (long *prefDirID, short *systemVolRef)
{
	OSErr		theErr;
									// Here's the wiley FindFolder() call.
	theErr = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
		systemVolRef, prefDirID);	// It returns to us the directory and volume ref.�
	if (theErr != noErr)			// Assuming it worked at all!
		return(FALSE);
	
	return(TRUE);
}

//--------------------------------------------------------------  CreatePrefsFolder

// This function won't be necessary for System 7, for System 6 though, it creates�
// a folder ("Preferences") in the System folder and returns whether or not it worked.

Boolean CreatePrefsFolder (short *systemVolRef)
{
	HFileParam	fileParamBlock;
	Str255		folderName;
	OSErr		theErr;
										// Here's our localization.  Rather than�
										// hard-code the name "Preferences" in the code�
										// we pull up the text from a string resource.
	GetIndString(folderName, kPrefsStringsID, kPrefsFNameIndex);
										// Set up a file parameter block.
	fileParamBlock.ioVRefNum = *systemVolRef;
	fileParamBlock.ioDirID = 0;
	fileParamBlock.ioNamePtr = folderName;
	fileParamBlock.ioCompletion = 0L;
										// And create a directory (folder).
	theErr = PBDirCreate((HParmBlkPtr)&fileParamBlock, FALSE);
	if (theErr != noErr)				// See that it worked.
	{
		RedAlert("\pPrefs Creation Error");
		return(FALSE);
	}
	return(TRUE);
}

//--------------------------------------------------------------  GetPrefsFPath6

// If ever there was a case to drop support for System 6 (and require System 7),�
// this is it.  Look at how insidious handling System 6 files can be.  The following�
// function is the "System 6 pedigree" of the above GetPrefsFPath() function.  Note�
// that the GetPrefsFPath() function was ONE CALL!  TWO LINES OF CODE!  The below�
// function is like a page or so.  Anyway, this function is called if Glypha is�
// running under System 6 and essentially returns a volume reference pointing to�
// the preferences folder.

Boolean GetPrefsFPath6 (short *systemVolRef)
{
	Str255		folderName, whoCares;
	SysEnvRec	thisWorld;
	CInfoPBRec	catalogInfoPB;
	DirInfo		*directoryInfo = (DirInfo *) &catalogInfoPB;
	HFileInfo	*fileInfo = (HFileInfo *) &catalogInfoPB;
	WDPBRec		workingDirPB;
	long		prefDirID;
	OSErr		theErr;
												// Yokelization.
	GetIndString(folderName, kPrefsStringsID, kPrefsFNameIndex);
												// SysEnvirons() for System folder volRef.
	theErr = SysEnvirons(2, &thisWorld);
	if (theErr != noErr)
		return(FALSE);
												// Okay, here's the volume reference.
	*systemVolRef = thisWorld.sysVRefNum;
	fileInfo->ioVRefNum = *systemVolRef;		// Set up another parameter block.
	fileInfo->ioDirID  = 0;						// Ignored.
	fileInfo->ioFDirIndex = 0;					// Irrelevant.
	fileInfo->ioNamePtr = folderName;			// Directory we're looking for.
	fileInfo->ioCompletion = 0L;
	theErr = PBGetCatInfo(&catalogInfoPB, FALSE);
	if (theErr != noErr)						// Did we fail to find Prefs folder?
	{
		if (theErr != fnfErr)					// If it WASN'T a file not found error�
		{										// then something more sinister is afoot.
			RedAlert("\pPrefs Filepath Error");
		}										// Otherwise, need to create prefs folder.
		if (!CreatePrefsFolder(systemVolRef))
			return(FALSE);
												// Again - can we find the prefs folder?
		directoryInfo->ioVRefNum = *systemVolRef;
		directoryInfo->ioFDirIndex = 0;
		directoryInfo->ioNamePtr = folderName;
		theErr = PBGetCatInfo(&catalogInfoPB, FALSE);
		if (theErr != noErr)
		{
			RedAlert("\pPrefs GetCatInfo() Error");
			return(FALSE);
		}
	}
	prefDirID = directoryInfo->ioDrDirID;		// Alright, the dir. ID for prefs folder.
	
	workingDirPB.ioNamePtr = whoCares;			// Now convert working dir. into a "real"�
	workingDirPB.ioVRefNum = *systemVolRef;		// dir. ID so we can get volume number.
	workingDirPB.ioWDIndex = 0;
	workingDirPB.ioWDProcID = 0;
	workingDirPB.ioWDVRefNum = 0;
	workingDirPB.ioCompletion = 0L;
	theErr = PBGetWDInfo(&workingDirPB, FALSE);
	if (theErr != noErr)
	{
		RedAlert("\pPrefs PBGetWDInfo() Error");
	}
												// The volume where directory is located.
	*systemVolRef = workingDirPB.ioWDVRefNum;
	
	workingDirPB.ioNamePtr = whoCares;
	workingDirPB.ioWDDirID = prefDirID;			// Okay, finally, with a directory ID, �
	workingDirPB.ioVRefNum = *systemVolRef;		// and a "hard" volume number�
	workingDirPB.ioWDProcID = 0;				// �
	workingDirPB.ioCompletion = 0L;				// �
	theErr = PBOpenWD(&workingDirPB, FALSE);	// we can create a working directory�
	if (theErr != noErr)						// control block with which to access�
	{											// files in the prefs folder.
		RedAlert("\pPrefs PBOpenWD() Error");
	}
	
	*systemVolRef = workingDirPB.ioVRefNum;
	
	return(TRUE);
}

//--------------------------------------------------------------  WritePrefs

// This is the System 7 version that handles all the above functions when you�
// want to write out the preferences file.  It is called by SavePrefs() below�
// if we're running under System 7.  It creates an FSSpec record to hold�
// information about where the preferences file is located, creates Glypha's�
// preferences if they are not found, opens the prefences file, writes out�
// the preferences, and the closes the prefs.  Bam, bam, bam.

Boolean WritePrefs (long *prefDirID, short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	Str255		fileName = kPrefFileName;
									// Create FSSpec record from volume ref and dir ID.
	theErr = FSMakeFSSpec(*systemVolRef, *prefDirID, fileName, &theSpecs);
	if (theErr != noErr)			// See if it failed.
	{								// An fnfErr means file not found error (no prefs).
		if (theErr != fnfErr)		// If that weren't the problem, we're cooked.
			RedAlert("\pPrefs FSMakeFSSpec() Error");
									// If it was an fnfErr, create the prefs.
		theErr = FSpCreate(&theSpecs, kPrefCreatorType, kPrefFileType, smSystemScript);
		if (theErr != noErr)		// If we fail to create the prefs, bail.
			RedAlert("\pPrefs FSpCreate() Error");
	}								// Okay, we either found or made a pref file, open it.
	theErr = FSpOpenDF(&theSpecs, fsRdWrPerm, &fileRefNum);
	if (theErr != noErr)			// As per usual, if we fail, bail.
		RedAlert("\pPrefs FSpOpenDF() Error");
	
	byteCount = sizeof(*thePrefs);	// Get number of bytes to write (your prefs struct).
									// And, write out the preferences.
	theErr = FSWrite(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)			// Say no more.
		RedAlert("\pPrefs FSWrite() Error");
	
	theErr = FSClose(fileRefNum);	// Close the prefs file.
	if (theErr != noErr)			// Tic, tic.
		RedAlert("\pPrefs FSClose() Error");
	
	return(TRUE);
}

//--------------------------------------------------------------  WritePrefs6

// This is the System 6 equivalent of the above function.  It handles prefs�
// opening, writing and closing for System 6.

Boolean WritePrefs6 (short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	Str255		fileName = kPrefFileName;
									// Attempt to open prefs file.
	theErr = FSOpen(fileName, *systemVolRef, &fileRefNum);
	if (theErr != noErr)			// If it failed, maybe the prefs don't exist.
	{								// An fnfErr means file not found.
		if (theErr != fnfErr)		// See if in fact that WASN'T the reason.
			RedAlert("\pPrefs FSOpen() Error");
									// If fnfErr WAS the problem, create the prefs.
		theErr = Create(fileName, *systemVolRef, kPrefCreatorType, kPrefFileType);
		if (theErr != noErr)
			RedAlert("\pPrefs Create() Error");
									// Open the prefs file.
		theErr = FSOpen(fileName, *systemVolRef, &fileRefNum);
		if (theErr != noErr)
			RedAlert("\pPrefs FSOpen() Error");
	}
	
	byteCount = sizeof(*thePrefs);	// Get number of bytes to write out.
									// Write the prefs out.
	theErr = FSWrite(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)
		RedAlert("\pPrefs FSWrite() Error");
									// And close the prefs file.
	theErr = FSClose(fileRefNum);
	if (theErr != noErr)
		RedAlert("\pPrefs FSClose() Error");
	
	return(TRUE);
}

//--------------------------------------------------------------  SavePrefs

// This is the single function called externally to save the preferences.
// You pass it a pointer to your preferences struct and a version number.
// One of the fields in your preferences struct should be a version number�
// (short prefVersion).  This function determines if we're on System 6 or 7�
// and then calls the appropriate routines.  It returns TRUE if all went well�
// or FALSE if any step failed.

Boolean SavePrefs (prefsInfo *thePrefs, short versionNow)
{
	long		prefDirID;
	short		systemVolRef;
	Boolean		canUseFSSpecs;
	
	thePrefs->prefVersion = versionNow;			// Set prefVersion to versionNow.
	
	canUseFSSpecs = CanUseFindFolder();			// See if we can use FindFolder().
	if (canUseFSSpecs)							// If so (System 7) take this route.
	{											// Get a path to Preferences folder.
		if (!GetPrefsFPath(&prefDirID, &systemVolRef))
			return(FALSE);
	}
	else										// Here's the System 6 version.
	{
		if (!GetPrefsFPath6(&systemVolRef))
			return(FALSE);
	}
	
	if (canUseFSSpecs)							// Write out the preferences.
	{
		if (!WritePrefs(&prefDirID, &systemVolRef, thePrefs))
			return(FALSE);
	}
	else
	{
		if (!WritePrefs6(&systemVolRef, thePrefs))
			return(FALSE);
	}
	
	return(TRUE);
}

//--------------------------------------------------------------  ReadPrefs

// This is the System 7 version for reading in the preferences.  It handles�
// opening the prefs, reading in the data to your prefs struct and closing�
// the file.

OSErr ReadPrefs (long *prefDirID, short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	Str255		fileName = kPrefFileName;
									// Get an FSSpec record to the prefs file.
	theErr = FSMakeFSSpec(*systemVolRef, *prefDirID, fileName, &theSpecs);
	if (theErr != noErr)
	{
		if (theErr == fnfErr)		// If it doesn't exist, return - we'll use defaults.
			return(theErr);
		else						// If some other file error occured, bail.
			RedAlert("\pPrefs FSMakeFSSpec() Error");
	}
									// Open the prefs file.
	theErr = FSpOpenDF(&theSpecs, fsRdWrPerm, &fileRefNum);
	if (theErr != noErr)
		RedAlert("\pPrefs FSpOpenDF() Error");
	
	byteCount = sizeof(*thePrefs);	// Determine the number of bytes to read in.
									// Read 'em into your prefs struct.
	theErr = FSRead(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)			// If there was an error reading the file�
	{								// close the file and we'll revert to defaults.
		if (theErr == eofErr)
			theErr = FSClose(fileRefNum);
		else						// If closing failed, bail.
			RedAlert("\pPrefs FSRead() Error");
		return(theErr);
	}
	
	theErr = FSClose(fileRefNum);	// Close the prefs file.
	if (theErr != noErr)
		RedAlert("\pPrefs FSClose() Error");
	
	return(theErr);
}

//--------------------------------------------------------------  ReadPrefs6

// This is the System 6 version of the above function.  It's basically the same,�
// but doesn't have the luxury of using FSSpec records.

OSErr ReadPrefs6 (short *systemVolRef, prefsInfo *thePrefs)
{
	OSErr		theErr;
	short		fileRefNum;
	long		byteCount;
	Str255		fileName = kPrefFileName;
								// Attempt to open the prefs file.
	theErr = FSOpen(fileName, *systemVolRef, &fileRefNum);
	if (theErr != noErr)		// Did opening the file fail?
	{
		if (theErr == fnfErr)	// It did - did it fail because it doesn't exist?
			return(theErr);		// Okay, then we'll revert to default settings.
		else					// Otherwise, we have a more serious problem.
			RedAlert("\pPrefs FSOpen() Error");
	}
								// Get number of bytes to read in.
	byteCount = sizeof(*thePrefs);
								// Read in the stream of data into prefs struct.
	theErr = FSRead(fileRefNum, &byteCount, thePrefs);
	if (theErr != noErr)		// Did the read fail?
	{							// Maybe we're reading too much data (new prefs vers).
		if (theErr == eofErr)	// That's okay, we'll use defaults for now.
			theErr = FSClose(fileRefNum);
		else
			RedAlert("\pPrefs FSRead() Error");
		return(theErr);
	}
								// Close the prefs file.
	theErr = FSClose(fileRefNum);
	if (theErr != noErr)
		RedAlert("\pPrefs FSClose() Error");
	
	return(theErr);
}

//--------------------------------------------------------------  DeletePrefs

// It can happen that you introduce a game with only a few preference settings�
// but then later update your game and end up having to add additional settings�
// to be stored in your games preferences.  In this case, the size of the old�
// prefs won't match the size of the new.  Or even if the size is the same, you�
// may have re-ordered the prefs and attempting to load the old prefs will result�
// in garbage.  It is for this reason that I use the "versionNeed" variable and�
// the "prefVersion" field in the prefs struct.  In such a case, the below function�
// will be called to delte the old prefs.  When the prefs are then written out, a�
// new pref file will be created.  This particular function is the System 7 version�
// for deleting the old preferences.

Boolean DeletePrefs (long *dirID, short *volRef)
{
	FSSpec		theSpecs;
	Str255		fileName = kPrefFileName;
	OSErr		theErr;
											// Create an FSSec record.
	theErr = FSMakeFSSpec(*volRef, *dirID, fileName, &theSpecs);
	if (theErr != noErr)					// Test to see if it worked.
		return(FALSE);
	else									// If it worked�
		theErr = FSpDelete(&theSpecs);		// delete the file.
	
	if (theErr != noErr)
		return(FALSE);
	
	return(TRUE);
}

//--------------------------------------------------------------  DeletePrefs6

// This is the System 6 version for deleting a preferences file (see above function).

Boolean DeletePrefs6 (short *volRef)
{
	Str255		fileName = kPrefFileName;
	OSErr		theErr;
	
	theErr = FSDelete(fileName, *volRef);	// Delete the prefs file.
	
	if (theErr != noErr)
		return(FALSE);
	
	return(TRUE);
}

//--------------------------------------------------------------  LoadPrefs

// Here is the single call for loading in preferences.  It handles all the�
// above function onvolved with opening and reading in preferences.  It�
// determines whether we are on System 6 or 7 (FSSpecs) and makes the right�
// calls.

Boolean LoadPrefs (prefsInfo *thePrefs, short versionNeed)
{
	long		prefDirID;
	OSErr		theErr;
	short		systemVolRef;
	Boolean		canUseFSSpecs, noProblems;
	
	canUseFSSpecs = CanUseFindFolder();	// See if we can use FSSpecs (System 7).
	if (canUseFSSpecs)
	{									// Get a path to the prefs file.
		noProblems = GetPrefsFPath(&prefDirID, &systemVolRef);
		if (!noProblems)
			return(FALSE);
	}
	else
	{									// Gets path to prefs file (System 6).
		noProblems = GetPrefsFPath6(&systemVolRef);
		if (!noProblems)
			return(FALSE);
	}
	
	if (canUseFSSpecs)
	{									// Attempt to read prefs.
		theErr = ReadPrefs(&prefDirID, &systemVolRef, thePrefs);
		if (theErr == eofErr)			// Fail the read?  Maybe an old prefs version.
		{								// Delete it - we'll create a new one later.
			noProblems = DeletePrefs(&prefDirID, &systemVolRef);
			return(FALSE);				// Meanwhile, we'll use defaults.
		}
		else if (theErr != noErr)
			return(FALSE);
	}
	else
	{									// Attempt to read prefs (System 6).
		theErr = ReadPrefs6(&systemVolRef, thePrefs);
		if (theErr == eofErr)			// Fail the read?  Maybe an old prefs version.
		{								// Delete it - we'll create a new one later.
			noProblems = DeletePrefs6(&systemVolRef);
			return(FALSE);				// Meanwhile, we'll use defaults.
		}
		else if (theErr != noErr)
			return(FALSE);
	}
										// Okay, maybe the read worked, but we still�
										// need to check the version number to see�
										// if it's current.
	if (thePrefs->prefVersion != versionNeed)
	{									// We'll delete the file if old version.
		if (canUseFSSpecs)
		{
			noProblems = DeletePrefs(&prefDirID, &systemVolRef);
			return(FALSE);
		}
		else
		{
			noProblems = DeletePrefs6(&systemVolRef);
			return(FALSE);
		}
	}
	
	return(TRUE);
}

