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

#include <StandardFile.h>

#include "DSGlobals.h"
#include "DSUserProcs.h"

// Static Prototypes
static OSErr ProcessFolder(FSSpecPtr myFSSPtr);


/*
	Uncomment this line if you want each item of a dropped folder processed
	as an individual item
*/
// #define qWalkFolders


/*
	This routine is called during init time.
	
	It allows you to install more AEVT Handlers beyond the standard four
*/
#pragma segment Main
pascal void InstallOtherEvents (void) {
}


/*	
	This routine is called when an OAPP event is received.
	
	Currently, all it does is set the gOApped flag, so you know that
	you were called initally with no docs, and therefore you shouldn't 
	quit when done processing any following odocs.
*/
#pragma segment Main
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
#pragma segment Main
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

	You will probably want to remove the #pragma unused (currently there to fool the compiler!)
*/
#pragma segment Main
pascal Boolean PreFlightDocs (Boolean opening, short itemCount, Handle *userDataHandle) {
#pragma unused ( itemCount )
#pragma unused ( userDataHandle )

	return opening;		// we support opening, but not printing - see above
}


/*	
	This routine is called for each file passed in the ODOC event.
	
	In this routine you would place code for processing each file/folder/disk that
	was dropped on top of you.
	
	You will probably want to remove the #pragma unused (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void OpenDoc ( FSSpecPtr myFSSPtr, Boolean opening, Handle userDataHandle ) {
#pragma unused ( myFSSPtr )
#pragma unused ( opening )
#pragma unused ( userDataHandle )
	OSErr	err = noErr;
	
	
	#ifdef qWalkFolders
	/*
		For this case we need to determine if the FSSpec is a file or folder.
		If it's a folder, we then need to process each item in that folder,
		otherwise just process the item.
	*/
	if (FSpIsFolder(myFSSPtr))
		err = ProcessFolder(myFSSPtr);
	else
		err = ProcessItem(myFSSPtr);
	#else
	/*
		For this case we just call ProcessItem on the FSSpec above.
	*/
	err = ProcessItem(myFSSPtr);
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
	
	You will probably want to remove the #pragma unusued (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void PostFlightDocs ( Boolean opening, short itemCount, Handle userDataHandle ) {
#pragma unused ( opening )
#pragma unused ( itemCount )
#pragma unused ( userDataHandle )

	if ( (opening) && (!gOApped) )
		gDone = true;	//	close everything up!

	/*
		The reason we do not auto quit is based on a recommendation in the
		Apple event Registry which specifically states that you should NOT
		quit on a 'pdoc' as the Finder will send you a 'quit' when it is 
		ready for you to do so.
	*/
}


#ifdef nodef
/*
	This routine gets called for each item (which could be either a file or a folder)
	that the caller wants dropped.  The determining factor is the definition of the 
	qWalkFolder compiler directive.   Either way, the item in question should be
	processed as a single item and not "dissected" into component units (like subfiles
	of a folder!)
*/
OSErr ProcessItem(FSSpecPtr myFSSPtr)
{
	OSErr	err = noErr;
	
	// do something here
	
	return(err);
}
#endif

/*
	This routine gets called for any folder (or disk) that the caller wants 
	processed as a set of component items, instead of as a single entity.
	The determining factor is the definition of the qWalkFolder compiler directive.
*/
static OSErr ProcessFolder(FSSpecPtr myFSSPtr)
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
					err = ProcessFolder(&curFSSpec);
				 } else
					err = ProcessItem(&curFSSpec);
			
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
	This routine is called when the user chooses "Select File�" from the
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
	return(true);	// nothing to do, it we must be successful!
}

/*
	This routine is called during the program's cleanup and gives you
	a chance to deallocate any of your own globals that you allocated 
	in the above routine.
*/
pascal void DisposeUserGlobals(void)
{
	// nothing to do for our sample dropbox
}
