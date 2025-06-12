/*
*********************************************************************
*	
*	Util.c
*	Mixture of general purpose routines
*
*	Rainer Fuchs
*  EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*		
**********************************************************************
*
*/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <Aliases.h>
#include <Script.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "util.h"
#include "pstr.h"
#include "events.h"
#include "window.h"

/*
******************************* Global variables *********************
*/

extern CursHandle	gCursor[8];
extern Boolean gHasAliasMgr;
extern Boolean gHasFSSpec;
extern short gAppResRef;							/* Application rsrc file ref num */
extern Boolean gInBackground;

char gError[256];
Boolean bErrorOccurred = FALSE;

static short lgCurCursor=0;
static long	gCursorTime;


/****************************** Byte manipulation utilities *******************
*	ISO format is least significant byte first, Mac format is reversed
*	order.
*/

/**************************************
*	Converts ISO 4-byte integer "val" to Mac long integer.
*	Return value:	the value of "val" in Mac format
*	Side-effect:	"val" becomes converted to Mac format
*/

u_long ConvertLong(u_long *val)
{
	register u_byte temp;
	
	temp=((u_byte *)val)[0];
	((u_byte *)val)[0]=((u_byte *)val)[3];
	((u_byte *)val)[3]=temp;
	
	temp=((u_byte *)val)[1];
	((u_byte *)val)[1]=((u_byte *)val)[2];
	((u_byte *)val)[2]=temp;
	
	return(*val);
}


/**************************************
*	Converts ISO 2-byte integer to Mac integer. in place and also
*	Return value:	the value of "val" in Mac format
*	Side-effect: 	"val" becomes converted to Mac format
*/

u_short ConvertShort(u_short *val)
{
	register u_byte temp;
	
	temp=((u_byte *)val)[0];
	*val <<= 8;
	((u_byte *)val)[1]=temp;
	
	return(*val);
}


/******************************* String manipulation utilities *******************/


/**************************************
*	Converts a string "str" to uppercase.
*	Return value:	Pointer to the converted string
*	Side-effect: 	the converted string in "str"
*/

char * str2upper(char *str)
{
	register char *s = str;
	
	if( !str )
		return(NULL);
		
	while( *s = toupper(*s) )
		++s;
		
	return( str );
}


/**************************************
*	Removes trailing blanks from a string "str".
*	Return value:	Pointer to the converted string
*	Side-effect: 	the converted string in "str"
*/

char * rtrim(char *str)
{
	register short p;
	
	if (!str || *str == EOS)
		return(str);
		
	p=strlen(str)-1;
	while ( p >= 0 && isspace(str[p]) )
		--p;
		
	str[p + 1] = EOS;
	
	return( str );
}

/**************************************
*	Removes leading blanks from a string "str".
*	Return value:	Pointer to the converted string
*	Side-effect: 	the converted string in "str"
*/

char * ltrim(char *str)
{
	register char *p;
	
	if (!str || *str == EOS)
		return(str);
		
	p = str;
	while(*p && isspace(*p))
		++p;
		
	if(p != str)
		strcpy(str,p);
	
	return( str );
}

/**************************************
*	Pads "str" to "len" characters with "c". "str" must be long enough to hold "len"
*  characters!!!
*	Return value:	Pointer to the converted string
*	Side-effect: 	the converted string in "str"
*/

char * rpad(char *str,char c,short len)
{
	register short i;
	
	for(i=strlen(str); i < len; ++i)
			str[i] = c;
	str[len] = EOS;

	return(str);
}


/**************************************
*	Removes all blanks from "str".
*	Return value:	Pointer to converted string
*	Side-effect:	 the converted string in "str"
*/

char * compress(char *str)
{
	register char *s,*t=str;
	
	for(s=str;*s;++s)
		if( !isspace(*s) )
			*str++ = *s;

	*str=EOS;
	return(t);
}

/**************************************
*	Checks for EMBL-style line type identifiers
*	Return value:	TRUE, if line starts with "id",
						FALSE, if not
*/

Boolean linetype(char *line, char *id)
{
	return( strncmp(line,id,strlen(id)) == 0);
}


/************************* File handling utilities *********************************/


/**************************************
*	This routine is used to center an SFPutFile/SFGetFile dialog
*	Return value: 
*/

