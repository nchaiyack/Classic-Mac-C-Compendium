/*
 * CPrefsFile.h
 * Version 1.1, 13 April 1992
 *
 * � Copyright 1992 by James R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 *
 */



/******************************/

#pragma once

/******************************/

#include <CResFile.h>

/******************************/



class CPrefsFile : public CResFile {
	
public:
	
	void			IPrefsFile(short strsID, OSType fileType,
						Boolean lookInApplFolderAlso);
	void			Dispose(void);
	
	
		/****	Methods which your subclass may override, if desired. ****/
	
	Boolean		getPrefsCanBeWritten(void);		// Can the prefs be written?
	
protected:
	
	OSType		getRsrcType(void);					// The resource type
	short			getRsrcID(void);						// The resource ID
	void			getRsrcName(Str63 theName);		// The resource name
	Boolean		areTheCurrentPrefsAcceptable(void);		// Verify they're OK
	
	
		/****	Methods which your subclass MUST override. ****/
	
	void			assignToPrefsHndl(Handle theHandle);
	Handle		valueOfPrefsHndl(void);
	short			sizeOfPrefs(void);
	
	
		/****	Methods which your subclass normally won't have to override. ****/
	
		/* Low-level convenience methods. */
	Boolean		getIndPrefsString(unsigned char *theString, short index);
	void			displayAlert(short firstIndex, short secondIndex);
	void			forgetStrListResource(void);
	void			findName(void);
	void			findFolder(Boolean lookInApplFolderAlso);
		/* Methods that mess with the file and/or determine its status. */
	void			tryToCreateNew(void);
	void			tryToOpen(void);
	void			tryToWrite(void);
		/* Preference-manipulating methods. */
	void			forgetPrefsHndl(void);
	Handle		getPrefsHndl(void);
	void			addPrefsHndl(Handle theHndl);
		/* Method to handle an error with the prefs file. */
	void			displayCannotWritePrefsAlert(OSErr theError);
		/* Methods to check acceptability and handle invalid prefs. */
	void			displayInvalidPrefsAlert(void);
	void			displayInvalidDefaultPrefsAlert(void);
	void			checkPrefsForAcceptability(void);
		/* High-level methods. */
	void			readPrefs(void);
	void			writePrefs(void);
	void			readDefaultPrefs(void);
	
	
		/****	Class variables. ****/
	
	Str31			itsApplName;	// Name of the application
	short			itsApplRefNum;	// Reference number for application's resource fork
	short			itsProjRsrcRefNum;	// If run from ThC, refNum for .�.rsrc file
	short			itsStrsID;		// 'STR#' rsrc to search for the "Prefs" appendads
	OSType		itsFileType;	// Type of prefs file
	OSType		itsRsrcType;	// Resource type to use to store data in prefs file
	short			itsRsrcID;		// Resource ID to use
	Str63			itsRsrcName;	// Name to give the resource
	short			itsPermissionType;		// Permission with which to open the file
	Boolean		itsPrefsCouldBeWritten;	// Did IPrefsFile succeed in writing?
	Boolean		isReadingDefaultPrefs;	// Are we in the middle of reading defaults?
	
} ;
