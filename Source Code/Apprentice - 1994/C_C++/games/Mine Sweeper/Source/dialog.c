/*	dialog.c
 *
 *		This is the preferences manager dialog box
 */

#include <stdio.h>
#include "mines.h"

/*	GetMyDialog
 *
 *		This gets my dialog box
 */

DialogPtr GetMyDialog(short id)
{
	DialogPtr dlog;
	short i1;
	Handle i2;
	Rect i3;
	
	dlog = GetNewDialog(id,NULL,(WindowPtr)-1);
	if (dlog == NULL) return;
	SetPort(dlog);
	GetDItem(dlog,1,&i1,&i2,&i3);
	InsetRect(&i3,-4,-4);
	PenSize(3,3);
	FrameRoundRect(&i3,16,16);
	PenSize(1,1);
	return dlog;
}

/*	GetButton
 *
 *		Return button state
 */

short GetButton(DialogPtr dlog, short id)
{
	short i1;
	Handle i2;
	Rect i3;
	
	GetDItem(dlog,id,&i1,&i2,&i3);
	return GetCtlValue((ControlHandle)i2);
}

/*	SetButton
 *
 *		Set button state
 */

void SetButton(DialogPtr dlog, short id, short val)
{
	short i1;
	Handle i2;
	Rect i3;
	
	GetDItem(dlog,id,&i1,&i2,&i3);
	SetCtlValue((ControlHandle)i2,val);
}

/*	GetName
 *
 *		What is the user's name
 */

void GetName(DialogPtr dlog, short id, char *c)
{
	short i1;
	Handle i2;
	Rect i3;
	unsigned char buffer[256];
	
	GetDItem(dlog,id,&i1,&i2,&i3);
	GetIText(i2,buffer);
	if (buffer[0] > 31) buffer[0] = 31;
	for (i1 = buffer[0]; i1 >= 0; i1--) c[i1] = buffer[i1];
}

/*	SetValue
 *
 *		Set the value
 */

void SetValue(DialogPtr dlog, short id, short val)
{
	unsigned char buffer[256];
	short i1;
	Handle i2;
	Rect i3;
	
	NumToString(val,buffer);
	GetDItem(dlog,id,&i1,&i2,&i3);
	SetIText(i2,buffer);
}

/*	GetValue
 *
 *		Get the value
 */

short GetValue(DialogPtr dlog, short id)
{
	unsigned char buffer[256];
	short i1;
	Handle i2;
	Rect i3;
	long n;
	
	GetDItem(dlog,id,&i1,&i2,&i3);
	GetIText(i2,buffer);
	StringToNum(buffer,&n);
	return n;
}

/********************************************************************/
/*																	*/
/*	Dialog execution												*/
/*																	*/
/********************************************************************/

/*	Preferences
 *
 *		This sets the various preferences
 */

void Preferences(void)
{
	DialogPtr dlog;
	short i;
	short j;
	short x,y;
	
	dlog = GetMyDialog(129);
	
	SetButton(dlog,3,(SizeFlag == 0) ? 1 : 0);
	SetButton(dlog,4,(SizeFlag == 1) ? 1 : 0);
	SetButton(dlog,5,(SizeFlag == 2) ? 1 : 0);
	SetButton(dlog,6,(BombFlag == 0) ? 1 : 0);
	SetButton(dlog,7,(BombFlag == 1) ? 1 : 0);
	SetButton(dlog,16,(Cruse) ? 1 : 0);
	
	SetValue(dlog,10,SizeX);
	SetValue(dlog,11,SizeY);
	SetValue(dlog,12,BRatio);
	
	for (;;) {
		ModalDialog(NULL,&i);
		switch (i) {
			case 1:
				SizeX = GetValue(dlog,10);
				SizeY = GetValue(dlog,11);
				BRatio = GetValue(dlog,12);
				for (j = 3; j <= 5; j++) if (GetButton(dlog,j)) break;
				SizeFlag = j - 3;
				for (j = 6; j <= 7; j++) if (GetButton(dlog,j)) break;
				BombFlag = j - 6;
				
				Cruse = GetButton(dlog,16);
				
				if (SizeX < 5) SizeX = 5;
				if (SizeX > 40) SizeX = 40;
				if (SizeY < 5) SizeY = 5;
				if (SizeY > 40) SizeY = 40;
				if (BRatio < 5) BRatio = 5;
				if (BRatio > 40) BRatio = 40;
				
				NewMines();
			case 2:
				DisposDialog(dlog);
				return;
			case 3:
			case 4:
			case 5:
				for (j = 3; j <= 5; j++) SetButton(dlog,j,(i == j) ? 1 : 0);
				switch (i) {
					case 3:		x = 10, y = 10; break;
					case 4:		x = 15, y = 15; break;
					case 5:		x = 30, y = 15; break;
				}
				SetValue(dlog,10,x);
				SetValue(dlog,11,y);
				break;
			case 6:
			case 7:
				for (j = 6; j <= 7; j++) SetButton(dlog,j,(i == j) ? 1 : 0);
				switch (i) {
					case 6:		x = 10; break;
					case 7:		x = 5; break;
				}
				SetValue(dlog,12,x);
				break;
			case 10:
			case 11:
				x = GetValue(dlog,10);
				y = GetValue(dlog,11);
				i = 0;
				if ((x == 10) && (y == 10)) i = 3;
				if ((x == 15) && (y == 15)) i = 4;
				if ((x == 30) && (y == 15)) i = 5;
				for (j = 3; j <= 5; j++) SetButton(dlog,j,(i == j) ? 1 : 0);
				break;
			case 12:
				x = GetValue(dlog,12);
				i = 0;
				if (x == 10) i = 6;
				if (x == 5)  i = 7;
				for (j = 6; j <= 7; j++) SetButton(dlog,j,(i == j) ? 1 : 0);
				break;
			case 16:
				SetButton(dlog,16,1 - GetButton(dlog,16));
				break;
		}
	}
}

