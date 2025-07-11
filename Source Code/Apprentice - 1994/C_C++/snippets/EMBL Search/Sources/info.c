/*
*********************************************************************
*	
*	info.c
*	Information alert boxes
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

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "info.h"
#include "window.h"
#include "util.h"
#include "pstr.h"

static StringPtr GetVersion(void);


/*
******************************* Global variables *********************
*/

extern DBInfo gDBInfo[DB_NUM];
extern short gAppResRef;


/**************************************
*	Draws About� box
*/

void ShowAbout()
{
	EventRecord	myEvent;
	DialogPtr	myDialog;
	
	CenterDA('DLOG',ABOUT_DLG,0);
	myDialog=GetNewDialog(ABOUT_DLG,NULL,(WindowPtr)-1);
	SetDlgText(myDialog,ABOUT_VERSION_STR,GetVersion());
	ShowWindow(myDialog);
	DrawDialog(myDialog);
	while(!WaitNextEvent(keyDownMask|autoKeyMask|mDownMask,&myEvent,0L,NULL))
		;
	DisposDialog(myDialog);
}

/**************************************
*	Reads version number from resource file
*	Return value:	Pointer to Pascal string containing the version number
*/

static StringPtr GetVersion()
{
	short				curResFile;
	VersRecHndl		version;
	static Str255	versNum;

	/* store refnum of current resource file */
	curResFile = CurResFile();
	
	UseResFile(gAppResRef);	
	version=(VersRecHndl)Get1Resource('vers',1);
	if(version != NULL) {
		pstrcpy(versNum,(**version).shortVersion);
		ReleaseResource((Handle)version);
	}
	else pstrcpy(versNum,"\p?");

	UseResFile(curResFile);	
	return(versNum);
}

/**************************************
*	Draws database information dialog
*/

void ShowDBInfo()
{
	char			str1[256],str2[256];
	Str255		str3,str4;
	DialogPtr	infoDlg;
	short			kind,itemHit;
	Handle		h;
	Rect			r;

	CenterDA('DLOG',INFO_DLG,33);
	infoDlg=GetNewDialog(INFO_DLG,NULL,(WindowPtr)-1L);
	
	/* Set user items to draw frame and surround OK button */
	InstallUserItem(infoDlg,INFO_USRITEM1,-1,DrawFrame);
	InstallUserItem(infoDlg,INFO_USRITEM2,OK,DrawOKBoxRect);

	/* Set EMBL database release number and date */	
	sprintf(str1,"%s (%s)",gDBInfo[DB_EMBL].DBRelNum,gDBInfo[DB_EMBL].DBRelDate);
	SetDlgText(infoDlg,INFO_EMBL_REL,CtoPstr(str1));

	/* Set Swiss-Prot database release number and date */	
	sprintf(str2,"%s (%s)",gDBInfo[DB_SWISS].DBRelNum,gDBInfo[DB_SWISS].DBRelDate);
	SetDlgText(infoDlg,INFO_SWISS_REL,CtoPstr(str2));

	/* Set # of entries in EMBL to # of records in entry name index */
	NumToString(gDBInfo[DB_EMBL].ename_nrec,str3);
	SetDlgText(infoDlg,INFO_EMBL_ENTRIES,str3);

	/* Set # of entries in Swiss-Prot to # of records in entry name index */
	NumToString(gDBInfo[DB_SWISS].ename_nrec,str4);
	SetDlgText(infoDlg,INFO_SWISS_ENTRIES,str4);

	ShowWindow(infoDlg);
	DrawDialog(infoDlg);
	do {
		ModalDialog(NULL,&itemHit);
	} while (itemHit != OK);
	DisposDialog(infoDlg);
}