void CenterSFDlg(short which, Point *where)
{
	DialogTHndl	myHandle;
	short			width, height;
	short			mBarHeight=GetMBarHeight();			/* subtract menu bar height */
	
	/* get the resource and calculate width and height */	
	myHandle=(DialogTHndl)GetResource('DLOG',which);
	if(myHandle == NULL) {
		/* set default values acc. to IM-I */
		if(which = putDlgID) {
			height = 184;  width = 304;
		}
		else {
			height = 200; width = 384;
		}
	}
	
	else {
		height=(**myHandle).boundsRect.bottom-(**myHandle).boundsRect.top;
		width=(**myHandle).boundsRect.right-(**myHandle).boundsRect.left;
	}
	
	where->v=((screenBits.bounds.bottom-screenBits.bounds.top)-height)/2 + mBarHeight;
	where->h=((screenBits.bounds.right-screenBits.bounds.left)-width)/2;
}

/**************************************
*	Delete a Mac file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*/

OSErr DeleteMacFile(StringPtr fName, short vRefNum)
{
	OSErr err;
	
	err=MyResolveAlias(fName,&vRefNum);
	err=FSDelete(fName,vRefNum);
	
	return(err);
}


/**************************************
*	Create a new Mac file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*/

OSErr CreateMacFile(StringPtr fName, short vRefNum, OSType creator, OSType type,
						  Boolean bShowErrMsg)
{
	OSErr err;
	
	/* Delete any old version */
	DeleteMacFile(fName,vRefNum);
	
	if( (err=Create(fName,vRefNum,creator,type)) != noErr && bShowErrMsg) {
		sprintf(gError,LoadErrorStr(ERR_CREATEFILE,FALSE),PtoCstr(fName),err);
		CtoPstr((char *)fName);
	   ErrorMsg(0);
	}
	return(err);
}


/**************************************
*	Open a Mac file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*	Side-effect:	file id in "output"
*/

OSErr OpenMacFile(StringPtr fName, short vRefNum, short *output, Boolean bShowErrMsg)
{
	OSErr err;
	
	if( (err=MyResolveAlias(fName,&vRefNum)) == noErr ) {
		err=FSOpen(fName,vRefNum,output);
	}
		
	if( err != noErr && bShowErrMsg) {
		sprintf(gError,LoadErrorStr(ERR_OPENFILE,FALSE),PtoCstr(fName),err);
   	CtoPstr((char *)fName);
	  	ErrorMsg(0);
	}

	return(err);
}

/**************************************
*	Open a Mac file in READONLY mode.
*	This is useful for reading in the index file so that they canbe moved to a shared
*	disk.
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*	Side-effect:	file id in "output"
*/

OSErr OpenMacFileReadOnly(StringPtr fName, short vRefNum, short *output,
		Boolean bShowErrMsg)
{
	OSErr err;
	ParamBlockRec myPB;
	
	if( (err=MyResolveAlias(fName,&vRefNum)) == noErr ) {
		myPB.ioParam.ioNamePtr = fName;
		myPB.ioParam.ioVRefNum = vRefNum;
		myPB.ioParam.ioVersNum = 0;
		myPB.ioParam.ioPermssn = fsRdPerm;
		myPB.ioParam.ioMisc = NULL;
		
		err=PBOpen(&myPB,FALSE);
		*output = myPB.ioParam.ioRefNum;
	}
		
	if( err != noErr && bShowErrMsg) {
		sprintf(gError,LoadErrorStr(ERR_OPENFILE,FALSE),PtoCstr(fName),err);
   	CtoPstr((char *)fName);
	  	ErrorMsg(0);
	}

	return(err);
}


/**************************************
*	Write to a Mac file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*	Side-effect: 	number of bytes written successfully in "count"
*/

OSErr WriteMacFile(short output, long *count, void *what, StringPtr fName,
						Boolean bShowErrMsg)
{
	OSErr err;
	
	if( (err=FSWrite(output, count, what)) != noErr  && bShowErrMsg) {
		sprintf(gError,LoadErrorStr(ERR_WRITEFILE,FALSE),PtoCstr(fName),err);
      CtoPstr((char *)fName);
   	ErrorMsg(0);
 	}
 	
 	return(err);
}
 

/**************************************
*	Read from a Mac file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*	Side-effect:	number of bytes read successfully in "count"
*/

