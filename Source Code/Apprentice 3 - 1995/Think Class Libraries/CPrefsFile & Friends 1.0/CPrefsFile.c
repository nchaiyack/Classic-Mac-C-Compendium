/*
 * CPrefsFile.c
 * This THINK C 5 object, which must be subclassed, makes storing data in
 * the Preferences folder almost painless.  It requires the TCL.
 * Version 1.1, 13 April 1992
 *
 * by Jamie McCarthy
 * © Copyright 1992 by James R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 * To implement preferences, you need to do two things:  create a subclass
 * of CPrefsFile tailored to your application, and add a preferences
 * resource to your project resource file.  It is strongly recommended
 * that you also do a third thing, namely create a 'STR#' resource for
 * CPrefsFile to use.
 *
 * There are two sample subclasses distributed with CPrefsFile, COOPrefs
 * and CGlobalPrefs.  There is a sample preferences resource and 'STR#'
 * resource that could be used with either of them.
 *
 * BRIEF OVERVIEW OF CPREFSFILE:
 *
 * CPrefsFile supports preferences stored in one (1) resource.  CPrefsFile
 * takes care of the mundane stuff:  finding the Preferences folder and
 * doing the opening, closing, reading, and writing from the file.  Your
 * subclass has to help by overriding three methods, and you may want to
 * write access methods to get at the resource.
 *
 * Everything is extremely customizable.  I spent quite some time making
 * the class as general as possible, and as override-able as possible.
 *
 * IPrefsFile() takes three arguments:  its strings ID, the preferences
 * file's fileType, and a Boolean indicating whether or not to look for
 * the preferences file in the same folder as the application.  The
 * strings ID is discussed later.  The fileType should be whatever you
 * like.  ('PREF' is suggested;  it seems to be a de facto standard.)
 *
 * When the object is initialized, the prefs are read in.  Your prefs
 * are thus always accessible, from the time you call
 * IYourSubclassOfPrefsFile() to the time you Dispose() of it.  The
 * preferences file is always kept open.
 *
 * To write the current (possibly modified) preferences to disk, call
 * writePrefs().  To throw away the current prefs and restore them to
 * what's in the preferences file, call readPrefs().  To throw away the
 * current prefs and restore the defaults, call readDefaultPrefs().  To
 * throw away the prefs on disk and replace them with the defaults,
 * call readDefaultPrefs() followed by writePrefs().
 *
 * THE BIG THREE METHOD OVERRIDES:
 *
 * You must override assignToPrefsHndl(), valueOfPrefsHndl(), and
 * sizeOfPrefs().  The sample code succinctly demonstrates how to
 * do this.
 *
 * THE 'STR#' RESOURCE:
 *
 * This resource is optional, but recommended;  if it's not there,
 * CPrefsFile uses the default, non-localizable, English strings when
 * errors occur.  Pass the resource's ID in strsID to IPrefsFile, or
 * zero for no resource.
 *
 * If any string is either blank (length zero) or missing, the default
 * string will be used in its place.
 *
 * The first string in the list is the text that will be displayed if
 * the prefs in the preferences file are invalid.  The default is
 * #defined as "kPFPrefsWereInvalidDefaultStr".
 *
 * The second string is the text that will be displayed if the default
 * prefs, stored in the application or the project resource file, are
 * invalid.  The default is "kPFDefaultPrefsWereInvalidDefaultStr".
 *
 * The third string is the beginning of the notice that will be
 * displayed if the preferences file cannot be written.  The default
 * is "kPFCannotWriteDefaultStr".
 *
 * The fourth string is the end of the notice, if the file is locked.
 * The default is "kPFFileLockedDefaultStr".
 *
 * The sixth string is the end of the notice, if the disk is locked.
 * The default is "kPFDiskLockedDefaultStr".
 *
 * The fifth string is the end of the notice, if the file is busy.
 * The default is "kPFFileBusyDefaultStr".
 *
 * The seventh string is the end of the notice, if there is something
 * else that's preventing the file from being written. The default
 * is "kPFUnknownErrorDefaultStr".
 *
 * The eighth string is the name of your application.  Use this string
 * only if you want copies of your app with differing names to use the
 * same preferences file.  For example, you might want both "MyApp 1.0"
 * and "MyApp 1.1" to use the file "MyApp Prefs".  The default is the
 * current name of the app.
 *
 * The ninth string is the name of the preferences folder, if Gestalt
 * can't find it.  The default is the English "Preferences".  I'm not
 * sure why you'd want to change this, unless your application was
 * going to be used exclusively on (say) French systems.  Then, you'd
 * put in "Les Préferènçieresment" or whatever the French folder is named
 * under System 7, so that if your user ran the program under both System
 * 6 and 7, her preferences would always be in the right place.  Most
 * people should just leave this blank.
 *
 * The tenth string is reserved for future use and should be left blank.
 *
 * The eleventh and subsequent strings are suffixes, one of which will be
 * appended to the name of your app to produce the name of the prefs file.
 * If none are found, " Preferences" or " Prefs" (note the leading spaces)
 * is used.  It uses the first suffix that, together with the application
 * name, comes to under 31 characters.  If all the suffices are too long,
 * it uses the last (or " Prefs"), and trims the app name down with an
 * ellipsis to make the name exactly 30 characters long.
 *
 * If you have every string--that is, if every entry of the 'STR#' is
 * meaningfully filled out--you can #define the constact
 * "everyCPrefsFileStrIsGuaranteed" in your project prefix to be TRUE.
 * That'll save you a few bytes of string data.
 *
 * EASY CUSTOMIZATION OF CPREFSFILE:
 *
 * If you want your preferences resource to be stored with a different
 * type or id, you may override the getRsrc...() methods as well.  (For
 * example, you might want to do this so your ResEdit template doesn't
 * conflict with someone else's.)  The resource name doesn't matter and
 * is normally blank;  override getRsrcName() if you don't like that.
 *
 * Prefs are tested for "acceptability" after they are read from and
 * before they are written to disk.  If they fail the test, the
 * default prefs are used instead.  The default test is just to check
 * the length of the handle.  If you want more strenuous tests,
 * override areTheCurrentPrefsAcceptable() and do them yourself!
 * You'll probably want to "&&" CPrefsFile's result with yours.
 *
 * If you never want to create a file on disk, override
 * getPrefsCanBeWritten() to return FALSE.
 *
 * LESS EASY CUSTOMIZATION OF CPREFSFILE:
 *
 * The remaining methods of CPrefsFile are divided into four groups:
 * low-level convenience methods, preference-manipulation methods,
 * acceptability-checking methods, and the three high-level methods.
 * The first and last groups you'll probably never need to touch;
 * if you do, you won't need my help anyway.  In the third group,
 * you may want to fiddle with displayAlert() if you want a fancier
 * way of indicating invalid preferences, but that should be fairly
 * straightforward.
 *
 * CPrefsFile raises an exception for invalid default prefs.  You
 * should only get that error if you've forgotten to change them,
 * or if your application has been tampered with.  Nonetheless, if
 * you prefer to handle the exception yourself, put something
 * appropriate into the CATCH handler for the writePrefs() or
 * readDefaultPrefs() call.  Again, you probably won't need my help.
 *
 * If you want to store the preferences differently, however, you'll
 * want to override the three methods in the second group:
 * forgetPrefsHndl(), getPrefsHndl(), and addPrefsHndl().  By
 * default, these three just call DisposHandle(), Get1Resource(),
 * and AddResource(), respectively, on your prefs handle.  If, for
 * example, you want your preferences spread out over ten resources,
 * you'll have to override these to handle those extra resources.
 * CPrefsFile does some miscellaneous calls on your "main" prefs
 * resource (the one that getPrefsHndl() returns), such as
 * DetachResource(), but it bottlenecks through these three methods
 * to dispose, read, or write.
 *
 * CHANGE HISTORY:
 *
 * Changes from 1.0 to 1.1:
 * Added seven error strings to the 'STR#' list.
 * Put default strings and string indices into constants.
 * Bottlenecked default string handling through getIndPrefsString().
 *    (Suggested by R.Exner@trl.oz.au.)
 * Put in tests for the file being unwriteable and unreadable.
 * Added sizeOfPrefs() and areTheCurrentPrefsAcceptable() calls.
 * Removed getFilePermission() and added getPrefsCanBeWritten().
 * Added ForgetResource() of the 'STR#' after using it.
 * Added the constant everyCPrefsFileStrIsGuaranteed.
 * Did miscellaneous code clean-ups.
 *
 * FUTURE PLANS:
 *
 * Near future:  a subclass that easily handles remembering
 *    application windows' (not document windows') placement
 *    on-screen.  (Suggested by R.Exner@trl.oz.au.)
 * Far future:  a subclass that reads a specified 'TMPL' from the
 *    app and (if necessary) modifies the prefs resource and your
 *    subclass' _code_, making adding prefs a snap.
 * Very far future:  a subclass that does the same thing from a
 *    'DITL', and also handles display and editing.  Wouldn't
 *    codeless preferences be wonderful?
 *
 */



