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


#include "DSGlobals.h"
#include "DSUserProcs.h"
#include "string.h"
#include "ftp.h"
#include "pref.h"


	char 	host[256];
	char 	user[256];
	char 	pswd[256];
	
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
pascal Boolean PreFlightDocs (Boolean opening, Handle *userDataHandle) {
#pragma unused ( userDataHandle )

	return opening;		// we support opening, but not printing - see above
}
void xstrcpy(char *s1,char *s2);
void xstrcpy(char *s1,char *s2)
{
	register char *p2 = s2;
	register char *p1 = s1;
	while (*p2) {
		*p1++ = *p2++;
	}
}
/*	
	This routine is called for each file passed in the ODOC event.
	
	In this routine you would place code for processing each file/folder/disk that
	was dropped on top of you.
	
	You will probably want to remove the #pragma unused (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void OpenDoc ( FSSpecPtr myFSSPtr, Boolean opening, Handle userDataHandle ) {


	char 	thefile[256];
	Ptr		data;
	long	filesize;
	short	refNum;
	short oe;
	Handle h;
	
//#pragma unused ( myFSSPtr )
//#pragma unused ( opening )
//#pragma unused ( userDataHandle )

	if (FSpOpenDF(myFSSPtr,fsRdPerm,&refNum)== noErr) {
		oe=GetEOF(refNum,&filesize);
		if ((h=NewHandle(filesize)) !=nil) {
			oe=FSRead(refNum,&filesize,*h);
			HLock(h);
			strcpy(thefile,p2cstr((StringPtr)myFSSPtr->name));
			
			FTPPutFile (host, user, pswd, thefile,*h,filesize);
			HUnlock(h);
			DisposeHandle(h);
		}
		oe=FSClose(refNum);
		InitCursor();
	}
}


/*	
	This routine is the last routine called as part of an ODOC event.
	
	In this routine you would place code to process any structures, etc. 
	that you setup in the PreflightDocs routine.

	If you created a userDataHandle in the PreFlightDocs routines, this is
	the place to dispose of it since the Shell will NOT do it for you!
	
	You will probably want to remove the #pragma unusued (currently there to fool the compiler!)
*/
#pragma segment Main
pascal void PostFlightDocs ( Boolean opening, Handle userDataHandle ) {
#pragma unused ( opening )
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

	ReadPref();
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