OSErr ReadMacFile(short input, long *count, void *what, StringPtr fName,
						Boolean bShowErrMsg)
{
	OSErr err;
	
	if( (err=FSRead(input, count, what)) != noErr && bShowErrMsg) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),PtoCstr(fName),err);
      CtoPstr((char *)fName);
   	ErrorMsg(0);
 	}
 	
 	return(err);
}


/**************************************
*	Converts a System 7 FSSpec to System 6 HFS specification
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*/

OSErr FSSpecToHFS(FSSpec *theFSS,short *wdRefNum,StringPtr fName)
{
	pstrcpy(fName,theFSS->name);
	return( OpenWD(theFSS->vRefNum,theFSS->parID,kApplSignature,wdRefNum) );
}
				

/**************************************
*	Ask user for new file name
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side-effect:	new filename in newFName
*/

Boolean GetNewFilename(Str255 oldFName,short *newVRefNum,Str255 newFName)
{
	SFReply	reply;
	Point		where;
	Str255	prompt;
	
	/* Deactivate current front window. SFPut/GetFile sends an activate event
		when it quits but not when it opens ! */
	HandleActivates(FrontWindow(),0);
	GetIndString(prompt,OTHERS,PROMPTSTR);
	where.h=((screenBits.bounds.right-screenBits.bounds.left)-304)/2;
	where.v=((screenBits.bounds.bottom-screenBits.bounds.top)-184)/2+20;
	SFPutFile(where, prompt, oldFName, NULL,&reply);
	
	if (reply.good) {
		pstrcpy(newFName,reply.fName);
		*newVRefNum = reply.vRefNum;
		return(TRUE);
	}
	else return(FALSE);
}

/**************************************
*	Get real vRefNum from volume name (without colon!)
*/

Boolean GetVRefNumFromName(StringPtr vName, short *vRefNum)
{
	ParamBlockRec myPB;
	Str255 ioName;
	OSErr ret;
	
	pstrcpy(ioName,vName);
	pstrcat(ioName,"\p:");
	
	myPB.volumeParam.ioNamePtr = ioName;
	myPB.volumeParam.ioVRefNum = 0x8000;
	myPB.volumeParam.ioVolIndex = -1;
	
	if((ret = PBGetVInfo(&myPB,FALSE)) == noErr)
		*vRefNum = myPB.volumeParam.ioVRefNum;
		
	return(ret == noErr);
}

/**************************************
*	Get volume name from real vRefNum
*/

Boolean GetNameFromVRefNum(StringPtr vName, short vRefNum)
{
	ParamBlockRec myPB;
	Str255 ioName;
	OSErr ret;
	
	*vName = 0;
	
	myPB.volumeParam.ioNamePtr = vName;
	myPB.volumeParam.ioVRefNum = vRefNum;
	myPB.volumeParam.ioVolIndex = 0;
	
	return( (ret = PBGetVInfo(&myPB,FALSE)) == noErr);
}


/**************************************
*	Resolve aliases if Alias Manager and new File Manager calls are available.
*	Otherwise simply return fName and wdRefNum unchanged.
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*/

OSErr MyResolveAlias(StringPtr fName, short *wdRefNum)
{
	OSErr err;
	FSSpec theFSS;
	Boolean targetIsFolder,wasAliased;
	
	err=noErr;
	if(gHasAliasMgr && gHasFSSpec) {
		/* convert HFS to FSSpec */
		err=FSMakeFSSpec(*wdRefNum,0L,fName,&theFSS);
		/* resolve alias */
		if(err == noErr) {
			err=ResolveAliasFile(&theFSS,TRUE,&targetIsFolder,&wasAliased);
			if(targetIsFolder)
				err=paramErr;	/* cannot open a folder */
		}
		/* convert FSSpec back to HFS */
		if(err == noErr)
			err=FSSpecToHFS(&theFSS,wdRefNum,fName);
	}
	return(err);
}


/********************************** General utilities *****************************/

/**************************************
*	Changes GrafPort
*	Return value: previous GrafPort
*/

GrafPtr ChangePort(GrafPtr newPort)
{
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(newPort);
	
	return(oldPort);
}

/**************************************
*	Moves a relocatable block high up in memory (to prevent memory
*	fragmentation) and locks it.
*	Return value:	Old status of handle flags
*/