/******************************/

#include "CPrefsFile.h"

/******************************/

#include <Errors.h>
#include <GestaltEqu.h>
#include <Folders.h>

#include <Constants.h>
#include <TBUtilities.h>
#include <CError.h>

/******************************/

enum {
	kPFPrefsWereInvalidStrIndex = 1,
	kPFDefaultPrefsWereInvalidStrIndex,
	kPFCannotWriteStrIndex,
	kPFFileLockedStrIndex,
	kPFDiskLockedStrIndex,
	kPFFileBusyStrIndex,
	kPFUnknownErrorStrIndex,
	kPFApplicationNameStrIndex,
	kPFPrefsFolderNameStrIndex,
	kPFReserved,
	kPFFirstPrefsSuffixStrIndex
} ;

#define kPFEmptyStr ((unsigned char *) "\p")
#define kPFPrefsWereInvalidDefaultStr ((unsigned char *) \
	"\pThe preferences were invalid, and were replaced by the defaults.")
#define kPFDefaultPrefsWereInvalidDefaultStr ((unsigned char *) \
	"\pThe default preferences are invalid. This is a fatal error.")
#define kPFCannotWriteDefaultStr ((unsigned char *) \
	"\pYou won’t be able to save preferences, because ")
#define kPFFileLockedDefaultStr ((unsigned char *) \
	"\pthe file is locked.")
