/*
*********************************************************************
*	
*	locate.c
*	Locating index directories
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

#include <stdio.h>
#include <string.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ****************************
*/

#include "locate.h"
#include "util.h"
#include "window.h"
#include "pstr.h"
#include "cd.h"
#include "getpath.h"
#include "events.h"

static void ShortenPath(StringPtr path, DialogPtr myDialog, short item);
static pascal Boolean FoldersOnly(ParmBlkPtr pb);
static pascal short myGetDirHook(short theItem, DialogPtr myDialog);


/*
********************************* Globals *****************************
*/

extern short	gPrefVRefNum;
extern Prefs	gPrefs;
extern VolInfo	gCurrentCD;
extern DBInfo	gDBInfo[DB_NUM];

static Boolean	lbCurDirValid, lbCDROM;
static long		lgMyCurDir;
static SFReply	reply;


/**************************************
*	Preferences: change directory locations
*/

void Locate()
{
	EventRecord	myEvent;
	DialogPtr	myDialog;
	Str255		EMBLIndexPath,SWISSIndexPath;
	Boolean		bQuit = FALSE,bEMBLchanged = FALSE, bSWISSchanged = FALSE;
	short			itemHit;
	DirSpec		newSpec[DB_NUM];
	OSErr			ret;
	short			i;
	
	CenterDA('DLOG',LOCATE_DLG,66);
	myDialog=GetNewDialog(LOCATE_DLG,NULL,(WindowPtr)-1);

	/* show current path to EMBL index directory */
	GetDirPath(&gPrefs.inxDirSpec[DB_EMBL],EMBLIndexPath);
	ShortenPath(EMBLIndexPath,myDialog,LOCATE_EMBLINX_TXT);
	SetDlgText(myDialog,LOCATE_EMBLINX_TXT,EMBLIndexPath);
	
	/* show current path to SWISS index directory */
	GetDirPath(&gPrefs.inxDirSpec[DB_SWISS],SWISSIndexPath);
	ShortenPath(SWISSIndexPath,myDialog,LOCATE_SWISSINX_TXT);
	SetDlgText(myDialog,LOCATE_SWISSINX_TXT,SWISSIndexPath);

	/* install user items to draw frame and default button outline */
	InstallUserItem(myDialog,LOCATE_USRITEM1,-1,DrawFrame);
	InstallUserItem(myDialog,LOCATE_USRITEM2,OK,DrawOKBoxRect);

	ShowWindow(myDialog);
	while(!bQuit) {	/* display dialog using our standard filter proc */			
		ModalDialog((ProcPtr)myDialogFilter,&itemHit);
		switch(itemHit) {
			case OK:
			case Cancel:
				bQuit=TRUE;
				break;
			case LOCATE_EMBLCHG_BTN:
				/* ask user for new directory location */
				if(DoGetDirectory(&newSpec[DB_EMBL])) {
					GetDirPath(&newSpec[DB_EMBL],EMBLIndexPath);
					ShortenPath(EMBLIndexPath,myDialog,LOCATE_EMBLINX_TXT);
					SetDlgText(myDialog,LOCATE_EMBLINX_TXT,EMBLIndexPath);
					bEMBLchanged = TRUE;
				}
				break;
			case LOCATE_SWISSCHG_BTN:
				/* ask user for new directory location */
				if(DoGetDirectory(&newSpec[DB_SWISS])) {
					GetDirPath(&newSpec[DB_SWISS],SWISSIndexPath);
					ShortenPath(SWISSIndexPath,myDialog,LOCATE_SWISSINX_TXT);
					SetDlgText(myDialog,LOCATE_SWISSINX_TXT,SWISSIndexPath);
					bSWISSchanged = TRUE;
				}
				break;
		}
	}
		
	DisposDialog(myDialog);
	/* if user clicked OK and something has changed we copy new directory
		information to gPrefs */
	if(itemHit == OK && (bEMBLchanged || bSWISSchanged)) {
		if(bEMBLchanged) {
			pstrcpy(gPrefs.inxDirSpec[DB_EMBL].volName, newSpec[DB_EMBL].volName);
			gPrefs.inxDirSpec[DB_EMBL].dirID = newSpec[DB_EMBL].dirID;
		}
		
		if(bSWISSchanged) {
			pstrcpy(gPrefs.inxDirSpec[DB_SWISS].volName, newSpec[DB_SWISS].volName);
			gPrefs.inxDirSpec[DB_SWISS].dirID = newSpec[DB_SWISS].dirID;
		}
		
		/* close old wd's and open new one's */
		ret=CloseWD(gDBInfo[DB_EMBL].SeqWDRefNum);
		ret=CloseWD(gDBInfo[DB_SWISS].SeqWDRefNum);
		ret=CloseWD(gDBInfo[DB_EMBL].InxWDRefNum);
		ret=CloseWD(gDBInfo[DB_SWISS].InxWDRefNum);
		if(!InitDB(gCurrentCD.vRefNum,gCurrentCD.volName)) {
			ErrorMsg(ERR_REINITFAIL);	/* this should never occur */
			for(i=0;i<DB_NUM;++i) {
				*gPrefs.inxDirSpec[i].volName = EOS;
				gPrefs.inxDirSpec[i].dirID = 0;
			}
			InitCD();	/* complete restart */
		}
		
		ErrorMsg(ERR_PREFCHGWARN);
	}
}

/**************************************
*	Build a complete path from a DirSpec. If volume name is EOS this indicates
*	that CD-ROM should be used (default).
*/