SignedByte LockHandleHigh(Handle theHandle)
{
	SignedByte	hstate;

	hstate = HGetState(theHandle);
	HLockHi(theHandle);
	return(hstate);
}

/**************************************
*	Locks a relocatable block.
*	Return value:	Old status of handle flags
*/

SignedByte MyHLock(Handle theHandle)
{
	SignedByte	hstate;

	hstate = HGetState(theHandle);
	HLock(theHandle);
	return(hstate);
}

#define TICK_PERIOD 120

/*************************************
*	Draw first of our wait cursors
*/

void StartWaitCursor()
{
	SetCursor(*gCursor[lgCurCursor=0]);
	gCursorTime = TickCount()+TICK_PERIOD;

}


/*************************************
*	Rotate wait cursor. Every TICK_PERIOD/60 seconds we change it. This way we
*	are somewhat independent of machine speed.
*/

void RotateWaitCursor()
{
	if(TickCount() >= gCursorTime) {
		gCursorTime += TICK_PERIOD;				/* approx. every two seconds */
		++lgCurCursor;
		if(lgCurCursor == 8) lgCurCursor = 0;
		SetCursor(*gCursor[lgCurCursor]);
	}
}


/***************************** Dialog handling utilities ****************************/


/**************************************
*	Standard filter proc for dialogs.
*	Notice that we assume that default button is always item #1!
*	Return value:	TRUE, if we handled the event
*						FALSE, if not
*/

pascal Boolean myDialogFilter(DialogPtr myDialog,EventRecord *theEvent,
										short *itemHit)
{
	char charCode;
      
   /* first check for keydown events */
   if ((theEvent->what == keyDown) || (theEvent->what == autoKey)) {
      charCode=(char)(theEvent->message & charCodeMask);
      
      /* ESCape pressed (extended keyboard) or Cmd-Period ? */
   	if( (theEvent->message & keyCodeMask)>>8 == ESCAPE || CmdPeriod(theEvent) ) {
   		/* Fake click on Cancel button */
			FakeClick(myDialog,Cancel);	
			*itemHit=Cancel;
      	return(TRUE);
		}
   	
   	if(theEvent->modifiers & cmdKey) {
   		switch(charCode) {
          /* or keyboard equivalents to Edit menu items ?
          	(we support clipboard usage in dialogs!)			*/
            case 'v':
            case 'V':
            	TEFromScrap();
            	DlgPaste(myDialog);
      			return(TRUE);
            break;
            	
            case 'x':
            case 'X':
            	DlgCut(myDialog);
            	ZeroScrap();
            	TEToScrap();
      			return(TRUE);
            break;
 
            case 'c':
            case 'C':
            	DlgCopy(myDialog);
            	ZeroScrap();
            	TEToScrap();
      			return(TRUE);
            break;
            
            default:return(FALSE);
   		} /* end switch */
   	} /* end if */
   	
   	/* Return or Enter ? */
      if(charCode == 3 || charCode == 13) {
			FakeClick(myDialog,OK);	
         *itemHit=OK;
         return(TRUE);
		}
			
   	return(FALSE);
   } /* end if */
   return(FALSE);
}


/**************************************
*	Mark default button by drawing a thick border.
*/

pascal void DrawOKBoxRect(WindowPtr myWindow,short itemNo)
{
	short		kind;
	Handle	h;
	Rect		r;
	PenState	oldPenState;
   
	GetPenState(&oldPenState);					/* save old pen state */
	PenSize(3,3);
   
	GetDItem(myWindow,itemNo,&kind,&h,&r);		/* get rect of OK button */
	InsetRect(&r,-4,-4);							/* and draw border */
	FrameRoundRect(&r,16,16);
	SetPenState(&oldPenState);					/* restore pen state */
}

/**************************************
*	Draw a thick border around a user item.
*/

pascal void DrawFrame(WindowPtr myWindow,short itemNo)
{
	short		kind;
	Handle	h;
	Rect		r;
	PenState	oldPenState;
   
	GetPenState(&oldPenState);					/* save old pen state */
	PenSize(2,2);
   
	GetDItem(myWindow,itemNo,&kind,&h,&r);	/* get rect of item */
	InsetRect(&r,-2,-2);								/* and draw border */
	FrameRect(&r);
	SetPenState(&oldPenState);					/* restore pen state */
}

/**************************************
*	Set radio button or check box to "status"
*/

