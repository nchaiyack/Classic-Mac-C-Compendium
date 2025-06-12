/*
OpenPreferencesFolder.c

Open the Preferences folder (in the Macintosh System Folder), creating it if
necessary.

All the Macintosh-dependent code is conditionally compiled, anticipating that
code will be added to support other operating systems, especially DOS and
Windows.

OpenPreferencesWD() is a stand-alone routine that finds the Preferences folder
(creating it if necessary) and allocates to it a working directory number that
should subsequently be deallocated by calling CloseWD().

The purpose of OpenPreferencesFolder() and ClosePreferencesFolder() is to
isolate the Macintosh dependence to this file, on the premise that many programs
will want to access files stored in the  Preferences folder, and that such
programs might be portable to other computers if a new routine is supplied to
open an appropriate "Preferences" directory. The real work is done by
OpenPreferencesWD(). OpenPreferencesFolder() and ClosePreferencesFolder() share
hidden state variables.

The FindFolder routine appeared with System 7, but, according to THINK
Reference, under System 6 it's implemented by glue code inserted by the compiler
(THINK C 5 or MPW 3.2 or better), but the glue code will only give us the System
Folder.

NOTE:
This routine calls several Apple routines for which the glue is in THINK C's
MacTraps2, not in MacTraps. So be sure to add both to your project if you use
OpenPreferencesFolder. If you forget you'll get an error message from the Linker.

HISTORY:
6/13/93	dgp wrote it.
9/15/93	dgp	made Mac-specific code conditional.
*/
#include <VideoToolbox.h>
#if MAC_C
	#include <Files.h>
	#include <Folders.h>
	#include <Errors.h>
	static short oldVRefNum,wdRefNum;
#endif

short OpenPreferencesFolder(void)
{
	short error=1;

	#if MAC_C
		error=GetVol(NULL,&oldVRefNum);
		if(error)return error;
		error=OpenPreferencesWD(&wdRefNum);
		if(error)return error;
		error=SetVol(NULL,wdRefNum);
	#endif
	return error;
}

short ClosePreferencesFolder(void)
{
	short error=1;

	#if MAC_C
		error=SetVol(NULL,oldVRefNum);
		if(error)return error;
		error=CloseWD(wdRefNum);
	#endif
	return error;
}

#if MAC_C
	OSErr OpenPreferencesWD(short *wdRefNumPtr)
	/*
	Gets a working directory number for the Preferences folder in the active
	System Folder. A Preferences folder is created if it doesn't already exist.
	*/
	{
		short wdRefNum,vRefNum;
		long dirID,createdDirID;
		OSErr error;
		WDPBRec pb;
		static unsigned char prefs[]="\pPreferences";
	
		#if 0	/* Support System 7 only */
			error=FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder
				,&vRefNum,&dirID);
			if(error)return error;
			error=OpenWD(vRefNum,dirID,0,wdRefNumPtr);
		#else	/* Support both Systems 6 and 7 */
			error=FindFolder(kOnSystemDisk,kSystemFolderType,kDontCreateFolder
				,&vRefNum,&dirID);
			if(error)return error;
			pb.ioWDProcID=0;
			pb.ioNamePtr=prefs;
			pb.ioWDDirID=dirID;
			pb.ioVRefNum=vRefNum;
			error=PBOpenWD(&pb,0);
			if(error==fnfErr){
				error=DirCreate(vRefNum,dirID,prefs,&createdDirID);
				if(error)return error;
				pb.ioVRefNum=vRefNum;
				error=PBOpenWD(&pb,0);
			}
			*wdRefNumPtr=pb.ioVRefNum;
		#endif
		return error;
	}
#endif