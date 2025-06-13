/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSUserProcs.c
**
**   Description:	Specific AppleEvent handlers used by the DropBox
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	MTC			Marshall Clow
**	SCS			Stephan Somogyi
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	06/23/94			LDR		Added support for ProcessItem and ProcessFolder handling
**	02/20/94			LDR		Modified Preflight & Postflight to take item count
**	01/25/92			LDR		Removed the use of const on the userDataHandle
**	12/09/91			LDR		Added the new SelectFile userProc
**								Added the new Install & DisposeUserGlobals procs
**								Modified PostFlight to only autoquit on odoc, not pdoc
**	11/24/91			LDR		Added the userProcs for pdoc handler
**								Cleaned up the placement of braces
**								Added the passing of a userDataHandle
**	10/29/91			SCS		Changes for THINK C 5
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								Added a bunch of comments for clarification
**	10/06/91	00:02	MTC		Converted to MPW C
**	04/09/91	00:02	LDR		Added to Projector
**
******************************************************************************/

/******************************************************************************
**
**	DropUnix history:	KRB = Kevin R. Boyce, RWD = Ryan Davis
**
**	04/21/95		KRB		Added conditional compilation for Think C (Universal headers,
**							 but old ANSI/console library).
**	04/??/95		RWD		First release.
**
********************************************************************************/

#include <Files.h>
#include <StandardFile.h>
#include <stdlib.h>			/* For malloc & free */
#include <string.h>			/* For strcpy */
#include <stdio.h>

#ifdef __MWERKS__
#include <SIOUX.h>			/* To manipulate the console */
#endif

#ifdef THINK_C
	#include <pascal.h>
	extern WindowPtr	gSplashScreen;		/* Needed for unhiliting THINK C console --KRB */
#endif

#include "DSGlobals.h"
#include "DSUserProcs.h"

// #define malloc	NewPtr
// #define free	DisposePtr
#define ValidatePtr(ptr) \
	if (!ptr) {\
		printf("Error allocating memory! Error #%d\n", MemError() );\
		exit(-1);\
	}

/* User Structs */
	typedef struct {
		unsigned int count;
		char ** names;
	} fileNameList, *fileNamePtr, **fileNameHandle;

/* Static Prototypes */
	static OSErr ProcessFolder(FSSpecPtr myFSSPtr, Handle userDataHandle);
	static OSErr ProcessItem(FSSpecPtr myFSSPtr, Handle userDataHandle);
	static void SetApplicationName(Handle userDataHandle);
	static void AllocateAppNames(Handle *userDataHandle, unsigned long numOfFiles);
	static void DeallocateAppNames(Handle userDataHandle);

/*
	This routine is called during init time.
	
	It allows you to install more AEVT Handlers beyond the standard four
*/

pascal void InstallOtherEvents (void) {
}


/*	
	This routine is called when an OAPP event is received.
	
	Currently, all it does is set the gOApped flag, so you know that
	you were called initally with no docs, and therefore you shouldn't 
	quit when done processing any following odocs.
*/

pascal void OpenApp (void) {
	gOApped = true;
}


/*	
	This routine is called when an QUIT event is received.
	
	We simply set the global done flag so that the main event loop can
	gracefully exit.  We DO NOT call ExitToShell for two reasons:
	1) It is a pretty ugly thing to do, but more importantly
	2) The Apple event manager will get REAL upset!
*/

pascal void QuitApp (void) {
	gDone = true;	/*	All Done! */
}


/*	
	This routine is the first one called when an ODOC or PDOC event is received.
	
	In this routine you would place code used to setup structures, etc. 
	which would be used in a 'for all docs' situation (like "Archive all
	dropped files")

	Obviously, the opening boolean tells you whether you should be opening
	or printing these files based on the type of event recieved.
	
	NEW IN 2.0!
	The itemCount parameter is simply the number of items that were dropped on
	the application and that you will be processing.  This gives you the ability
	to do a single preflight for memory allocation needs, rather than doing it
	once for each item as in previous versions.
	
	userDataHandle is a handle that you can create & use to store your own
	data structs.  This dataHandle will be passed around to the other 
	odoc/pdoc routines so that you can get at your data without using
	globals - just like the new StandardFile.  
	
	We also return a boolean to tell the caller if you support this type
	of event.  By default, our dropboxes don't support the pdoc, so when
	opening is FALSE, we return FALSE to let the caller send back the
	proper error code to the AEManager.
*/