void SetRadioButton(DialogPtr myDialog, short item, short status)
{
	short		kind;
	Handle	h;
	Rect		r;
	
	GetDItem(myDialog,item,&kind,&h,&r);
	SetCtlValue((ControlHandle)h,status);
}

/**************************************
*	Get status of radio button or check box
*/

short GetRadioButton(DialogPtr myDialog, short item)
{
	short		kind;
	Handle	h;
	Rect		r;
	
	GetDItem(myDialog,item,&kind,&h,&r);
	return(GetCtlValue((ControlHandle)h));
}

/**************************************
*	Toggle status of radio button or check box
*/

void ToggleRadioButton(DialogPtr myDialog, short item)
{
	SetRadioButton(myDialog,item,!GetRadioButton(myDialog,item));
}

/**************************************
*	Select one from a group of radio buttons. Buttons must be numbered
*	consecutively in the rsrc file.
*/

void SelRadioButton(DialogPtr myDialog, short firstItem, short n,
							short whichItem)
{
	register short i;
	
	for(i=0; i<n; ++i)
		SetRadioButton(myDialog,firstItem+i,BTNOFF);
	
	SetRadioButton(myDialog,whichItem,BTNON);
}

/**************************************
*	Set the text of a dialog item
*/

void SetDlgText(DialogPtr myDialog, short item, StringPtr text)
{
	short		kind;
	Handle	h;
	Rect		r;
	
	GetDItem(myDialog,item,&kind,&h,&r);
	SetIText(h,text);
}

/**************************************
*	Get the text of a dialog item
*/

void GetDlgText(DialogPtr myDialog, short item, StringPtr text)
{
	short		kind;
	Handle	h;
	Rect		r;
	
	GetDItem(myDialog,item,&kind,&h,&r);
	GetIText(h,text);
}

/**************************************
*	Install a user item. If useRect != -1 then this procedure will set the rect of item
*	to the rect of the useRect control.
*/

void InstallUserItem(DialogPtr myDialog, short item, short useRect,
	pascal void (*userRoutine)(WindowPtr myWindow,short itemNo))
{
	short		kind;
	Handle	h;
	Rect		r;
	
	GetDItem(myDialog,item,&kind,&h,&r);
	if(useRect != -1)
		GetDItem(myDialog,useRect,&kind,&h,&r);
		
	SetDItem(myDialog,item,userItem,(Handle)userRoutine,&r);
}

/**************************************
*	Hilite a clicked button to show its activation
*/

void FakeClick(DialogPtr myDialog, short item)
{
	short				kind;
	ControlHandle	h;
	Rect				r;
	long				ignore;

	GetDItem(myDialog,item,&kind,(Handle *)&h,&r);
	HiliteControl(h,1);
	Delay(8,&ignore);
	HiliteControl(h,0);
}

/**************************************
*	Check for Cmd-Period according to TN 263
*/

Boolean CmdPeriod(EventRecord *theEvent)
{
#define kMaskModifiers 0xFE00			/* we need the modifiers without the cmd key for KeyTrans */
#define kMaskVirtualKey 0x0000FF00	/* get virtual key from event message for KeyTrans */
#define kMaskASCII1 0x00FF0000
#define kMaskASCII2 0x000000FF
#define kPeriod 0x2E
#define kShiftWord 8						/* shift the virt. key  mask it into the keyCode for
													KeyTrans */
													
	Boolean	fTimeToQuit;
	short		keyCode;
	long 		virtualKey,keyInfo,lowChar,highChar,state,keyCId;
	Handle	hKCHR;
	Ptr		KCHRPtr;
	
	fTimeToQuit = FALSE;
	
	if(theEvent->what == keyDown || theEvent->what == autoKey) {
		if (theEvent->modifiers & cmdKey ) {
			virtualKey = (theEvent->message & kMaskVirtualKey) >> kShiftWord;
			/* And out the the cmd key and Or in the virtual key */
			keyCode = (theEvent->modifiers & kMaskModifiers) | virtualKey;
			state = 0;
			
			hKCHR = NULL;
			KCHRPtr = (Ptr)GetEnvirons(smKCHRCache); /* Pre-System 7 will return NULL */
			
			if(KCHRPtr == NULL) {	/* System < 7 */
				keyCId = GetScript(GetEnvirons(smKeyScript),smScriptKeys);
				hKCHR = GetResource('KCHR',keyCId);
				KCHRPtr = *hKCHR;
			}
			
			if(KCHRPtr != NULL) {
				/* Don't bother locking since KeyTrans will never move memory */
				keyInfo = KeyTrans(KCHRPtr, keyCode, &state);
				if(hKCHR)
					ReleaseResource(hKCHR);
			}
			else
				keyInfo = theEvent->message;
				
			lowChar = keyInfo & kMaskASCII2;
			highChar = (keyInfo & kMaskASCII1) >>16;
			if(lowChar == kPeriod || highChar == kPeriod)
				fTimeToQuit = TRUE;
				
		}
	}
	
	return(fTimeToQuit);
}


