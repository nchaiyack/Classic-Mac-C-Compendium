/*
*********************************************************************
*	
*	Export.c
*	Export of database entries
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <string.h>
#include <stdio.h>
#include <Notification.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "export.h"
#include "util.h"
#include "hitstorage.h"
#include "window.h"
#include "pstr.h"
#include "events.h"

static Boolean Export1(HitlistHdl hlHdl, short pos, short output, short dbcode, StringPtr outFName, char *inpBuf, char *outBuf, DialogPtr myDialog);
static Boolean WriteEMBLEntry(char *inpBuf, char *outBuf, short output, StringPtr outFName, long count);
static Boolean WriteSeqOnly(char *inpBuf, char *outBuf, short output, StringPtr outFName, long count);
static short GetExportFilename(StringPtr oldFName, short *vRefNum, StringPtr newFName);
static pascal short myExportDlgHook(short theItem, DialogPtr myDialog);
static Boolean UserBreak(DialogPtr myDialog);
static void WaitUntilForeground(void);
static void NotifyUser(NMRec *myNMPtr);
static pascal void NotifyResponse(NMRec *myNMPtr);
static void RemoveNotification(NMRec *myNMPtr);



/*
********************************* Globals *****************************
*/

extern Prefs 		gPrefs;
extern DBInfo		gDBInfo[DB_NUM];
extern IndexFiles	gFileList;
extern char			gError[256];
extern short		gSysVersion;
extern Boolean		gInBackground;
extern Rect			gDragRect;
extern MenuHandle	gMenu[MENUNR];

static Boolean lbSkipButton,lbSaveAllButton;


/**************************************
*	Export entries
*/

void ExportRes(WDPtr wdp)
{
	ResultHdl		resHdl;
	HitlistHdl		hlHdl;
	CString80Hdl	bufHdl;
	Str255			ename,oldFName,newFName;
	short 			vRefNum;
	short				i, found;
	short				output=0;
	SignedByte		oldState;
	Boolean			ret,ret2;
	char 				*inpBuf,*outBuf;
	DialogPtr		myDialog = NULL;
	DialogTHndl		myHandle;
	Boolean			bUserBreak = FALSE, bUserNotified = FALSE;
	NMRec				myNotification;
	
	if( wdp == NULL || ((WindowPeek)wdp)->windowKind != resW )
		return;
		
	resHdl = (ResultHdl)(wdp->userHandle);
	if( !resHdl )
		return;
		
	hlHdl = (**resHdl).hlHdl;
	if( !hlHdl )
		return;
	oldState = LockHandleHigh((Handle)hlHdl);
	
	bufHdl = (**resHdl).descBufHdl;
	
	/* Allocate file buffers */
	inpBuf=NewPtr((Size)FILEBUF);
	if(inpBuf)
		if( !(outBuf=NewPtr((Size)FILEBUF)) )
			DisposPtr((Ptr)inpBuf);
	if ( !inpBuf || !outBuf ) {
		HSetState((Handle)hlHdl,oldState);
		ErrorMsg(ERR_MEMORY);
		return;
	}
		
	StartWaitCursor();
	
	/* Loop for all hits */
	for(i=0,found=0,lbSaveAllButton = bUserBreak = FALSE;
		 i < (**resHdl).nhits && bUserBreak == FALSE;
		 ++i) {
		/* and export those which are selected */
		if( GetSelectState(hlHdl,i) ) {
			if( i < (**resHdl).buftop || i >= (**resHdl).buftop + MAXBUFLINES) {
				ret2 = FillDEBuffer(resHdl,i,FALSE);
				if( !ret2 )
					break;
			}

			ret = OK;
			
			ExtractEname(bufHdl,i-(**resHdl).buftop,ename);
			
			/* if necessary, ask user for filename */
			if( lbSaveAllButton == FALSE ) {
				InitCursor();
				BuildFName(oldFName,ename,gPrefs.format);
				ret=GetExportFilename(oldFName,&vRefNum,newFName);
				if( ret == OK ) {
					if( CreateMacFile(newFName,vRefNum,
								gPrefs.creatorSig,kSeqFileType,TRUE) != noErr )
						break;
					if( OpenMacFile(newFName,vRefNum, &output, TRUE) != noErr)
						break;
				}
			}
			
			/* check whether user cancelled or wants to skip an entry */
			if( ret == Cancel ) {
				if(lbSkipButton)
					continue;
				else
					break;
			}
			else {
				RotateWaitCursor();
				
				if(myDialog == NULL) {
					if(gSysVersion >= 0x0700) {
						myHandle = (DialogTHndl)GetResource('DLOG',EXPORT_DLG);
						(**myHandle).procID = movableDBoxProc;
					}
					CenterDA('DLOG',EXPORT_DLG,50);
					myDialog = GetNewDialog(EXPORT_DLG,NULL,(WindowPtr) -1);
					DisableMenuBar();
				}
				
				ParamText(ename,"\p","\p","\p");
				ShowWindow(myDialog);
				DrawDialog(myDialog);
				if(UserBreak(myDialog))
					bUserBreak = TRUE;
				else {
					/* copy entry to disk */
					if( !Export1(hlHdl,i,output,(**resHdl).dbcode,
									newFName,inpBuf,outBuf,myDialog) )
						break;
									
					++found;
					if( found > (**resHdl).nsel )
						break;
						
					/* close file if necessary */
					if( lbSaveAllButton == FALSE ) {
						FSClose(output);
						output = 0;
					}
					
					if(lbSaveAllButton == FALSE) {
						DisposDialog(myDialog);
						myDialog = NULL;
					}
				}
			}
		}
	}
	
	if(myDialog) DisposDialog(myDialog);
	if (output) FSClose(output);
	FlushVol(NULL,vRefNum);
	HSetState((Handle)hlHdl,oldState);
	DisposPtr((Ptr)inpBuf);
	DisposPtr((Ptr)outBuf);
	EnableMenuBar();
	InitCursor();
	
	/* if we were switched to background we notify user of completion */
	if (gInBackground) {
		bUserNotified = TRUE;
		NotifyUser(&myNotification);
	}
	else
		bUserNotified = FALSE;
		
	/* wait till we're activated */
	WaitUntilForeground();
	
	/* remove notification */
	if(bUserNotified == TRUE)
		RemoveNotification(&myNotification);
}