#define kPFDiskLockedDefaultStr ((unsigned char *) \
	"\pthe disk is locked.")
#define kPFFileBusyDefaultStr ((unsigned char *) \
	"\pthe file is busy.")
#define kPFUnknownErrorDefaultStr ((unsigned char *) \
	"\pof an unexpected error.")
#define kPFPrefsFolderNameDefaultStr ((unsigned char *) "\pPreferences")
#define kPFLongPrefsSuffixDefaultStr ((unsigned char *) "\p Preferences")
#define kPFShortPrefsSuffixDefaultStr ((unsigned char *) "\p Prefs")

#if !defined(everyCPrefsFileStrIsGuaranteed)
#define everyCPrefsFileStrIsGuaranteed (FALSE)
#endif

/******************************/



void CPrefsFile::IPrefsFile(short strsID, OSType fileType,
	Boolean lookInApplFolderAlso)
{
		/*
		 * This initialization procedure must be called before opening
		 * any resource files besides the application, or it will not
		 * work when run as a project.  Running as a standalone should
		 * work in any case, but why tempt fate?
		 */
	
	Handle dummyHndl;
	Boolean wasLocked;
	extern OSType gSignature;
	
	inherited::IResFile();
	
	wasLocked = Lock(TRUE);
	
	itsStrsID = strsID;
	itsFileType = fileType;
	itsRsrcType = getRsrcType();
	itsRsrcID = getRsrcID();
	getRsrcName(itsRsrcName);
	itsPermissionType = fsRdWrPerm;
	itsPrefsCouldBeWritten = TRUE;
	isReadingDefaultPrefs = FALSE;
	
	GetAppParms(itsApplName, &itsApplRefNum, &dummyHndl);
	itsProjRsrcRefNum = CurResFile();
	if (itsProjRsrcRefNum == itsApplRefNum) itsProjRsrcRefNum = 0;
	
	Lock(wasLocked);
	
	findName();
	findFolder(lookInApplFolderAlso);
	
	forgetStrListResource();
	
	if (!ExistsOnDisk()) {
		tryToCreateNew();
	}
	
	if (ExistsOnDisk()) {
		tryToOpen();
	}
	
		/*
		 * This will read from the file if it's open, and from the app
		 * if it's not.
		 */
	readPrefs();
	
	if (getPrefsCanBeWritten()) {
		tryToWrite();
	}
}