pascal Boolean PreFlightDocs (Boolean opening, short itemCount, Handle *userDataHandle) {

	unsigned int index;

	if ((opening) && (itemCount > 0)) {	/* Don't support printing, so don't waste our time */

		/*
		** The following is a bug, it is the responsibility of ProcessItem to increment g_argc
		** g_argc += itemCount;
		*/

		AllocateAppNames(userDataHandle, itemCount);
		SetApplicationName(*userDataHandle);
	}

	return opening;		// we support opening, but not printing - see above
}

/*	
	This routine is called for each file passed in the ODOC event.
	
	In this routine you would place code for processing each file/folder/disk that
	was dropped on top of you.
*/

pascal void OpenDoc ( FSSpecPtr myFSSPtr, Boolean opening, Handle userDataHandle ) {

	OSErr	err = noErr;
	
	#ifdef qWalkFolders
		/*
			For this case we need to determine if the FSSpec is a file or folder.
			If it's a folder, we then need to process each item in that folder,
			otherwise just process the item.
		*/
		if (FSpIsFolder(myFSSPtr))
			err = ProcessFolder(myFSSPtr, userDataHandle);
		else
			err = ProcessItem(myFSSPtr, userDataHandle);
	#else
		/*
			For this case we just call ProcessItem on the FSSpec above.
		*/
		err = ProcessItem(myFSSPtr, userDataHandle);
	#endif
	
	// you should probably do something if you get back an error ;)
}


/*	
	This routine is the last routine called as part of an ODOC event.
	
	In this routine you would place code to process any structures, etc. 
	that you setup in the PreflightDocs routine.

	NEW IN 2.0!
	The itemCount parameter was the number of items that you processed.
	It is passed here just in case you need it ;)  
	
	If you created a userDataHandle in the PreFlightDocs routines, this is
	the place to dispose of it since the Shell will NOT do it for you!
*/

pascal void PostFlightDocs ( Boolean opening, short itemCount, Handle userDataHandle ) {

	int index;
	int argc;
	char ** argv;
	fileNamePtr fileNames;
	
	extern int Main(int argc, char **argv);

	/*
	** OK, this is IT! We have finally processed all the files, now we send the
	** pathnames & count off to the REAL unix function...
	*/
	HLock(userDataHandle);
		fileNames = *((fileNameHandle) userDataHandle);
		argc = fileNames->count;
		argv = fileNames->names;

		(void) Main(argc, argv);
	HUnlock(userDataHandle);
	/*
	** Now deallocate the memory stored by PreFlightDocs
	*/
	
	DeallocateAppNames(userDataHandle);
	
	/*
	** Reset the globals, this is in case the user invoked us by the menuitem,
	** the user _could_ run through again if wanted...
	*/
	
	#ifdef THINK_C
	/*
	** Unhilite the stupid THINK C console, so we get keypresses.
	** This may not be necessary if you have an up-to-date (8.0 or
	** paid-for 7.0) ANSI/console library.  Or it might, who knows?
	*/
	SelectWindow( gSplashScreen );
	#endif

	if ( (opening) && (!gOApped) )
		gDone = true;	//	close everything up!

	/*
		The reason we do not auto quit is based on a recommendation in the
		Apple event Registry which specifically states that you should NOT
		quit on a 'pdoc' as the Finder will send you a 'quit' when it is 
		ready for you to do so.
	*/
}