/**************************************
*	Look up entries in entryname index for export, and call Export2 to actually
*	do the job
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean Export1(HitlistHdl hlHdl, short pos,short output,short dbcode,
						StringPtr outFName, char *inpBuf,char *outBuf,DialogPtr myDialog)
{
	OSErr		err;
	short		input;
	long		size,count;
	u_long	rec;
	EnameRec	enameRec;
	Str255	indexFName,divFName;
	Boolean	ret;
	
	/* Open entryname index  */
	pstrcpy(indexFName,gFileList.enameIdxFName);
	if( OpenMacFileReadOnly(indexFName,gDBInfo[dbcode].InxWDRefNum,&input, TRUE) != noErr )
		return(FALSE);
	
	/* goto appropriate position */
	size = sizeof(EnameRec);
	rec = (*hlHdl)[pos].ename_rec;
	
	if( (err=SetFPos(input,fsFromStart,rec*size + sizeof(Header))) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
					PtoCstr(indexFName),err );
		FSClose(input);
		return(ErrorMsg(0));
	}

	/* read record */
	count=size;
	if( ReadMacFile(input,&count,&enameRec,indexFName,TRUE) ) {
		FSClose(input);
		return(FALSE);
	}
	else
		FSClose(input);
	
	/* convert values */
	ConvertLong(&enameRec.annotation_offset);
	ConvertLong(&enameRec.sequence_offset);
	ConvertShort(&enameRec.div_code);
	
	/* Store data file name */	
	pstrcpy(divFName,
		(StringPtr)(*gDBInfo[dbcode].gDivNames + (enameRec.div_code)*(DIVNAMELEN+1)));
	
	/* Open sequence data file */
	if(OpenMacFileReadOnly(divFName,gDBInfo[dbcode].SeqWDRefNum,&input,TRUE) != noErr )
		return(FALSE);
	
	ret=Export2(input,enameRec.annotation_offset,enameRec.sequence_offset,dbcode,
					divFName,inpBuf,outBuf,output,outFName,myDialog);
	FSClose(input);
	
	if(!ret)
		return(FALSE);
	else {
		count=1;
		WriteMacFile(output,&count,"\r",outFName, TRUE);
		return(TRUE);
	}
}