void CPrefsFile::Dispose(void)
	/* Subclasses shouldn't need to override this method unless they allocate
	 * memory in their initialization method.  If you need to clean up memory
	 * which you allocated in readPrefs or readDefaultPrefs (which you won't,
	 * unless you override them), do so in forgetPrefsHndl.
	 */
{
	forgetPrefsHndl();
	inherited::Dispose();
}



Boolean CPrefsFile::getPrefsCanBeWritten(void)
{
	return ExistsOnDisk() && itsPrefsCouldBeWritten;
}



OSType CPrefsFile::getRsrcType(void)
	/* Return the resource type that the preferences are stored in.
	 * Subclasses may override this method, but they don't have to.
	 */
{
	return 'PREF';
}



short CPrefsFile::getRsrcID(void)
	/* Return the resource ID that the preferences are stored in.
	 * Subclasses may override this method, but they don't have to.
	 */
{
	return 128;
}



void CPrefsFile::getRsrcName(Str63 theName)
	/* Return the name of the resource that the preferences are stored in.
	 * Note that this is used for writing only; the name isn't
	 * significant when the prefs are read.
	 * Subclasses may override this method, but they don't have to.
	 */
{
	FailNIL(theName);
	CopyPString(kPFEmptyStr, theName);
}



Boolean CPrefsFile::areTheCurrentPrefsAcceptable(void)
	/* This is called by checkPrefsForAcceptability().  It returns TRUE
	 * if the prefs handle contains reasonable values.  You can redefine
	 * "reasonable" by overriding this method, though you don't have to.
	 * This code defines "reasonable" as "being of the right length."
	 */
{
	Boolean returnValue = FALSE;
	FailNIL(valueOfPrefsHndl());
	returnValue = (GetHandleSize(valueOfPrefsHndl()) == sizeOfPrefs());
	return returnValue;
}



void CPrefsFile::assignToPrefsHndl(Handle theHndl)
	/* Your subclass must override this method.  It's pretty simple--
	 * you just assign the generic-type Handle to your own handle.  This
	 * method serves only to handle the type coercion gracefully.
	 */
{
	SubclassResponsibility();
}



Handle CPrefsFile::valueOfPrefsHndl(void)
	/* Your subclass must override this method.  It's pretty simple--
	 * you just return your own handle as a (Handle).  This method serves
	 * only to handle the type coercion gracefully.
	 */
{
	SubclassResponsibility();
}



short CPrefsFile::sizeOfPrefs(void)
	/* Your subclass must override this method.  It's a one-liner--
	 * you just return the size of your preferences data.
	 */
{
	SubclassResponsibility();
}



