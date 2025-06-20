/*
	Preferences.h
*/

#pragma once

#include	"FileUtils.h"
#include	"ResourceUtils.h"
#include	"StringUtils.h"

#define	eCouldntSavePrefs		1
#define	rPrefsRosterType		'PrRo'
#define	rPrefsRosterID		128

typedef struct {
	Handle		resHndl;
	ResType		resType;
	short		resID;
	short		flags;		// This may be used in a future version � leave the low 8 bits alone if you use this field
} PrefsRosterEntry;

typedef struct {
	short			refNum;			// RefNum of the file the resources belong to
	short			numRsrcs;		// Number of resources
	PrefsRosterEntry	rsrc[1];
} PrefsRosterList;

class Preferences {

	protected:
		short			appResFork;			// A refNum to the application's resource fork (must never == -1)
		OSType			fileCreator;			// Signature of the application that created the file
		OSType			fileType;				// The file's type
		PrefsRosterList	**prefsRoster;			// Roster of resource types and IDs � used for resource housekeeping
		short			numPrefs;			// Number of preferences resources
		FSSpec			file;					// The preferences file
		short			fileResFork;			// A refNum to the file's resource fork (-1 if !fileExists)
		Boolean			fileExists;			// Does the preferences file referred to in this->file exist?
		Boolean			canSaveFile;			// Can we save changes to the file?
		Boolean			usingAppResources;	// Are we using the preferences resources in the application file?
		
	public:
							Preferences (void);
							~Preferences (void);
		virtual Boolean		Init (short appRF, OSType creator, OSType type);
		virtual Boolean		UseFile (FSSpec *fss);
		virtual Handle			GetPrefResource (short index);
		virtual OSErr			DetachPrefResource (short index);
		virtual OSErr			ReleasePrefResource (short index);
		virtual OSErr			SavePrefResource (short index);
		
	protected:
		virtual Boolean		VerifyResources (PrefsRosterList **roster);
		virtual void			CloseFile (void);
		virtual OSErr			CopyPrefs (short destRF);
		virtual PrefsRosterList	**InitPrefsRoster (short refNum);
		virtual Handle			ReadPrefResource (short index);
};