/**************************************
*	Read a sequence from database and write it to a file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

Boolean Export2(short input,long ann_offset,long seq_offset,short dbcode,StringPtr fName,
				char *inpBuf,char *outBuf,short output,StringPtr outFName,
				DialogPtr myDialog)
{
	OSErr		err;
	Boolean	bDone;
	long		count;
	short		i;
	char		ename[ENTRYNAMELEN+1];
	char		*pos;
	long		offset;
	char		c;
	
#define _MAXDESCLEN	60
	char description[_MAXDESCLEN+1];
	
	/* Go to entry in database */
	offset = (gPrefs.format == STADEN_FORMAT) ? seq_offset : ann_offset;
	if( (err=SetFPos(input,fsFromStart,offset)) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),PtoCstr(fName),err );
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}

	if(gPrefs.format == PIR_FORMAT || gPrefs.format == FASTA_FORMAT) {
		RotateWaitCursor();	/* we're busy */
		
		/* read first block of data */
		count=FILEBUF;
		if( (err=ReadMacFile(input,&count,inpBuf,fName,TRUE)) != noErr && err != eofErr )
			return(FALSE);
		if (!count)
			return(TRUE);	/* or false ?! */
	
		strncpy(ename,inpBuf+5,ENTRYNAMELEN);
		ename[ENTRYNAMELEN]=EOS;
		rtrim(ename);
	
		if ( (pos = strstr(inpBuf,"\nDE   ")) != NULL) {
			for(i=0,pos += 6; *pos != '\r' && i < _MAXDESCLEN; ++i,++pos) 
				description[i] = *pos;
			description[i] = EOS;
		}
		else *description = EOS;
		
		/* skip to sequence */
		RotateWaitCursor();	/* we're busy */
		if( (err=SetFPos(input,fsFromStart,seq_offset)) != noErr) {
			sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),PtoCstr(fName),err );
			CtoPstr((char *)fName);
			return(ErrorMsg(0));
		}
		
		if(gPrefs.format == PIR_FORMAT) {
			c = (dbcode == DB_SWISS) ? 'P' : 'D';
			sprintf(outBuf,">%c1;%s\r%s\r",c,ename,description);
		}
		else if (gPrefs.format == FASTA_FORMAT) {
			sprintf(outBuf,">%s - %s\r",ename,description);
		}
		
		count=strlen(outBuf);
		if( WriteMacFile(output,&count,outBuf,outFName, TRUE) )
			return(FALSE);
	}
	
	bDone=FALSE;
	while( !bDone ) {
		RotateWaitCursor();	/* we're busy */
		if(UserBreak(myDialog))
			return(FALSE);
			
		/* read from database */
		count=FILEBUF;
		if( (err=ReadMacFile(input,&count,inpBuf,fName,TRUE)) != noErr && err != eofErr )
			return(FALSE);
		if (!count)
			return(TRUE);	/* or false ?! */
		
		/* write to file. check success ?! */
		switch( gPrefs.format) {
			case EMBL_FORMAT:
				bDone=WriteEMBLEntry(inpBuf,outBuf,output,outFName,count);
				break;
			case STADEN_FORMAT:
			case PIR_FORMAT:
			case FASTA_FORMAT:
				bDone=WriteSeqOnly(inpBuf,outBuf,output,outFName,count);
				break;
		}
	}
	
	return(TRUE);
}

/**************************************
*	Write EMBL format
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean WriteEMBLEntry(char *inpBuf,char *outBuf,short output,
								StringPtr outFName,long count)
{
	static char	lastChar;
	Boolean		foundEnd;
	register		long i,j;
	
	/* We go through the input buffer and copy every character to output buffer,
		skipping over line feeds. If we find a //, we stop. Because it may happen
		that the first / is the last character of one read operation, and the
		second / is the first character of the next read operation, we check
		for this special case
	*/

	lastChar=EOS;
	for(i=0,j=0,foundEnd=FALSE;i<count;++i) {
		if(inpBuf[i] == '\n')				/* skip over line feeds */
			continue;
				
		outBuf[j++]=inpBuf[i];				/* move character to output buffer */
		if(inpBuf[i] == '/') {				/* check for end of sequence */
			if(i>0) {						/* if it's not the first char in buffer */
				if(inpBuf[i-1] == '/') {	/* check previous character */
					foundEnd=TRUE;			/* found // */
					break;
				}
			}
			else if(lastChar == '(') {		/* if / is the first char in buffer,
															check last character of previous buffer
															contents */
				foundEnd=TRUE;
				break;
			}
		}				
	}
	
	count=j;
	if( WriteMacFile(output,&count,outBuf,outFName,TRUE) ) {
		lastChar = EOS;
		return(TRUE);
	}
		
	lastChar=inpBuf[j];
		
	if(foundEnd)
		lastChar=EOS;
		
	return(foundEnd);
}