Boolean CPrefsFile::getIndPrefsString(unsigned char *theString, short index)
	/* The return value indicates whether the string was found in a
	 * resource (TRUE), or was "hard-coded" in (FALSE);  that value
	 * can almost always be ignored.
	 * Subclasses may override this method, but normally shouldn't have to.
	 */
{
	if (itsStrsID != 0) {
		GetIndString(theString, itsStrsID, index);
	} else {
		CopyPString(kPFEmptyStr, theString);
	}
	
	if (theString[0] != 0) {
		
			/* The string was in the resource file. */
		
		return TRUE;
		
	}
	
#if (!everyCPrefsFileStrIsGuaranteed)
	
	else {
		
			/* The string was blank or not there.  Substitute the default. */
		
		switch (index) {
			
			case kPFPrefsWereInvalidStrIndex:
				CopyPString(kPFPrefsWereInvalidDefaultStr, theString);
				break;
				
			case kPFDefaultPrefsWereInvalidStrIndex:
				CopyPString(kPFDefaultPrefsWereInvalidDefaultStr, theString);
				break;
				
			case kPFCannotWriteStrIndex:
				CopyPString(kPFCannotWriteDefaultStr, theString);
				break;
				
			case kPFFileBusyStrIndex:
				CopyPString(kPFFileBusyDefaultStr, theString);
				break;
				
			case kPFDiskLockedStrIndex:
				CopyPString(kPFDiskLockedDefaultStr, theString);
				break;
				
			case kPFUnknownErrorStrIndex:
				CopyPString(kPFUnknownErrorDefaultStr, theString);
				break;
				
			case kPFApplicationNameStrIndex:
				CopyPString(this->itsApplName, theString);
				break;
				
			case kPFPrefsFolderNameStrIndex:
				CopyPString(kPFPrefsFolderNameDefaultStr, theString);
				break;
				
			case kPFFirstPrefsSuffixStrIndex:
				CopyPString(kPFLongPrefsSuffixDefaultStr, theString);
				break;
				
			case kPFFirstPrefsSuffixStrIndex + 1:
				CopyPString(kPFShortPrefsSuffixDefaultStr, theString);
				break;
				
			case kPFReserved:
			default:
					/* Leave the string blank. */
				break;
				
		}
	}
	
#endif
	
	return FALSE;
}



void CPrefsFile::displayAlert(short firstIndex, short secondIndex)
	/* The alert-displaying code was stolen from CError::PostAlert().
	 * Subclasses are welcome to override this method,
	 * but they don't have to.
	 */
{
	Str255 firstStr, secondStr;
	
	getIndPrefsString(firstStr, firstIndex);
	if (secondIndex >= 1) {
		getIndPrefsString(secondStr, secondIndex);
	} else {
		secondStr[0] = 0;
	}
	
	ConcatPStrings(firstStr, secondStr);
	
	ParamText(firstStr, NULL, NULL, NULL);
	PositionDialog('ALRT', ALRTgeneral);
	InitCursor();
	Alert(ALRTgeneral, NULL);
	
	forgetStrListResource();
}



void CPrefsFile::forgetStrListResource(void)
	/* Subclasses may override this method, but they don't have to. */
{
	Handle theStrsResource;
	theStrsResource = GetResource('STR#', itsStrsID);
	ForgetResource(theStrsResource);
}



void CPrefsFile::findName(void)
	/* Subclasses should not use or override this method. */
{
	Str63 theName, suffix;
	short cStrIndex;
	Boolean foundName;
	Boolean stillHaveSuffices;
	
	foundName = FALSE;
	stillHaveSuffices = TRUE;
	
	getIndPrefsString(theName, kPFApplicationNameStrIndex);
	
	cStrIndex = kPFFirstPrefsSuffixStrIndex;
	
	while (!foundName && stillHaveSuffices) {
		getIndPrefsString(suffix, cStrIndex);
		if (suffix[0] == 0) {
			stillHaveSuffices = FALSE;
		} else {
			if (theName[0] + suffix[0] < 31) {
				ConcatPStrings(theName, suffix);
				foundName = TRUE;
			} else {
				++cStrIndex;
			}
		}
	}
	
	if (!foundName) {
			/* None of the strings were short enough.  Use the last one,
			 * and squish the application's name to make it fit.
			 */
		getIndPrefsString(suffix, cStrIndex - 1);
		theName[0] = 30-suffix[0];
		theName[theName[0]] = '…';
		ConcatPStrings(theName, suffix);
	}
	
	CopyPString(theName, this->name);
}