Boolean GetDirPath(DirSpec *dirSpec,StringPtr path)
{
	Str31 cdStr;
	short vRefNum;
	char	fullPath[256];
	
	GetIndString(cdStr,OTHERS,CDROMPATH);
	pstrcpy(path,cdStr);

	if(*dirSpec->volName == EOS) {
		return(TRUE);
	}
	else {
		/* find volume number for a given volume name */
		if(!GetVRefNumFromName(dirSpec->volName,&vRefNum)) {
			return(FALSE);
		}
		else {
			/* get complete path from dir ID and volume name */
			if(!GetPathFromDirID(vRefNum,dirSpec->dirID,
									"\p",fullPath)) {
				return(FALSE);
			}
			else {
  				strcpy((char *)path,fullPath);
				CtoPstr((char *)path);
				return(TRUE);
			}
		}
	}
}

/**************************************
*	Shorten a complete path to fit into a dialog static text item.
*	We first try the complete path. If it doesn't fit, we take the 
*	volume name, add a 'É' character and then as many subfolder names
*	as possible to fit into the given box. We will always show at least
*	two levels of the file system hierarchy, the volume name and the file name
*	or the directory name in case the path points to a directory.
*	This routine works for both, file and directory path specifications.
*	Return value: by side-effect: modified path in "path"
*/

static void ShortenPath(StringPtr path, DialogPtr myDialog, short item)
{
	short		kind;
	Handle	h;
	Rect		r;
	short		boxWidth;
	GrafPtr	savePort;
	Str255	newPath;
	char		topFolder[256], *p;
	short		depth,maxDepth;
	short		i;
	
	savePort = ChangePort(myDialog);
	
	GetDItem(myDialog,item,&kind,&h,&r);
	boxWidth = r.right - r.left;
		
	PtoCstr(path);
	strcpy((char *)newPath,(char *)path);
	/* for each directory/volume in path increase depth counter */
	for(p=(char *)newPath,maxDepth=0;*p;++p) {	
		if(*p == ':')
			++maxDepth;
	}
	/* if last element in path is filename, add one level of depth */
	if(*(p-1) != ':')			 
		++maxDepth;
		
	/* we will keep at least two levels in the path, therefore we need to do
		any modifications only if maxDepth is greater than 2 */
	if(maxDepth > 2) {
		strcpy(topFolder,(char *)newPath);
		for(p=topFolder;*p != ':';++p)
			;
		*p = EOS;
		strcat(topFolder,":É:");
	}
		
	CtoPstr((char *)newPath);
	for(depth=2;depth < maxDepth && StringWidth(newPath) > boxWidth;++depth) {
		strcpy((char *)newPath,topFolder);
		for(p=(char *)path,i=0;i<depth;++p) {
			if(*p == ':')
				++i;
		}
		strcat((char *)newPath,p);
		CtoPstr((char *)newPath);
	}
	
	pstrcpy(path,newPath);
	SetPort(savePort);
}

/**************************************
*	Get new directory from user. We use a modified SFGetFile dialog,
*	modelled on Apple DTS SC.18. User can also choose "CD-ROM".
*	Return values:	TRUE, if user selected a directory
*						FALSE; if user cancelled
*/

Boolean DoGetDirectory(DirSpec *dirSpec)
{
	Str31 str;
	Point where;
	
	/* Deactivate current front window. SFPut/GetFile sends an activate event
		when it quits but not when it opens ! */
	HandleActivates(FrontWindow(),0);
	CenterSFDlg(DIRSELECT_DLG,&where);
	SFPGetFile(where,"\p",(ProcPtr)FoldersOnly,-1,NULL,
				(ProcPtr)myGetDirHook,&reply,DIRSELECT_DLG,NULL);
					
	if(lbCurDirValid) {	/* lbCurDirValid is TRUE, if user selected a directory */
		if(lbCDROM) {		/* lbCDROM is TRUE, if user selected CDROM */
			*dirSpec->volName = EOS;	
			dirSpec->dirID = 0;
		}
		else {
			/* Pick up volume vRefNum from LoMem variable and find volume name */
			if(!GetNameFromVRefNum(str,-SFSaveDisk))
				return(FALSE);
			else {
				pstrcpy(dirSpec->volName,str);
				dirSpec->dirID = lgMyCurDir;
			}
		}
		
		return(TRUE);
	}
	else return(FALSE);
}

/**************************************
*	Make sure we show only folders in the SFDialog list
*/

static pascal Boolean FoldersOnly(ParmBlkPtr pb)
{
	return((pb->fileParam.ioFlAttrib & 0x10) == 0);
}

/**************************************
*	Our hook routine to handle the non-standard buttons
*/

static pascal short myGetDirHook(short theItem, DialogPtr myDialog)
{
#define getDirButton 11
#define getDirNowButton 12
#define getCDButton 13

	switch(theItem) {
		/* initialisation of dialog
			(called first time before dialog is displayed) */
		case -1:
			lbCurDirValid = FALSE;
			lbCDROM = FALSE;
			break;
		case getDirButton:
			if(reply.fType != 0) {
				lgMyCurDir = reply.fType;
				lbCurDirValid = TRUE;
				theItem = getCancel;
			}
			break;
		case getDirNowButton:
			lgMyCurDir = CurDirStore; /* get dirID from LoMem variable */
			lbCurDirValid = TRUE;
			theItem = getCancel;
			break;
		case getCDButton:
			lbCDROM = TRUE;
			lbCurDirValid = TRUE;
			theItem = getCancel;
			break;
	}
	
	return(theItem);
}