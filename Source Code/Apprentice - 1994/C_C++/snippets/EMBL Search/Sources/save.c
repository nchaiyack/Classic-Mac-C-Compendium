/*
*********************************************************************
*	
*	save.c
*	Saving window contents to disk
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <stdio.h>
#include <string.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "save.h"
#include "util.h"
#include "hitstorage.h"
#include "pstr.h"
#include "sequence.h"
#include "query.h"
#include "window.h"
#include "export.h"

static Boolean SaveResData(Str255 fName, short vRefNum, ResultHdl resHdl);
static Boolean SaveResRsrc(Str255 fName, ResultHdl resHdl);
static Boolean SaveQueryRsrc(Str255 fName, QueryHdl queryHdl);
static Boolean SaveRes(WDPtr wdp, Str255 fName, short vRefNum);
static Boolean SaveSeq(WDPtr wdp, Str255 fName, short vRefNum);
static Boolean SaveQuery(WDPtr wdp, Str255 fName, short vRefNum);
static Boolean SaveResAs(WDPtr wdp);
static Boolean SaveSeqAs(WDPtr wdp);
static Boolean SaveQueryAs(WDPtr wdp);



/*
******************************** Globals *****************************
*/

extern CursHandle	gCursor[8];
extern short		gCurrentCursor;
extern char			gError[256];
extern DBInfo		gDBInfo[DB_NUM];
extern Prefs		gPrefs;

/**************************************
*	Save routines dispatcher
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

Boolean DoSave(WDPtr wdp,Boolean save)
{
	Boolean ret=TRUE;
	
	if( wdp == NULL)
		return(FALSE);
		
	switch( ((WindowPeek)wdp)->windowKind) {
		case queryW:
			if(save && wdp->inited)
				/* if Save� selected and if a file already exists */
				ret=SaveQuery(wdp, wdp->fName,wdp->vRefNum);
			else
				ret=SaveQueryAs(wdp);
			break;
		case seqW:
			ret=SaveSeqAs(wdp);
			break;
		case resW:
			if(save && wdp->inited)
				/* if Save� selected and if a file already exists */
				ret=SaveRes(wdp, wdp->fName,wdp->vRefNum);
			else
				ret=SaveResAs(wdp);
			break;
		default: SysBeep(10);
	}
	return(ret);
}


/**************************************
* 	Create data fork of result file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveResData(Str255 fName, short vRefNum,ResultHdl resHdl)
{
	short				output;
	long				count;
	CString80Hdl	bufHdl;
	SignedByte		oldState;
	short				hitPos, bufPos;

	/* Create and open a new file */
	if( CreateMacFile(fName,vRefNum,kApplSignature,kResFileType,TRUE) != noErr )
	   return(FALSE);

	if( OpenMacFile(fName,vRefNum,&output, TRUE) != noErr )
	  	return(FALSE);	
	
	/* Lock down result data */
	bufHdl = (**resHdl).descBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
	
	/* now save hit by hit */
	StartWaitCursor();
	for(	hitPos = 0,bufPos = 0;
			hitPos < (**resHdl).nhits;
			++hitPos, ++bufPos) {
		RotateWaitCursor();
		/* refill buffer if necessary */
		if( hitPos < (**resHdl).buftop || hitPos >= (**resHdl).buftop + MAXBUFLINES) {
			if( !FillDEBuffer(resHdl,hitPos,FALSE) )
				break;
			bufPos = 0;
		}
		
		/* write output */
		count = (long) pstrlen((StringPtr)(*bufHdl)[bufPos]);
		if( WriteMacFile(output, &count, (*bufHdl)[bufPos]+1,fName, TRUE) != noErr ) 
   		break;
   	/* add newline */
   	count = 1L;
   	if( WriteMacFile(output,&count, "\r",fName,TRUE) != noErr)
   		break;
	}
	FSClose(output);
	HSetState((Handle)bufHdl,oldState);
	InitCursor();
	
	return( hitPos == (**resHdl).nhits );
}