void CPrefsFile::findFolder(Boolean lookInApplFolderAlso)
	/* Subclasses should not use or override this method. */
{
	Boolean foundFolder;
	short myFoundVRefNum;
	long myFoundDirID;
	HParamBlockRec params;
	CInfoPBRec cInfo;
	extern tSystem gSystem;
	
	foundFolder = FALSE;
	
	if (lookInApplFolderAlso) {
		
			/* Look in the application's folder */
		
		Str63 volName;
		WDPBRec wdpb;
		
			/* Get the current (application's folder) working directory */
		FailOSErr(GetVol(volName, &myFoundVRefNum));
		wdpb.ioCompletion = NULL;
		wdpb.ioNamePtr = NULL;
		wdpb.ioVRefNum = myFoundVRefNum;
		wdpb.ioWDIndex = 0;
		wdpb.ioWDProcID = 0;
		wdpb.ioWDVRefNum = 0;
		FailOSErr(PBGetWDInfo(&wdpb, FALSE));
		
		myFoundVRefNum = wdpb.ioWDVRefNum;
		myFoundDirID = wdpb.ioWDDirID;
		
		this->volNum = myFoundVRefNum;
		this->dirID = myFoundDirID;
		
		if (ExistsOnDisk()) {
			foundFolder = TRUE;
		}
	}
	
	if (!foundFolder && gSystem.hasGestalt) {
		
			/* Use Gestalt to look in the Preferences folder */
		
		long foldAttr;
		OSErr theOSErr;
		
		theOSErr = Gestalt(gestaltFindFolderAttr, &foldAttr);
		if ((theOSErr == noErr) && (foldAttr & (1L << gestaltFindFolderPresent))) {
			
			FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType,
				TRUE, /* create the folder, if necessary */
				&myFoundVRefNum, &myFoundDirID));
			foundFolder = TRUE;
			
		}
		
	}
	
	if (!foundFolder) {
		
			/* There's no Gestalt;  look manually in the Preferences folder */
		
		Str63 theFolder;
		SysEnvRec theWorld;
		OSErr theOSErr;
		
		theOSErr = SysEnvirons(2, &theWorld);
		if (theOSErr == envNotPresent || theOSErr == envBadVers) {
			FailOSErr(theOSErr);
		}
		
		getIndPrefsString(theFolder, kPFPrefsFolderNameStrIndex);
		
		params.volumeParam.ioCompletion = NULL;
		params.volumeParam.ioNamePtr = NULL;
		params.volumeParam.ioVRefNum = theWorld.sysVRefNum;
		params.volumeParam.ioVolIndex = 0;
		FailOSErr(PBHGetVInfo(&params, FALSE));
		
		myFoundVRefNum = params.volumeParam.ioVRefNum;
		
			/* Look for the folder */
		cInfo.dirInfo.ioCompletion = NULL;
		cInfo.dirInfo.ioNamePtr = theFolder;
		cInfo.dirInfo.ioVRefNum = theWorld.sysVRefNum;
		cInfo.dirInfo.ioFDirIndex = 0;
		cInfo.dirInfo.ioDrDirID = 0;
		
		if (PBGetCatInfo(&cInfo, FALSE) == fnfErr) {
				/* There's no folder in the blessed folder called "Preferences"
				 * (or whatever we decided is its proper name).  Make one.
				 */
			FailOSErr(DirCreate(theWorld.sysVRefNum, 0, theFolder, &myFoundDirID));
#if 0
			params.fileParam.ioCompletion = NULL;
			params.fileParam.ioNamePtr = theFolder;
			params.fileParam.ioVRefNum = theWorld.sysVRefNum;
			params.fileParam.ioFVersNum = 0;
			params.fileParam.ioFDirIndex = 0;
			params.fileParam.ioDirID = 0;
			FailOSErr(PBDirCreate(&params, FALSE));
			myFoundDirID = params.fileParam.ioDirID;
#endif
		} else {
			myFoundDirID = cInfo.dirInfo.ioDrDirID;
		}
	}
	
	this->volNum = myFoundVRefNum;
	this->dirID = myFoundDirID;
}



