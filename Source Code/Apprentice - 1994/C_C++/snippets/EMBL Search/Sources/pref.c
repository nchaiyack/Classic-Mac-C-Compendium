/*
*********************************************************************
*	
*	pref.c
*	Preferences file
*		
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*	
*	The structure of the Prefs file is simple yet flexible. Each element of gPrefs has
*	got a "tag" to identify it. Written out to the Prefs file is the tag (0-255), two
*	bytes specifying the length of the following data, and the value of the gPrefs
*	element. Using this tag-value approach keeps the Prefs file upward and downward
*	compatible and extendible compared to a simple dump of gPrefs.
**********************************************************************
*	
*/

#include <stdio.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"
#include "tag.h"

typedef struct PrefHeader {
	Byte tag;
	unsigned short dataLen;
} PrefHeader;

/*
******************************* Prototypes ****************************
*/

#include "pref.h"
#include "util.h"
#include "pstr.h"
#include "window.h"

static OSErr ReadPrefs(void);
static void SetPref(Byte tag, Byte *data);
static void InitPrefs(void);
static Boolean Write1Pref(short output, PrefHeader *ph, Byte *data);



/*
********************************* Globals *****************************
*/

extern short gPrefVRefNum;
extern char gError[256];

Prefs gPrefs;
OSType gCreatorSig[6] = {'????','MSWD','MACA','ttxt','????','????'};

/**************************************
*	Read and set preferences
*/

void GetPrefs()
{
	OSErr err;
	
	InitPrefs();
	err=ReadPrefs();
	if(err != noErr) {		/* on error, we reset preferences */
		InitPrefs();
		if( err == fnfErr)	/* and create a new Prefs file if necessary */
			WritePrefs();
	}
}

/**************************************
*	Read Preferences file
*	Return value:	noErr, if successful
*						OS error code, if error occurred
*/

static OSErr ReadPrefs()
{
	OSErr		err;
	short		input;
	unsigned long		count;
	short		wdRefNum;
	Str255	fName;
	PrefHeader ph;
	Byte		data[512];
	
	/* get file name */
	GetIndString(fName,OTHERS,PREFSNAME);
	wdRefNum=gPrefVRefNum;

	/* open file */
	err = OpenMacFileReadOnly(fName,wdRefNum,&input,FALSE);
	if(err != noErr) {
		if(err == fnfErr)
   		ErrorMsg(ERR_NOPREF);
   	else {
			sprintf(gError,LoadErrorStr(ERR_OPENPREF,FALSE),err);
			ErrorMsg(0);
		}
		return(err);
	}
	else {
		while(err == noErr) {	/* continue until we hit EOF */
			/* read header */
			count = sizeof(PrefHeader);
			err=ReadMacFile( input, (long *)&count, &ph,fName, FALSE );
			if(err == eofErr) {	/* hit end of file */
				err = noErr;
				break;
			}
			if(err == noErr) {
				/* read data */
    			if(ph.dataLen >512) ph.dataLen = 512; /* should issue a warning here */
    			count=(unsigned long)ph.dataLen;
    			err=ReadMacFile( input, (long *)&count, &data,fName, FALSE );
    		}
			if( err != noErr ) {
				sprintf(gError,LoadErrorStr(ERR_READPREF,FALSE),err);
				ErrorMsg(0);
			}
			else
				SetPref(ph.tag,data);
		}
		
   	FSClose(input);
   	return(err);
   }
}

/**************************************
*	Set a global preference defined by "tag" to the value given in "data"
*/

static void SetPref(Byte tag, Byte *data)
{
	switch(tag) {
		case T_EMBLINXDIR:
			gPrefs.inxDirSpec[DB_EMBL] = *(DirSpec *)data;
			break;
		case T_SWISSINXDIR:
			gPrefs.inxDirSpec[DB_SWISS] = *(DirSpec *)data;
			break;
		case T_FORMAT:
			gPrefs.format = *(short *)data;
			break;
		case T_CREATOR:
			gPrefs.creator = *(short *)data;
			break;
		case T_CREATORNAME:
			pstrcpy(gPrefs.creatorName,(StringPtr)data);
			break;
		case T_CREATORSIG:
			gPrefs.creatorSig = *(OSType *)data;
			break;
		case T_CONFIRMCHG:
			gPrefs.confirmChg = *(Boolean *)data;
			break;
		case T_STARTQOPEN:
			gPrefs.startQOpen = *(Boolean *)data;
			break;
	}
}

