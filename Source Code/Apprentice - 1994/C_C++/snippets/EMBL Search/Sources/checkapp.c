/*
*********************************************************************
*	
*	CheckApp.c
*	If EMBL-Search is started by opening an EMBL-Search document from
*	the Finder
*	
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*
*********************************************************************
*	
*/ 

/*
****************************** Include files *************************
*/

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "checkapp.h"
#include "results.h"
#include "print.h"
#include "window.h"
#include "util.h"
#include "query.h"

/*
******************************** Global variables ******************
*/

extern Boolean gHasAppleEvents;
extern WDRec gWindows[MAXWIN];
extern Boolean gQuitApplication;


/**************************************
*	Handle information passed by Finder of Systems < 7.0
*	Return value:	none
*/

void CheckAppFiles()
{
	short		message,count;
	short		index;
	AppFile	theFile;
	WDPtr		dummy;

	if(gHasAppleEvents)					/* Apple Events do everything for us */
		return;
		
	CountAppFiles(&message,&count);	/* check startup information */
	if(message == appPrint)	{			/* print files */
		for(index=1;index<=count;index++) {	/* print files, one by one */
			GetAppFiles(index,&theFile);
			if(!DoDocPrint(theFile.fType,theFile.vRefNum,theFile.fName))
				break;
			ClrAppFiles(index);
		}
  	}
	else {
		for(index=1;index<=count;index++) {	/* open files, one by one */
			GetAppFiles(index,&theFile);
			if(!DoDocOpen(theFile.fType,theFile.vRefNum,theFile.fName,&dummy))
				break;
			ClrAppFiles(index);
   	}
   }
	/* should we ExitToShell if application was started from the Finder to
		print a document!? Couldn't find any clear interface guidelinesÉ */
}


/**************************************
*	Open document
*	Return value:	TRUE, if sucessful
*						FALSE, if an error occurred
*	By side-effect a WDPtr in wdpPtr, if a window was opened
*/

Boolean DoDocOpen(OSType fType,short vRefNum,StringPtr fName, WDPtr *wdpPtr)
{
	char		fullPath[256];
	short		w;
	Boolean	ret = TRUE;

	*wdpPtr = NULL;
	if( (w=GetFreeWindow()) == -1) {		/* find a free window */
		return(ErrorMsg(ERR_MAXWIN));
	}

  	StartWaitCursor();
	switch(fType) {
		case kQryFileType:
   		if ( (ret=LoadQuery(w,fName,vRefNum)) )
   			*wdpPtr = &gWindows[w];
	   	break;
		case kPrefFileType:		/* no special action required */
			break;
		case kResFileType:
   		if( (ret=Load1Results(w,fName,vRefNum)) )
   			*wdpPtr = &gWindows[w];
	   	break;
	}
	InitCursor();
	
	return(ret);
}

/**************************************
*	Print document
*	Return value:	TRUE, if sucessful
*						FALSE, if an error occurred
*/

Boolean DoDocPrint(OSType fType, short vRefNum, StringPtr fName)
{
	WDPtr wdp = NULL;
	
	if(fType == kQryFileType)
		return(TRUE);

	if(DoDocOpen(fType,vRefNum,fName,&wdp)) {	/* open window */
		if(wdp) {
			PrintIt(wdp);
			CloseMyWindow(wdp,TRUE);
		}
	
		/* See above: should we set gQuitApplication to TRUE if application was
		started from the Finder to print a document */
		return(TRUE);
	}
	else return(FALSE);
}