/**************************************
*	Create resource fork of result file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveResRsrc(Str255 fName, ResultHdl resHdl)
{  
	short			refNum;
	HitmapHdl 	hitmapHdl;
	QueryHdl		queryHdl;
	OSErr			err;
	
	/* open resource fork			*/
   
   CreateResFile(fName);
	if((refNum=OpenResFile(fName)) == -1) {
		sprintf(gError,LoadErrorStr(ERR_OPENRES,FALSE),
					PtoCstr(fName),ResError());
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}

	/* Store query */
	
	queryHdl = (**resHdl).queryHdl;
	AddResource((Handle)queryHdl,kQueryRsrcType,kStdRsrc,kQueryRsrcName);
	if( (err=ResError()) != noErr ) {
		DetachResource((Handle)queryHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	WriteResource((Handle)queryHdl);	/* and write it (we have to clear the
												change attr before a DetachResource() */
	if( (err=ResError()) != noErr ) {				/* error 						*/
		DetachResource((Handle)queryHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	/* Get back control over queryHdl from resource mgr */
	DetachResource((Handle)queryHdl);

	/* Now store hitmap */
	
	hitmapHdl = (**resHdl).hitmapHdl;
	AddResource((Handle)hitmapHdl,kHitmapRsrcType,kStdRsrc,kHitmapRsrcName);
	if( (err=ResError()) != noErr ) {
		DetachResource((Handle)hitmapHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	WriteResource((Handle)hitmapHdl);	/* and write it (we have to clear the
												change attr before a DetachResource() */
	if( (err=ResError()) != noErr ) {				/* error 						*/
		DetachResource((Handle)hitmapHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	/* Get back control over hitmapHdl from resource mgr */
	DetachResource((Handle)hitmapHdl);

	CloseResFile(refNum);
	return(TRUE);
}


/**************************************
*	Create resource fork of query file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveQueryRsrc(Str255 fName, QueryHdl queryHdl)
{  
	short			refNum;
	OSErr			err;
	
	/* open resource fork			*/
   
   CreateResFile(fName);
	if((refNum=OpenResFile(fName)) == -1) {
		sprintf(gError,LoadErrorStr(ERR_OPENRES,FALSE),
					PtoCstr(fName),ResError());
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}

	/* Store query */
	
	AddResource((Handle)queryHdl,kQueryRsrcType,kStdRsrc,kQueryRsrcName);
	if( (err=ResError()) != noErr ) {
		DetachResource((Handle)queryHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	WriteResource((Handle)queryHdl);	/* and write it (we have to clear the
												change attr before a DetachResource() */
	if( (err=ResError()) != noErr ) {				/* error 						*/
		DetachResource((Handle)queryHdl);
      CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_WRITERES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	/* Get back control over queryHdl from resource mgr */
	DetachResource((Handle)queryHdl);

	WriteNameStringRsrc(refNum);	/* write name string resource (for system 7.0) */
	
	CloseResFile(refNum);
	return(TRUE);
}


/**************************************
*	Save result data to a file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveRes(WDPtr wdp, Str255 fName, short vRefNum)
{
	ResultHdl	resHdl;
	Str255		volName;
	short			oldVRefNum;
	Boolean		ret;
	
	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return(FALSE);

	StartWaitCursor();
	if ( !SaveResData(fName,vRefNum,resHdl) )
		return(FALSE);
	
	/* set default directory */
	GetVol(volName,&oldVRefNum);
	SetVol(NULL,vRefNum);
	
	ret = SaveResRsrc(fName,resHdl);
	
	FlushVol(NULL,vRefNum);
	SetVol(NULL,oldVRefNum);
	
	/* update window descr. record */
	if(ret) {
		wdp->dirty = FALSE;
		wdp->inited = TRUE;
	}
	InitCursor();
	return(ret);
}

/**************************************
*	Save a sequence
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveSeq(WDPtr wdp, Str255 fName, short vRefNum)
{
	SeqRecHdl	seqRecHdl;
	Boolean		ret;
	
	seqRecHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqRecHdl == NULL)
		return(FALSE);

	StartWaitCursor();
	if ( !SaveSeqData(fName,vRefNum,seqRecHdl) )
		return(FALSE);
	
	/* update window descr. record */
	wdp->dirty = FALSE;
	wdp->inited = TRUE;

	InitCursor();
	return(ret);
}

/**************************************
*	Save query data to a  file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveQuery(WDPtr wdp, Str255 fName, short vRefNum)
{
	QueryHdl		queryHdl;
	Str255		volName;
	short			oldVRefNum;
	Boolean		ret;
	FInfo			fndrInfo;
	
	queryHdl = (QueryHdl)(wdp->userHandle);
	if(queryHdl == NULL)
		return(FALSE);

	UpdateQueryRec(wdp);
	
	StartWaitCursor();

	/* Delete any old version */
	DeleteMacFile(fName,vRefNum);
	
	/* set default directory */
	GetVol(volName,&oldVRefNum);
	SetVol(NULL,vRefNum);
	
	ret = SaveQueryRsrc(fName,queryHdl);
	                    
	/* update Finder information */
   GetFInfo(fName,vRefNum,&fndrInfo);
   fndrInfo.fdType = kQryFileType;
   fndrInfo.fdCreator = kApplSignature;
   SetFInfo(fName,vRefNum,&fndrInfo);
	FlushVol(NULL,vRefNum);
	SetVol(NULL,oldVRefNum);
	
	/* update window descr. record */
	if(ret) {
		wdp->dirty = FALSE;
		wdp->inited = TRUE;
	}
	InitCursor();
	return(ret);
}

/**************************************
*	Save results in a new file 
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveResAs(WDPtr wdp)
{
	Str255	newFName;
	short		newVRefNum;
	Boolean	ret=FALSE;
		
	if(GetNewFilename(wdp->fName,&newVRefNum,newFName)) {
		ret=SaveRes(wdp,newFName,newVRefNum);
		if( ret ) {
			DelWindowFromMenu(wdp->fName);
			pstrcpy(wdp->fName,newFName);
			wdp->vRefNum = newVRefNum;
			SetWTitle((WindowPtr)wdp,newFName);
			AddWindowToMenu(newFName);
		}
	}
	return(ret);
}

/**************************************
*	Save sequence in a new file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveSeqAs(WDPtr wdp)
{
	Str255	ename,oldFName,newFName;
	short		newVRefNum;
	Boolean	ret=FALSE;

	GetWTitle((WindowPtr)wdp,ename);
	BuildFName(oldFName,ename,gPrefs.format);

	if(GetNewFilename(oldFName,&newVRefNum,newFName)) {
		ret=SaveSeq(wdp,newFName,newVRefNum);
		if( ret ) {
			pstrcpy(wdp->fName,newFName);
			wdp->vRefNum = newVRefNum;
		}
	}
	return(ret);
}

/**************************************
*	Save query in a new file 
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean SaveQueryAs(WDPtr wdp)
{
	Str255	newFName;
	short		newVRefNum;
	Boolean	ret=FALSE;
		
	if(GetNewFilename(wdp->fName,&newVRefNum,newFName)) {
		ret=SaveQuery(wdp,newFName,newVRefNum);
		if( ret ) {
			DelWindowFromMenu(wdp->fName);
			pstrcpy(wdp->fName,newFName);
			wdp->vRefNum = newVRefNum;
			SetWTitle((WindowPtr)wdp,newFName);
			AddWindowToMenu(newFName);
		}
	}
	return(ret);
}