/*
	This routine gets called for any folder (or disk) that the caller wants 
	processed as a set of component items, instead of as a single entity.
	The determining factor is the definition of the qWalkFolder compiler directive.
*/
static OSErr ProcessFolder(FSSpecPtr myFSSPtr, Handle userDataHandle)
{
	OSErr		err = noErr;
	short		index, oldIndex, localIndex;
	FSSpec		localFSSpec, curFSSpec;
	CInfoPBRec	cipb;
	Str255		fName, vFName;
	long		dirID, origDirID;
	Boolean		foundPosition;
 
 	// copy the source locally to avoid recursion problems
 	BlockMoveData(myFSSPtr, &localFSSpec, sizeof(FSSpec));
 	
	//	get the dirID for THIS folder, not it's parent!
	BlockMoveData(localFSSpec.name, fName, 32);
	
	cipb.hFileInfo.ioCompletion	= 0L;
	cipb.hFileInfo.ioNamePtr	= fName;
	cipb.hFileInfo.ioVRefNum	= localFSSpec.vRefNum;
	cipb.hFileInfo.ioFDirIndex	= 0;	// use the dir & vRefNum;
	cipb.hFileInfo.ioDirID		= localFSSpec.parID;
	err = PBGetCatInfoSync(&cipb);
	
	if (!err) {		
		origDirID = cipb.dirInfo.ioDrDirID; // copy the sucker
		index = 1;
				
		// index through all contents of this folder
		while (err == noErr) {
			dirID = origDirID;
			localIndex = index;
			fName [0] = 0;
			cipb.hFileInfo.ioCompletion	= 0L;
			cipb.hFileInfo.ioNamePtr	= fName;
			cipb.hFileInfo.ioVRefNum	= localFSSpec.vRefNum;
			cipb.hFileInfo.ioFDirIndex	= localIndex;	// use a real index
			cipb.hFileInfo.ioDirID		= dirID;
			err = PBGetCatInfoSync(&cipb);

			if (!err) {
				BlockMoveData(fName, curFSSpec.name, 32);
				curFSSpec.vRefNum	= cipb.hFileInfo.ioVRefNum;
				curFSSpec.parID		= dirID;
			
				/*	
					Check to see if this entry is a folder.
				*/
				if (cipb.hFileInfo.ioFlAttrib & ioDirMask) {
					err = ProcessFolder(&curFSSpec, userDataHandle);
				 } else
					err = ProcessItem(&curFSSpec, userDataHandle);
			
				/*	If we've had an error, get out! */
				if (err)	break;

				// dirID = origDirID;	
				localIndex = index;	

				/*	
					Now take into account new files being created
					in the current directory & messing up our index.
					See Dev.CD Vol. XI:Tools & Apps (Moof!):Misc Utilities:
					Disinfectant & Source 2.5.1:Sample:Notes:Scan Alg	
				*/
				vFName [0] = 0;
				cipb.hFileInfo.ioCompletion	= 0L;
				cipb.hFileInfo.ioNamePtr	= vFName;
				cipb.hFileInfo.ioVRefNum	= localFSSpec.vRefNum;
				cipb.hFileInfo.ioFDirIndex	= localIndex;	// use a real index
				cipb.hFileInfo.ioDirID		= dirID;
				err = PBGetCatInfoSync(&cipb);
				oldIndex = index;
				if (!err) {
					/*	If they're equal - same place, go to next */
					if (EqualString (vFName, fName, false, false))
						index++;
				}
				
				/*	If we didn't advance, then perhaps a file was created or deleted */
				if (oldIndex == index) {
					oldIndex		= index;	/* save off the old */
					index			= 0;		/* and start at the beginning */
					err				= noErr;
					vFName [0]		= 0;
					foundPosition	= false;
					
					while (!foundPosition) {
						index++;
						vFName [0] = 0;
						cipb.hFileInfo.ioCompletion	= 0L;
						cipb.hFileInfo.ioNamePtr	= vFName;
						cipb.hFileInfo.ioVRefNum	= localFSSpec.vRefNum;
						cipb.hFileInfo.ioFDirIndex	= index;	/* now use a real index */
						cipb.hFileInfo.ioDirID		= dirID;
						err = PBGetCatInfoSync(&cipb);
						
						if (err == fnfErr) {  // we've just been deleted
							index = oldIndex;
							foundPosition = true;
							err = noErr;	// have to remember to reset this!
						}
						
					/*	found same file & same index position */
					/*	so try the next item */
						if ((!foundPosition) && EqualString(fName, vFName, false, false)) {
							index++;
							foundPosition = true;
						}
					}
				}
			}
		}
	}
	
	return(err);
}