/**************************************
*	Set default values for Preferences
*	Return value:	none
*/

static void InitPrefs()
{
	register short i;
	
	for(i=0;i<DB_NUM;++i) {
		*gPrefs.inxDirSpec[i].volName = EOS;
		gPrefs.inxDirSpec[i].dirID = 0;
	}
	gPrefs.format=EMBL_FORMAT;
	gPrefs.creator=WORD_I;
	*gPrefs.creatorName = EOS;
	gPrefs.creatorSig = gCreatorSig[WORD_I];
	gPrefs.confirmChg = TRUE;
	gPrefs.startQOpen = TRUE;
}

/**************************************
*	Write Preferences file
*/

void WritePrefs()
{
	OSErr		err;
	short		output;
	long		count;
	short		wdRefNum;
	Str255	fName;
	PrefHeader ph;
	Byte		data[512];
	Boolean	ret;
	
	/* get file name */	
	GetIndString(fName,OTHERS,PREFSNAME);
	wdRefNum=gPrefVRefNum;
	
	/* create new version */
	if( (err=CreateMacFile(fName,wdRefNum,kApplSignature,kPrefFileType,FALSE)) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_CREATPREF,FALSE),err);
		ErrorMsg(0);
	}
   else {	/* open file */
   	if( (err=OpenMacFile(fName,wdRefNum,&output,TRUE)) == noErr) {
   		/* write data */
   		
   		ph.tag = T_EMBLINXDIR;
   		ph.dataLen = (unsigned short) sizeof(DirSpec);
   		*(DirSpec *)data = gPrefs.inxDirSpec[DB_EMBL];
   		ret = Write1Pref(output,&ph,data);

			if(ret) {
				ph.tag = T_SWISSINXDIR;
				ph.dataLen = (unsigned short) sizeof(DirSpec);
   			*(DirSpec *)data = gPrefs.inxDirSpec[DB_SWISS];
   			ret = Write1Pref(output,&ph,data);
   		}
   		
			if(ret) {
				ph.tag = T_FORMAT;
				ph.dataLen = (unsigned short) sizeof(short);
   			*(short *)data = gPrefs.format;
   			ret = Write1Pref(output,&ph,data);
   		}
   		
 			if(ret) {
				ph.tag = T_CREATOR;
				ph.dataLen = (unsigned short) sizeof(short);
   			*(short *)data = gPrefs.creator;
   			ret = Write1Pref(output,&ph,data);
   		}
   		
 			if(ret) {
				ph.tag = T_CREATORNAME;
				ph.dataLen = (unsigned short) pstrlen(gPrefs.creatorName) + 1;
   			pstrcpy((StringPtr)data,gPrefs.creatorName);
   			ret = Write1Pref(output,&ph,data);
   		}
   		
			if(ret) {
				ph.tag = T_CREATORSIG;
				ph.dataLen = (unsigned short) sizeof(OSType);
   			*(OSType *)data = gPrefs.creatorSig;
   			ret = Write1Pref(output,&ph,data);
   		}
   		
			if(ret) {
				ph.tag = T_CONFIRMCHG;
				ph.dataLen = (unsigned short) sizeof(Boolean);
   			*(Boolean *)data = gPrefs.confirmChg;
   			ret = Write1Pref(output,&ph,data);
   		}
   		
			if(ret) {
				ph.tag = T_STARTQOPEN;
				ph.dataLen = (unsigned short) sizeof(Boolean);
   			*(Boolean *)data = gPrefs.startQOpen;
   			ret = Write1Pref(output,&ph,data);
   		}

   		if(!ret) {
				sprintf(gError,LoadErrorStr(ERR_WRITEPREF,FALSE),err);
				ErrorMsg(0);
			}
			
   		FSClose(output);
		}
   }
}