void CPrefsFile::tryToCreateNew(void)
{
	TRY {
		CreateNew(gSignature, itsFileType);
	} CATCH {
		itsPrefsCouldBeWritten = FALSE;
		displayCannotWritePrefsAlert(gLastError);
		NO_PROPAGATE;
	} ENDTRY;
}



void CPrefsFile::tryToOpen(void)
{
	TRY {
		Open(itsPermissionType);
	} CATCH {
		itsPrefsCouldBeWritten = FALSE;
		if (itsPermissionType != fsRdPerm) {
			itsPermissionType = fsRdPerm;
			displayCannotWritePrefsAlert(gLastError);
			RETRY;
		} else {
			readDefaultPrefs();
			NO_PROPAGATE;
		}
	} ENDTRY;
}



void CPrefsFile::tryToWrite(void)
{
	short oldResFile;
	Handle testHndl;
	oldResFile = CurResFile();
	testHndl = NULL;
	TRY {
		MakeCurrent();
		testHndl = NewHandle(0);
		AddResource(testHndl, 'test', 0, kPFEmptyStr);
		FailResError();
		RmveResource(testHndl);
		ForgetHandle(testHndl);
	} CATCH {
		ReleaseResource(testHndl);
		if (ResError() == resNotFound) {
			ForgetHandle(testHndl);
		} else {
			testHndl = NULL;
		}
		UseResFile(oldResFile);
		itsPrefsCouldBeWritten = FALSE;
		displayCannotWritePrefsAlert(gLastError);
		NO_PROPAGATE;
	} ENDTRY;
}



void CPrefsFile::forgetPrefsHndl(void)
{
	if (valueOfPrefsHndl() != NULL) {
		DisposHandle(valueOfPrefsHndl());
		assignToPrefsHndl(NULL);
	}
}



Handle CPrefsFile::getPrefsHndl(void)
{
	return Get1Resource(itsRsrcType, itsRsrcID);
}



void CPrefsFile::addPrefsHndl(Handle theHndl)
{
	Str63 theRsrcName;
	FailNIL(theHndl);
	getRsrcName(theRsrcName);
	AddResource(theHndl, itsRsrcType, itsRsrcID, theRsrcName);
}



void CPrefsFile::displayCannotWritePrefsAlert(OSErr theReason)
{
	switch (theReason) {
		
		case afpAccessDenied:	// if locked when file sharing is on
		case afpObjectLocked:	// if we're in a foreign file system (I think) (not bloody likely!)
		case fLckdErr:
		case permErr:				// could be anything, but this seems most likely
			displayAlert(kPFCannotWriteStrIndex, kPFFileLockedStrIndex);
			break;
			
		case wPrErr:
		case vLckdErr:
				/* I don't distinguish between a hardware and a software lock.
				 * Most people don't really care, do they?
				 */
			displayAlert(kPFCannotWriteStrIndex, kPFDiskLockedStrIndex);
			break;
			
		case afpFileBusy:
		case opWrErr:
		case fBsyErr:
				/* Actually, I wouldn't expect to get either fBsyErr or permErr,
				 * because we're dealing with the Resource Manager.  But you
				 * never can tell with the File Mananger...
				 */
			displayAlert(kPFCannotWriteStrIndex, kPFFileBusyStrIndex);
			break;
			
		default:
			displayAlert(kPFCannotWriteStrIndex, kPFUnknownErrorStrIndex);
			break;
			
	}
}



void CPrefsFile::displayInvalidPrefsAlert(void)
{
	displayAlert(kPFPrefsWereInvalidStrIndex, 0);
}



void CPrefsFile::displayInvalidDefaultPrefsAlert(void)
{
	displayAlert(kPFDefaultPrefsWereInvalidStrIndex, 0);
}