/************************** Error messaging ***************************************/


/**************************************
*	Get a string from resource file from STR# Error messages.
*	Depending on value of 'pas',  a pascal (pas==TRUE) or a C string (pass==FALSE)
*	is returned.
*	Return value:	Pascal or C string
*/

char * LoadErrorStr(short index, Boolean pas)
{
	static Str255 theString;
		
	*theString = EOS;
	GetIndString(theString,ERROR_MSGS,index);
	if(pas) return((char *)theString);
	else return(PtoCstr(theString));
}


/**************************************
*	Notify user by displaying an error alert
*
*	If 'index' is not 0, then a message is read from resource file.
*	If 'index' is 0, then gError is used instead. gError contains an error msg in C format.
*	We check whether we're in the background. If no, we simply display the error msg,
*	otherwise we only set the bErrorOccurred flag.
*	Return value:	FALSE
*/

Boolean ErrorMsg(short index)
{
	char *str;
	
	InitCursor();										/* back to arrow cursor */
	if(index) {
		/* get error message from resouce */
		strcpy(gError,LoadErrorStr(index,FALSE));
	}
   
   if(gInBackground) {
   	bErrorOccurred = TRUE;
   }
   else {
		ParamText(CtoPstr(gError),"\p","\p","\p");
		CenterDA('ALRT',ERROR_ALRT,66);				/* Calculate position on screen */
		StopAlert(ERROR_ALRT,NULL);
   	*gError = EOS;
   	bErrorOccurred = FALSE;
	}
	
	return(FALSE);										/* always returns false */
}


/**************************************
*	Notify user by displaying an error alert, then exit to shell
*/

void FatalErrorMsg(short index)
{
	ErrorMsg(index);
	ExitToShell();
}

/**************************************
*	Follow Apple guidelines for inverting
*/

void DoInvertRect(Rect *r)
{
#define _HiliteMode 0x938

	BitClr((Ptr)_HiliteMode,pHiliteBit);
	InvertRect(r);
}

/**************************************
*	Read name string resource
*/

StringHandle GetNameStringRsrc()
{
	StringHandle	strHdl;
	short				curResFile;

	/* store refnum of current resource file */
	curResFile = CurResFile();

	UseResFile(gAppResRef);	
	strHdl = GetString(kNameStringRsrcTmpl);
	DetachResource((Handle)strHdl);
	UseResFile(curResFile);
	
	return(strHdl);
}

/**************************************
*	Write name string resource to resource file
*/

OSErr WriteNameStringRsrc(short refNum)
{
	Handle			newNameHdl,oldNameHdl;
	OSErr				err;
	short				curResFile;
	
	err = noErr;
	
	if( (newNameHdl = (Handle)GetNameStringRsrc()) != NULL) {
		curResFile = CurResFile();
		UseResFile(refNum);
	
		/* delete any old incarnation of the name string resource */
		oldNameHdl = Get1Resource(kNameStringRsrcType,kNameStringRsrc);
		if(oldNameHdl != NULL) {
			RmveResource(oldNameHdl);
			UpdateResFile(refNum);
			DisposHandle(oldNameHdl);
		}
	
		/* add name string resource */
		AddResource(newNameHdl,
			kNameStringRsrcType,kNameStringRsrc,kNameStringRsrcName);
		
		if( (err=ResError()) == noErr ) {
			WriteResource(newNameHdl);
			err = ResError();
		}
		
		if(err != noErr) {
			DetachResource(newNameHdl);
			DisposHandle(newNameHdl);
		}
			
		UseResFile(curResFile);
	}
	
	return(err);
}