/*
	This routine is called when the user chooses "Select FileÉ" from the
	File Menu.
	
	Currently it simply calls the new StandardGetFile routine to have the
	user select a single file (any type, numTypes = -1) and then calls the
	SendODOCToSelf routine in order to process it.  
			
	The reason we send an odoc to ourselves is two fold: 1) it keeps the code
	cleaner as all file openings go through the same process, and 2) if events
	are ever recordable, the right things happen (this is called Factoring!)

	Modification of this routine to only select certain types of files, selection
	of multiple files, and/or handling of folder & disk selection is left 
	as an exercise to the reader.
*/
pascal void SelectFile (void)
{
	StandardFileReply	stdReply;
	SFTypeList			theTypeList;

	StandardGetFile(NULL, -1, theTypeList, &stdReply);
	if (stdReply.sfGood)	// user did not cancel
		SendODOCToSelf(&stdReply.sfFile);	// so send me an event!
}

/*
	This routine is called during the program's initialization and gives you
	a chance to allocate or initialize any of your own globals that your
	dropbox needs.
	
	You return a boolean value which determines if you were successful.
	Returning false will cause DropShell to exit immediately.
*/

pascal Boolean InitUserGlobals(void)
{
	Str255 appName;
	
	#ifdef __MWERKS__
	SIOUXSettings.initializeTB	= FALSE;
	SIOUXSettings.setupmenus	= FALSE;
	#endif

	return(true);	// nothing to do, it we must be successful!
}

/*
	This routine is called during the program's cleanup and gives you
	a chance to deallocate any of your own globals that you allocated 
	in the above routine.
*/
pascal void DisposeUserGlobals(void)
{
}

long GetFullPath (FSSpec *fsspec, char *path, OSErr *err);
unsigned short ReverseCopyP2CStr (unsigned char *pas, char *c);
void ReverseCStr (char *str);

/*
	This routine gets called for each item (which could be either a file or a folder)
	that the caller wants dropped.  The determining factor is the definition of the 
	qWalkFolder compiler directive.   Either way, the item in question should be
	processed as a single item and not "dissected" into component units (like subfiles
	of a folder!)
*/
OSErr ProcessItem(FSSpecPtr myFSSPtr, Handle userDataHandle)
{
	char	path[1000];
	OSErr	err = noErr;
	unsigned long	len;	
	
	len = GetFullPath (myFSSPtr, path, &err);
	if (err != noErr) {
		return err;
	}

	if (path != NULL) {
		fileNamePtr fileNames;
		
		HLock(userDataHandle);
			fileNames = *((fileNameHandle) userDataHandle);
			
			fileNames->names[(fileNames->count)] = (char *) NewPtr((len+1) * sizeof(char));
			ValidatePtr(fileNames->names[(fileNames->count)]);
	
			strcpy(fileNames->names[(fileNames->count)], path);
			(fileNames->count)++;
		HUnlock(userDataHandle);
	}

	return(err);
}