void CPrefsFile::checkPrefsForAcceptability(void)
{
	if (!areTheCurrentPrefsAcceptable()) {
		if (isReadingDefaultPrefs) {
			displayInvalidDefaultPrefsAlert();
			Failure(kSilentErr, 0);
		} else {
			displayInvalidPrefsAlert();
			readDefaultPrefs();
			if (getPrefsCanBeWritten()) writePrefs();
		}
	}
}



void CPrefsFile::readPrefs(void)
	/* Reads in preferences from the prefs file.  The resource is detached
	 * immediately after being read, so it's just a plain handle.
	 * Subclasses may override this method, but normally shouldn't have to.
	 */
{
	if (!IsOpen()) {
		
		readDefaultPrefs();
		
	} else {
		
		short oldResFile;
		short theReturnVal;
		
		forgetPrefsHndl();
		
		oldResFile = CurResFile();
		MakeCurrent();
		
		TRY {
			assignToPrefsHndl(getPrefsHndl());
			if (valueOfPrefsHndl() == NULL) {
				
					/* Silently put the default prefs into the file. */
				readDefaultPrefs();
				if (getPrefsCanBeWritten()) writePrefs();
				
			} else {
				
				DetachResource(valueOfPrefsHndl());
				
			}
			
			checkPrefsForAcceptability();
			
			UseResFile(oldResFile);
		} CATCH {
			UseResFile(oldResFile);
		} ENDTRY;
		
	}
}



void CPrefsFile::writePrefs(void)
	/* Writes out preferences to the prefs file.  After this method
	 * returns, variables are in a state as if readPrefs() had just been
	 * called.  Specifically, the prefs resource has been detached.
	 * Subclasses may override this method, but normally shouldn't have to.
	 */
{
	short oldResFile;
	Handle oldPrefsHndl;
	short theReturnVal;
	Boolean oldResLoad;
	
	checkPrefsForAcceptability();
	
	ASSERT(getPrefsCanBeWritten());
	
	oldResFile = CurResFile();
	oldResLoad = ResLoad;
	
	TRY {
		MakeCurrent();
		SetResLoad(FALSE);
		
		oldPrefsHndl = getPrefsHndl();
		
		SetResLoad(TRUE);
		
		if (oldPrefsHndl != NULL) {
			RmveResource(oldPrefsHndl);
			DisposHandle(oldPrefsHndl);
		}
		
		addPrefsHndl(valueOfPrefsHndl());
		UpdateResFile(this->refNum);
		
		DetachResource(valueOfPrefsHndl());
		forgetPrefsHndl();
		
		readPrefs();
		
		SetResLoad(oldResLoad);
		UseResFile(oldResFile);
	} CATCH {
		SetResLoad(oldResLoad);
		UseResFile(oldResFile);
	} ENDTRY;
}



void CPrefsFile::readDefaultPrefs(void)
	/* Gets the default preferences from the appropriate resource in the
	 * application.
	 * Subclasses may override this method, but normally shouldn't have to.
	 */
{
	Handle defPrefsHndl;
	short oldResFile;
	
	isReadingDefaultPrefs = TRUE;
	
	forgetPrefsHndl();
	
	oldResFile = CurResFile();
	
	TRY {
		UseResFile(itsApplRefNum);
		assignToPrefsHndl(getPrefsHndl());
		if (valueOfPrefsHndl() == NULL && itsProjRsrcRefNum != 0) {
			UseResFile(itsProjRsrcRefNum);
			assignToPrefsHndl(getPrefsHndl());
			if (valueOfPrefsHndl() == NULL) FailResError();
		}
		DetachResource(valueOfPrefsHndl());
		
		checkPrefsForAcceptability();
		
		UseResFile(oldResFile);
		isReadingDefaultPrefs = FALSE;
	} CATCH {
		UseResFile(oldResFile);
		isReadingDefaultPrefs = FALSE;
	} ENDTRY;
}
