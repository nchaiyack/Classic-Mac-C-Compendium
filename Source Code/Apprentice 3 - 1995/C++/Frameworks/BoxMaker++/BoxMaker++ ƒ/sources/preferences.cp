#include <Files.h>
#include <Finder.h>
#include <Folders.h>

#include "preferences.h"

template<class C> preferences<C>::preferences( Str255 fName, const C &defaults,
	OSType fileCreator) : iFileCreator( fileCreator)
{
	//
	// if the preferences file exists already, use the defaults specified in it
	// instead of the defaults passed to us.
	//
	long foundDirID;
	short foundVRefnum;

	short theFile;
	long numtoread = sizeof( C);
	
	(void)FindFolder( kOnSystemDisk, 'pref', kCreateFolder, &foundVRefnum, &foundDirID);
	(void)FSMakeFSSpec( foundVRefnum, foundDirID, fName, &theFileSpec);
	
	if( FSpOpenDF( &theFileSpec, fsCurPerm, &theFile) == noErr)
	{
		if( FSRead( theFile, &numtoread, (Ptr)&theDefaults) != noErr)
		{
			//
			// Could not read entire preferences file, possibly because of a
			// version change => use defaults
			//			
			theDefaults = defaults;
		}
		(void)FSClose( theFile);
	} else {
		theDefaults = defaults;
	}
	//
	// Start with default values:
	//
	*(C*)this = theDefaults;
}

template<class C> preferences<C>::~preferences()
{
	if( defaultsChanged())
	{
		long foundDirID;
		short foundVRefnum;
		//
		// Create the preferences file, in case it doesn't exist yet
		// This will fail if the file exists already, but we don't care
		// about that.
		//
		short theFile;
		FSpCreateResFile( &theFileSpec, iFileCreator, 'pref', 0);
		if( ResError() == 0)
		{
			//
			// First time through: put 'STR ' resource in,
			// if it can be found in the application.
			//
			const short applResFile = CurResFile();
			theFile = FSpOpenResFile( &theFileSpec, fsWrPerm);
			if( theFile != -1)
			{
				UseResFile( applResFile);
				Handle theRes = Get1Resource( 'STR ', kPreferencesInfo);
				if( theRes != 0)
				{
					DetachResource( theRes);
					UseResFile( theFile);
					AddResource( theRes, 'STR ', kPreferencesInfo, "\p");
					ReleaseResource( theRes);	// AddResource made it a resource again
				}
				CloseResFile( theFile);
			}
			//
			// Set 'Name Locked' flag of preferences file:
			//
			FInfo theInfo;
			if( FSpGetFInfo( &theFileSpec, &theInfo) == noErr)
			{
				theInfo.fdFlags |= kNameLocked;
				(void)FSpSetFInfo( &theFileSpec, &theInfo);
			}
		}
		if( FSpOpenDF( &theFileSpec, fsWrPerm, &theFile) == noErr)
		{
			long numtowrite = sizeof( C);
			(void)FSWrite( theFile, &numtowrite, (Ptr)(C*)this);
			FSClose( theFile);
		}
	}
}

template<class C> Boolean preferences<C>::defaultsChanged() const
{
	const char *one   = (const char *)this;
	const char *other = (const char *)&theDefaults;
	const int numBytes = sizeof( C);
	Boolean result = false;
	
	for( int i = 0; i < numBytes; i++)
	{
		if( *one++ != *other++)
		{
			result = true;
			break;
		}
	}
	return result;
}