long GetFullPath (FSSpec *fsspec, char *path, OSErr *err)
{
	short		volume = fsspec->vRefNum;
	OSErr		fsErr = noErr;
	CInfoPBRec	catinfo;
	char			dirName[64];
	long			retLen = 0L, len;
	register char	*p = path;
	
	// All comments assume a  file whose full path is
	//	HD:Fonts:Garamond
	
	// If fsspec doesn't designate a file (i.e., it's a volume or directory), we write a colon
	if ( FSpIsFolder (fsspec)) {
		*p++ = ':';
		retLen++;
	}
	
	// First, copy the file name backwards from a Pascal to a C string:
	//	"\pGaramond"  becomes "dnomaraG"

	len = (long) ReverseCopyP2CStr (fsspec->name, p);
	p += len;
	retLen += len;
	
	/* Yea, I know this is cheap, but it is used ONCE AND it makes it clearer {RWD} */
	
	#define FSpIsVolume(FSSpec) ((FSSpec)->parID == fsRtParID)
	
	if ( ! FSpIsVolume (fsspec)) {		// Don't do anything more if we've got a volume
	
		catinfo.dirInfo.ioVRefNum = volume;
		catinfo.dirInfo.ioNamePtr = (StringPtr) dirName;
		catinfo.dirInfo.ioDrParID = fsspec->parID;
		
		// Now copy the rest of the path, one level at a time, backwards:
		//	"dnomaraG:stnoF:DH"
		do {
			catinfo.dirInfo.ioFDirIndex = -1;
			catinfo.dirInfo.ioDrDirID = catinfo.dirInfo.ioDrParID;	// <= This is the key Ñ go from the
														//	current folder to its parent
			fsErr = PBGetCatInfoSync (&catinfo);
			if (fsErr == noErr) {
				*p++ = ':';
				retLen += 1 + (len = ReverseCopyP2CStr ((StringPtr) dirName, p));
				p += len;
			} else {
				*err = fsErr;
				return retLen;
			}
		} while (catinfo.dirInfo.ioDrDirID != fsRtDirID);
	}
	
	// Finally, reverse the string and return its length and any error that might have occurred (ReverseCopyP2CStr has
	//	already appended a null character, so we don't have to worry about it here)
	ReverseCStr (path);
	*err = fsErr;
	return retLen;
}

unsigned short ReverseCopyP2CStr (register unsigned char *pas, register char *c)
{
	register short	i, len = *pas++;
	
	for (i = len, c += len; i > 0; i--)
		*--c = *pas++;
	c += len;
	*c = '\0';
	return len;
}

void ReverseCStr (register char *str)
{
	register short		n = 0L, i;
	register char		t, *p1, *p2;
	
	p1 = p2 = str;
	while (*p2++)
		;
	p2--;
	n = (p2 - p1) / 2;
	for (i = n; i > 0; i--) {
		t = *--p2;
		*p2 = *p1;
		*p1++ = t;
	}
}

static void SetApplicationName(Handle userDataHandle) {

	Str255 appName;
	fileNamePtr fileNames;
	
	GetMyAppName(appName);
	
	HLock(userDataHandle);
		fileNames = *((fileNameHandle) userDataHandle);
		if ( fileNames->names[0] == NULL) {
			fileNames->names[0] = (char *) NewPtr(**fileNames->names + 1); /* pstr's size in byte 0 */
			if (fileNames->names[0] == NULL || MemError()) {
				SysBeep(60);
				ExitToShell();
			}
			strcpy(fileNames->names[0], p2cstr(appName));
		}
	HUnlock(userDataHandle);
}

static void AllocateAppNames(Handle *userDataHandle, unsigned long numOfFiles) {

	fileNameHandle fileNameHdl;
	fileNamePtr fileNames;
	fileNameHdl = (fileNameHandle) NewHandleClear(sizeof(fileNameList));
	if (!fileNameHdl || MemError() ) {
		SysBeep(60);
		ExitToShell();
	}
	
	HLock((Handle) fileNameHdl);
		fileNames = *fileNameHdl;
		
		fileNames->count = 1; /* we count one right off for the app name */
		fileNames->names = (char **) NewPtrClear((numOfFiles + 1) * sizeof(char *));
		if (fileNames->names == NULL) {
			SysBeep(60);
			ExitToShell();
		}
		fileNames = NULL;
	HUnlock((Handle) fileNameHdl);
	
	*userDataHandle = (Handle) fileNameHdl;
}

static void DeallocateAppNames(Handle userDataHandle) {

	fileNamePtr fileNames;
	unsigned int index;
	
	HLock(userDataHandle);
		fileNames = *((fileNameHandle) userDataHandle);
		for (index = 0; index < fileNames->count; index++) {
			if (fileNames->names[index] != NULL) {
				DisposePtr(fileNames->names[index]);
				if (MemError()) {
					SysBeep(60);
					ExitToShell();
				}
				fileNames->names[index] = NULL;
			}
		}
		DisposePtr((Ptr) fileNames->names);
		if (MemError()) {
			SysBeep(60);
			ExitToShell();
		}
		fileNames = NULL;
	HUnlock(userDataHandle);
	
	DisposeHandle(userDataHandle);
	if (MemError()) {
		SysBeep(60);
		ExitToShell();
	}
}