/**************************************
*	Write sequence only to a file
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

static Boolean WriteSeqOnly(char *inpBuf,char *outBuf,short output,
								StringPtr outFName,long count)
{
	Boolean			foundEnd;
	register long	i,j;
	
	/* We go through the input buffer and copy every character to output buffer,
		skipping over line feeds. If we find a //, we stop. We don't copy the // */

	for(i=0,j=0,foundEnd=FALSE;i<count;++i) {
		if(inpBuf[i] == '\n' || inpBuf[i] == ' ')	/* skip over line feeds and blanks */
			continue;
				
		if(inpBuf[i] == '/') {			/* check for end of sequence */
			/* add * for PIR format */
			if (gPrefs.format == PIR_FORMAT)
				outBuf[j++] = '*';
				
			foundEnd=TRUE;			/* found // */
			break;
		}

		outBuf[j++]=inpBuf[i];				/* move character to output buffer */
	}
		
	count=j;
	if( WriteMacFile(output,&count,outBuf,outFName,TRUE) )
		return(TRUE);
	return(foundEnd);				
}

/**************************************
*	Extracts entryname from short description
*	Return value:	none
*	Side-effect:	Pascal file name in "name"
*/

void ExtractEname(CString80Hdl bufHdl, short pos, StringPtr ename)
{
	BlockMove((*bufHdl)[pos]+1,ename,ENTRYNAMELEN);
	ename[ENTRYNAMELEN]=EOS;
	rtrim((char *)ename);
	CtoPstr((char *)ename);
}
	

/**************************************
*	Construct default file name from entryname
*	Return value:	none
*	Side-effect:	Pascal file name in "filename"
*/

void BuildFName(StringPtr filename, StringPtr ename, short format)
{
	Str255 extension;
	
	pstrcpy(filename,ename);
	GetIndString(extension,EXTENSIONS_STR,gPrefs.format);
	pstrcat(filename,extension);
}

/**************************************
*	Ask user for filename
*	Return value:	Code of selected button
*	Side-effect:	selected file name in "newFName"
*/

static short GetExportFilename(StringPtr oldFName,short *vRefNum,StringPtr newFName)
{
	SFReply	reply;
	Point		where;
	Str255	prompt;
	
	/* Deactivate current front window. SFPut/GetFile sends an activate event
		when it quits but not when it opens ! */
	HandleActivates(FrontWindow(),0);
	GetIndString(prompt,OTHERS,PROMPTSTR);
	CenterSFDlg(SAVEENTRY_DLG,&where);
	lbSkipButton = FALSE;
	SFPPutFile(where, prompt, oldFName, (ProcPtr)myExportDlgHook,
		&reply,SAVEENTRY_DLG,NULL);
	
	if (reply.good) {
		pstrcpy(newFName,reply.fName);
		*vRefNum = reply.vRefNum;
		return(OK);
	}
	else return(Cancel);
}

/**************************************
*	Dialog hook routine to initialise file dialog and to handle skip button
*	Return value:	theItem
*/

static pascal short myExportDlgHook(short theItem, DialogPtr myDialog)
{
	short		kind;
	Handle	h;
	Rect		r;
	Str255	str;
	
	switch(theItem) {
		/* initialisation of dialog (called first time before dialog is displayed) */
		case -1:
			GetDItem(myDialog,SAVEALL_BUTTON,&kind,&h,&r);
			if(gPrefs.format != STADEN_FORMAT)
				HiliteControl((ControlHandle)h,ACTIVE);
			else
				HiliteControl((ControlHandle)h,INACTIVE);
				
			GetIndString(str,FORMAT_STRINGS,gPrefs.format);
			SetDlgText(myDialog, FORMAT_STR, str);
			break;
			
		case SKIP_BUTTON:
			lbSkipButton = TRUE;
			theItem=putCancel;
			break;
			
		case SAVEALL_BUTTON:
			lbSaveAllButton = TRUE;
			theItem = OK;
			break;
	}
	
	return(theItem);
}

