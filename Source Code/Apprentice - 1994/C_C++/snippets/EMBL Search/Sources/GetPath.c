/*
*********************************************************************
*	
*	GetPath.c
*	acording to TN.SC018
*	Apple Computer Developer Technical Support
*	modified by
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
*

/* Definition:

	short GetPathFromWD(short vRefNum, Str255 fName,char *fullPathName);

	From a working directory reference number vRefNum and a file name fName
	(Pascal string!) GetPathFromWD builds the complete access path.
	Return value:  TRUE (1) or FALSE (0).
		by side effect: fullPathName contains access path (C string!).
						
	short GetPathFromDirID(short vRefNum,short DirId,Str255 fName,char *fullPathName);
	
	From a directory ID, a real volume reference number GetPathFromDirID and
	a file name (Pascal string !) GetPathFromDirID builds the complete access
	path.
	Return value:  TRUE (1) or FALSE (0).
		by side effect: fullPathName contains access path (C string!).
*/

#include <String.h>

/*
******************************* Prototypes ***************************
*/

#include "GetPath.h"

static Boolean CheckAUX(void);



static Boolean CheckAUX()
{
	SysEnvRec theWorld;
	short *flagPtr;
	
#define HWCfgFlags	0xA50

	SysEnvirons(curSysEnvVers,&theWorld);
	flagPtr= (short *)HWCfgFlags;
	if(*flagPtr & (1<<9))
		return(TRUE);
	else return(FALSE);
}

Boolean GetPathFromDirID(short vRefNum,long DirID, StringPtr fName,char *fullPathName)
{
	CInfoPBRec myCPB;
	Str255 directoryName;
	OSErr ret;

	strcpy(fullPathName,PtoCstr(fName));
	CtoPstr((char *)fName);
	
	myCPB.dirInfo.ioNamePtr= directoryName;
	myCPB.dirInfo.ioDrParID= DirID;
	myCPB.dirInfo.ioVRefNum= vRefNum;
	myCPB.dirInfo.ioFDirIndex= -1;
	
	do {
		myCPB.dirInfo.ioDrDirID=myCPB.dirInfo.ioDrParID;
		if((ret=PBGetCatInfo(&myCPB,FALSE)) != noErr) return (FALSE);
		
		PtoCstr(directoryName);
		if(CheckAUX()) {
			if(*directoryName != '/') /* if not root */
				strcat((char *)directoryName,"/");
		}
		else
			strcat((char *)directoryName,":");
		strcat((char *)directoryName,fullPathName);
		strcpy(fullPathName,(char *)directoryName);
	} while (myCPB.dirInfo.ioDrDirID != 2);
	
	return(TRUE);
}

Boolean GetPathFromWD(short vRefNum,StringPtr fName,char *fullPathName)
{
	WDPBRec myPB;
	OSErr ret;
	
	/* to work around A/UX 1.1 bug: */
	
	if(CheckAUX() && vRefNum == -1)
		return(GetPathFromDirID(-1,2,fName,fullPathName));
		
	/* get working directory information */

	myPB.ioNamePtr=	NULL;
	myPB.ioVRefNum=	vRefNum;
	myPB.ioWDProcID=	0;
	myPB.ioWDIndex=	0;
	
	if((ret=PBGetWDInfo(&myPB,FALSE)) != noErr) return(FALSE);
	
	return(GetPathFromDirID(myPB.ioWDVRefNum,myPB.ioWDDirID,fName,fullPathName));
}