/*	GetUserName
 *
 *		Who is the user?
 */

void GetUserName(char *c)
{
	DialogPtr dlog;
	short i;
	
	dlog = GetMyDialog(132);
	for (;;) {
		ModalDialog(NULL,&i);
		if (i == 1) break;
	}
	GetName(dlog,3,c);
	DisposDialog(dlog);
}

/********************************************************************/
/*																	*/
/*	Preferences Files												*/
/*																	*/
/********************************************************************/

static unsigned char myName[] = "\pMines Preferences";
static unsigned char myFolder[] = "\pPreferences";

/*	FindPrefFolder
 *
 *		This determines the DirID of the preferences folder.  If the
 *	preferences folder doesn't exist and can't be created, then
 *  return DirID of the system folder.
 */

long FindPrefFolder(void)
{
	SysEnvRec world;
	CInfoPBRec p;
	HParamBlockRec h;
	long d;
	
	/*
	 *	Get location and DirID of the system folder
	 */
	
	SysEnvirons(1,&world);								/* Get environs record */
	
	h.wdParam.ioCompletion = NULL;
	h.wdParam.ioVRefNum = world.sysVRefNum;				/* System ref ID */
	h.wdParam.ioWDIndex = 0;
	h.wdParam.ioWDProcID = 'wMNd';
	h.wdParam.ioWDVRefNum = world.sysVRefNum;
	h.wdParam.ioNamePtr = NULL;
	if (PBGetWDInfo((WDPBPtr)&h,0)) return 0;			/* Get system DirID */
	d = h.wdParam.ioWDDirID;							/* Store it away */

	/*
	 *	Figure out if the preferences directory exists.  If it doesn't, then
	 *	try to create it
	 */
	
	p.dirInfo.ioCompletion = NULL;
	p.dirInfo.ioNamePtr = (unsigned char *)myFolder;
	p.dirInfo.ioVRefNum = world.sysVRefNum;				/* System Folder */
	p.dirInfo.ioFDirIndex = 0;							/* Look up by work dir */
	p.dirInfo.ioDrDirID = 0;							/* Ignored? */
	if (PBGetCatInfo(&p,0)) {							/* Get dir ID if there */
		/*
		 *	the Preferences folder doesn't exist.  Create it
		 */

		h.fileParam.ioCompletion = NULL;
		h.fileParam.ioNamePtr = (unsigned char *)myFolder;
		h.fileParam.ioVRefNum = world.sysVRefNum;
		h.fileParam.ioDirID = d;						/* System folder */
		if (PBDirCreate(&h,0)) return d;				/* Return system folder ID */
		
		return h.fileParam.ioDirID;						/* Return *my* Dir ID */
	} else return p.dirInfo.ioDrDirID;					/* PBCat worked... */
}

/*	LoadPreferences
 *
 *		This loads the preferences file
 */