/**************************************
*	EventHandling during movable modal dialog
*	Return value: TRUE if user cancelled
*/

static Boolean UserBreak(DialogPtr myDialog)
{
	EventRecord theEvent;
	DialogPtr	whichDialog;
	short			itemHit;
	WindowPtr	whichWindow;
	static long	ticks = 0;
	
	if(gInBackground || TickCount() - ticks > 30) {
		ticks = TickCount();
		if(WaitNextEvent(everyEvent,&theEvent,
								gInBackground ? 25 : 0,NULL)) {
			if(theEvent.what == osEvt) /* see "Zen and..." (bug in IsDialogEvent) */
				DoEvent(theEvent);
			else if(theEvent.what == keyDown || theEvent.what == autoKey) {
				if ( CmdPeriod(&theEvent)  ||
					( (theEvent.message & keyCodeMask) >> 8 == ESCAPE))
					return(TRUE);
			}
			else {
				if(IsDialogEvent(&theEvent)) {
					if(DialogSelect(&theEvent,&whichDialog,&itemHit))
						if(whichDialog == myDialog && itemHit == OK)
							return(TRUE);
				}
				else {
					switch(theEvent.what) {
						case updateEvt:
						case activateEvt:
							DoEvent(theEvent);
							break;
						case mouseDown:
							switch (FindWindow( theEvent.where, &whichWindow )) {
								case inSysWindow:
									SystemClick( &theEvent, whichWindow );
									break;
								case inDrag:
									if(whichWindow == myDialog)
										DragWindow(whichWindow,theEvent.where,&gDragRect);
									break;
									case inMenuBar:
								MenuSelect(theEvent.where);
									HiliteMenu(0);
									break;
								default:
									SysBeep(10);
									break;
							}
							break;
						default:
							break;
					}
				}
			}
		}
	}
	
	return(FALSE);
}

/**************************************
*	Disable menu bar before drawing movable modal dialog
*/

void DisableMenuBar()
{
	register short i;
	
	HiliteMenu(0);	
	for(i=0;i <= WINDOWS;DisableItem(gMenu[i++],0));
	DrawMenuBar();
}

/**************************************
*	reenable menu bar after "movable modal dialog"
*/

void EnableMenuBar()
{
	register short i;
	
	for(i=0;i <= WINDOWS;EnableItem(gMenu[i++],0));
	DrawMenuBar();
}

/**************************************
*	Simply looping until we are switched to foreground
*/

static void WaitUntilForeground()
{
	EventRecord theEvent;
	
	while(gInBackground) {
		if(WaitNextEvent(everyEvent,&theEvent,0x7FFFFFFF,NULL)) {
			switch (theEvent.what) {
				case updateEvt:
				case activateEvt:
				case osEvt:
					DoEvent(theEvent);
					break;
				default:
					break;
			}
		}
	}
}

/**************************************
*	Install notification record in system queue
*/

static void NotifyUser(NMRec *myNMPtr)
{
	static Handle myResHdl;		/* we use static because the Handle must be valid
											when the notification actually takes place ! */	
	
	myResHdl = GetResource('SICN',128);
	
	myNMPtr->qType = nmType;
	myNMPtr->nmMark = 1;
	myNMPtr->nmIcon = myResHdl;
	myNMPtr->nmSound = (Handle)-1;
	myNMPtr->nmStr = NULL;
	myNMPtr->nmResp = (ProcPtr)NotifyResponse;
	
	NMInstall(myNMPtr);
}

/**************************************
*	Notification response procedure. We don't do anything
*/

static pascal void NotifyResponse(NMRec *myNMPtr)
{
}

/**************************************
*	Removal of notification record from system queue
*/

static void RemoveNotification(NMRec *myNMPtr)
{
	NMRemove(myNMPtr);
}