/**************************************
*	Write header and data block for one Preferences field
*/

static Boolean Write1Pref(short output,PrefHeader *ph,Byte *data)
{
	OSErr				err;
	unsigned long	count;

	count = sizeof(PrefHeader);
	err = WriteMacFile( output, (long *)&count, ph,"\p",FALSE);
	if(err == noErr) {
	 	count = (unsigned long) ph->dataLen;
	 	err = WriteMacFile( output, (long *)&count, data,"\p",FALSE);
	}

	return(err == noErr);
}

/**************************************
*	Select a new text file creator
*	Return value:	True, if user selected one and no error occurred
*						False, if user canceled or an error occured
*/

Boolean PickNewCreator()
{
	DialogPtr	myDialog;
	Point			where;
	SFTypeList	myTypes;
	SFReply 		reply;
	FInfo			fndrInfo;
	OSErr			err;
	
	/* show pick dialog */
	myDialog=GetNewDialog(PICKCREATOR_DLG,NULL,(WindowPtr)-1);
	
	CenterSFDlg(getDlgID,&where);
	MoveWindow(myDialog,where.h,where.v-39,TRUE);
	ShowWindow(myDialog);
	DrawDialog(myDialog);

	myTypes[0]='APPL';
	InitCursor();
	SFGetFile(where,"\p",NULL,1,myTypes,NULL,&reply); /* ask for file name */
   DisposDialog(myDialog);

   if (reply.good) {							/* File was selected 					*/
   	if((err = GetFInfo(reply.fName,reply.vRefNum,&fndrInfo)) != noErr) {
  			sprintf(gError,LoadErrorStr(ERR_OPENFILE,FALSE),PtoCstr(reply.fName),err);
   		CtoPstr((char *)reply.fName);
	  		return(ErrorMsg(0));
	  	}
   	else {
   		pstrcpy(gPrefs.creatorName,reply.fName);
   		gPrefs.creatorSig = fndrInfo.fdCreator;
   	}
	}
	
	return(reply.good);
}

/**************************************
*	Show Options dialog and let user set some general preferences
*/

void GeneralOptions()
{
	DialogPtr	myDialog;
	Boolean		bQuit = FALSE,bChanged = FALSE;
	short			itemHit;
	
	CenterDA('DLOG',OPTIONS_DLG,66);
	myDialog=GetNewDialog(OPTIONS_DLG,NULL,(WindowPtr)-1);

	/* show current settings */
	if(gPrefs.confirmChg)
		SetRadioButton(myDialog,CONFIRM_BOX,BTNON);
	else
		SetRadioButton(myDialog,CONFIRM_BOX,BTNOFF);
		
	if(gPrefs.startQOpen)
		SetRadioButton(myDialog,STARTQ_BOX,BTNON);
	else
		SetRadioButton(myDialog,STARTQ_BOX,BTNOFF);	
	
	/* install user items to draw frame and default button outline */
	InstallUserItem(myDialog,OPTIONS_USRITEM1,-1,DrawFrame);
	InstallUserItem(myDialog,OPTIONS_USRITEM2,OK,DrawOKBoxRect);

	ShowWindow(myDialog);
	while(!bQuit) {	/* display dialog using our standard filter proc */			
		ModalDialog((ProcPtr)myDialogFilter,&itemHit);
		switch(itemHit) {
			case OK:
			case Cancel:
				bQuit=TRUE;
				break;
			case CONFIRM_BOX:
			case STARTQ_BOX:
				ToggleRadioButton(myDialog,itemHit);
				bChanged = TRUE;
				break;
		}
	}
		
	/* if user clicked OK and something has changed we keep new settings */
	if(itemHit == OK && bChanged) {
		gPrefs.confirmChg = (Boolean)GetRadioButton(myDialog,CONFIRM_BOX);
		gPrefs.startQOpen = (Boolean)GetRadioButton(myDialog,STARTQ_BOX);
		
		ErrorMsg(ERR_PREFCHGWARN);
	}
	
	DisposDialog(myDialog);
}