void LoadPreferences(void)
{
	HParamBlockRec h;
	PrefFiles pf;
	long l;
	SysEnvRec world;
	short x;

	SysEnvirons(1,&world);								/* Get environs record */
	if (world.machineType < 0) {
		/*
		 *	HFS Doesn't exist.  Don't use a preferences file.
		 */
		
		SizeX = 10;
		SizeY = 10;
		SizeFlag = 0;
		BRatio = 5;
		BombFlag = 1;
		XLoc = 2;
		YLoc = 38;
		
		Cruse = 0;
		
		for (x = 0; x < NUMSCORE; x++) {
			Scores[x].name[0] = '\0';
			Scores[x].time = 1000;
		}
		return;
	}
	
	h.ioParam.ioCompletion = NULL;
	h.ioParam.ioNamePtr = (unsigned char *)myName;
	h.ioParam.ioVRefNum = world.sysVRefNum;
	h.ioParam.ioPermssn = fsRdPerm;						/* Read only */
	h.ioParam.ioMisc = NULL;
	h.fileParam.ioDirID = FindPrefFolder();				/* Preferences folder... */
	if (PBHOpen(&h,0)) {
		SizeX = 10;
		SizeY = 10;
		SizeFlag = 0;
		BRatio = 5;
		BombFlag = 1;
		
		XLoc = 2;
		YLoc = 38;
		
		Cruse = 0;
		
		for (x = 0; x < NUMSCORE; x++) {
			Scores[x].name[0] = '\0';
			Scores[x].time = 1000;
		}
		return;											/* File didn't exist */
	}

	l = sizeof(pf);
	FSRead(h.ioParam.ioRefNum,&l,(Ptr)&pf);
	FSClose(h.ioParam.ioRefNum);						/* Read and close file */
	
	SizeX = pf.SizeX;
	SizeY = pf.SizeY;
	SizeFlag = pf.SizeFlag;
	BRatio = pf.BRatio;
	BombFlag = pf.BombFlag;
	
	XLoc = pf.XWindLoc;
	YLoc = pf.YWindLoc;
	Cruse = pf.Cruse;
	for (x = 0; x < NUMSCORE; x++) Scores[x] = pf.Scores[x];
}

/*	SavePreferences
 *
 *		Save preferences file
 */

void SavePreferences(void)
{
	HParamBlockRec h;
	PrefFiles pf;
	long l;
	short err;
	SysEnvRec world;
	short x;
	
	SysEnvirons(1,&world);								/* Get environs record */
	if (world.machineType < 0) return;					/* No HFS; don't save prefs */
	
	pf.SizeX = SizeX;
	pf.SizeY = SizeY;
	pf.SizeFlag = SizeFlag;
	pf.BRatio = BRatio;
	pf.BombFlag = BombFlag;
	pf.XWindLoc = MineWindow->portRect.left - MineWindow->portBits.bounds.left;
	pf.YWindLoc = MineWindow->portRect.top - MineWindow->portBits.bounds.top;
	pf.Cruse = Cruse;
	for (x = 0; x < NUMSCORE; x++) pf.Scores[x] = Scores[x];

	h.ioParam.ioCompletion = NULL;
	h.ioParam.ioNamePtr = (unsigned char *)myName;
	h.ioParam.ioVRefNum = world.sysVRefNum;
	h.ioParam.ioPermssn = fsWrPerm;						/* Read only */
	h.ioParam.ioMisc = NULL;
	h.fileParam.ioDirID = FindPrefFolder();				/* Preferences folder... */
	err = PBHOpen(&h,0);
	if (err) {
		err = PBHCreate(&h,0);
		if (err) return;
		if (PBHOpen(&h,0)) return;						/* Try to open it... */
	}
	
	SetEOF(h.ioParam.ioRefNum,0L);
	l = sizeof(pf);
	FSWrite(h.ioParam.ioRefNum,&l,(Ptr)&pf);
	FSClose(h.ioParam.ioRefNum);
	
	l = h.fileParam.ioDirID;							/* Store away DirID... */	
	PBHGetFInfo(&h,0);									/* Get finder info */
	h.fileParam.ioFlFndrInfo.fdType = 'Pref';			/* Preferences file */
	h.fileParam.ioFlFndrInfo.fdCreator = 'wMNd';		/* Mines package */
	h.fileParam.ioDirID = l;							/* Put back DirID */
	PBHSetFInfo(&h,0);
}
