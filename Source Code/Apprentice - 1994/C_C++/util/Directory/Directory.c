//-- Directory.c --//

// This takes the directory specified and produces a picture in the window 
// outlined with the contents of that picture.

#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include "struct.h"

HParamBlockRec	myCPB;
char			fName [255];
struct DrawWindow *myWindow;
long			curPLine;
long			totFiles, totDirect;
extern struct DrawWindow *drawList;

//-- CanEject --//

//- Return 1 if this drive can be ejected and unmounted.

int CanEject (i)
short i;
{
	char buffer [64];
	short vRefNum;
	long fileSize;
	QHdrPtr fqueue;
	struct DRIVE {
		QElemPtr qLink;
		short qType;
		short dQDrive;
		short dQRefNum;
		short dQFSID;
		short dQDrvSz;
		short dQDrvSz2;
	} *drive;
	
// Figure out what drive this thing is on by searching the mounted drive queue.
		
	fqueue = GetDrvQHdr ();
	drive = (struct DRIVE *) (fqueue->qHead);	// Offset -4.
	while (drive != NULL) {
		GetVInfo (drive->dQDrive, buffer, &vRefNum, &fileSize);
		if (vRefNum == i) {
			if (0x08 & ((unsigned char *) drive) [-3]) return 0;
			else return 1;
		}
		drive = (struct DRIVE *)(drive->qLink);
	}
	return 1;
}

//-- TAdd --//

// This adds data with the tab indentation specified to the window.

TAdd (w, t, s)
struct DrawWindow *w;
short t;
char *s;
{
	struct DirectData d;

	curPLine = GetHandleSize (w->data) / sizeof (struct DirectData);
	strcpy (d.data, s);
	CtoPstr (d.data);
	d.indent = t;
	d.auxdata [0] = '\0';
	d.auxdata2 [0] = '\0';

	PtrAndHand (&d, w->data, sizeof (d));
}

//-- TAppend --//

// Append string to the data line.

TAppend (w, l, s)
struct DrawWindow *w;
long l;
char *s;
{
	struct DirectData *ptr;

	HLock (w->data);
	ptr = * (w->data);
	PtoCstr (ptr [l].auxdata);
	strcpy (ptr [l].auxdata, s);
	CtoPstr (ptr [l].auxdata);
	HUnlock (w->data);
}

//-- TAppend2 --//

// Append string to the data line for file use.

TAppend2 (w, l, s)
struct DrawWindow *w;
long l;
char *s;
{
	struct DirectData *ptr;

	HLock (w->data);
	ptr = * (w->data);
	PtoCstr (ptr [l].auxdata2);
	strcpy (ptr [l].auxdata2, s);
	CtoPstr (ptr [l].auxdata2);
	HUnlock (w->data);
}

//-- SearchDisks --//

// This scans all the open windows to assure that the list of disks and windows
// do indeed match.  If they don't, this routine closes and opens as appropriate.

SearchDisks ()
{
	HParamBlockRec myHParam;
	int i;
	int j;
	
	for (j = 0; j < MAXWINDOWS; j++) if (drawList [j].inuse) drawList [j].inuse = 2;

	for (i = 1; ; i++) {
		myHParam.volumeParam.ioCompletion = NULL;
		myHParam.volumeParam.ioNamePtr = NULL;
		myHParam.volumeParam.ioVRefNum = 0;
		myHParam.volumeParam.ioVolIndex = i;
		if (PBHGetVInfo (&myHParam, 0)) break;
		for (j = 0; j < MAXWINDOWS; j++) {
			if ((myHParam.volumeParam.ioVRefNum == drawList [j].vRefNum) &&
				 (drawList [j].inuse != 0)) {
					drawList [j].inuse = 1;
					break;
			}
		}
		if (j == MAXWINDOWS) {
			for (j = 0; j < MAXWINDOWS; j++) if (drawList [j].inuse == 0) break;
			if (j != 0) NewPlan (myHParam.volumeParam.ioVRefNum);
		}
	}
	
	for (j = 0; j < MAXWINDOWS; j++) {
		if (drawList [j].inuse == 2) {
			DisposHandle (drawList [j].data);	/* Close without unmounting */
			CloseWindow (& (drawList [j]));	/* as this disk doesn't exist */
			FreeWind (& (drawList [j]));
		}
	}
}

//-- GetMounted --//

// This opens windows for all mounted volumes.

GetMounted ()
{
	HParamBlockRec myHParam;
	int i;

	for (i = 1; ; i++) {
		myHParam.volumeParam.ioCompletion = NULL;
		myHParam.volumeParam.ioNamePtr = NULL;
		myHParam.volumeParam.ioVRefNum = 0;
		myHParam.volumeParam.ioVolIndex = i;
		if (PBHGetVInfo (&myHParam, 0)) break;
		NewPlan (myHParam.volumeParam.ioVRefNum);
	}
}

//-- MyGetDInfo --//

// Print the directory information.

MyGetDInfo (t)
short t;
{
	char buffer [128];

	PtoCstr (fName);
	sprintf (buffer, ":%s:", fName);
	TAdd (myWindow, t, buffer);
	CtoPstr (fName);
}

//-- AddDirSize --//

// This adds the directory size to the file name stuff.

AddDirSize (l, s)
long l;
long s;
{
	char buffer [64];

	sprintf (buffer, "%ld", s);
	TAppend (myWindow, l, buffer);
}

//-- MyGetFInfo

// Print the file information.

MyGetFInfo (t)
short t;
{
	char buffer [128];
	union {
		OSType type;
		char name [6];
	} u;
	
	switch (myWindow->state) {
		case 2:
			break;
		case 1:
			if (myCPB.fileParam.ioFlFndrInfo.fdType != 'APPL') return;
			break;
		case 0:
			return;
	}

	PtoCstr (fName);
	sprintf (buffer, "%s", fName);
	TAdd (myWindow, t, buffer);
	CtoPstr (fName);
	
	AddDirSize (curPLine, myCPB.fileParam.ioFlRPyLen + myCPB.fileParam.ioFlPyLen);
	
	u.name [4] = '\0';
	u.type = myCPB.fileParam.ioFlFndrInfo.fdType;
	sprintf (buffer, "%s ", u.name);
	u.type = myCPB.fileParam.ioFlFndrInfo.fdCreator;
	strcat (buffer, u.name);
	TAppend2 (myWindow, curPLine, buffer);
}



long lenMem;					/* Don't need to recursively save in Enumer... */



//-- UpdateStatus

// Update what's going on in the current window.

UpdateStatus ()
{
	Rect r;
	char buffer [128];

	TextFont (1);
	TextSize (12);
	r.top = 0;
	r.left = 0;
	r.right = 500;
	r.bottom = 20;
	EraseRect (&r);
	MoveTo (10, 15);
	sprintf (buffer, "Found %ld files, %ld directories, %ld lines", totFiles, totDirect, curPLine);
	CtoPstr (buffer);
	DrawString (buffer);
}

//-- EnumerateCatalog

// Right out of TN68.

long EnumerateCatalog (t, dirIDToSearch)
short t;
long dirIDToSearch;
{
	short	index = 1;
	OSErr	err;
	long	curLine;
	long	curmem = 0;

	do {
		myCPB.fileParam.ioFDirIndex = index;
		myCPB.fileParam.ioDirID = dirIDToSearch;
		err = PBGetCatInfo (&myCPB, 0);
		if (err == noErr) {
			if (((myCPB.fileParam.ioFlAttrib >> 4) & 0x01) == 1) {
				MyGetDInfo (t);
				curLine = curPLine;
				lenMem = EnumerateCatalog (t+1, myCPB.fileParam.ioDirID);
				err = 0;
				AddDirSize (curLine, lenMem);
				curmem += lenMem;
				totDirect++;
			} else {
				MyGetFInfo (t);
				curmem += myCPB.fileParam.ioFlRPyLen + myCPB.fileParam.ioFlPyLen;
				totFiles++;
			}
		}
		index++;
	} while (err != fnfErr);
	UpdateStatus ();
	return curmem;
}






//-- ComputePict (name, w);

// This computes the content of the window specified.

ComputePict (name, w)
short name;
struct DrawWindow *w;
{
	char buffer [255];
	char vname [64];
	Rect r;
	HParamBlockRec myHParam;
	long curLine;
	CursHandle cursor;


	cursor = GetCursor (watchCursor);
	HLock (cursor);
	SetCursor (*cursor);
	HUnlock (cursor);
	HPurge (cursor);

	w->vRefNum = name;
	w->data = (struct DirectData **)NewHandle (0);

// Get volume name and information block.

	myHParam.volumeParam.ioCompletion = NULL;
	vname [0] = '\0';
	myHParam.volumeParam.ioNamePtr = (unsigned char *)vname;
	myHParam.volumeParam.ioVRefNum = name;
	myHParam.volumeParam.ioVolIndex = 0;
	PBHGetVInfo (&myHParam, 0);

	PtoCstr (vname);
	
	strcpy (buffer, vname);
	switch (w->state) {
		case 0:
			strcat (buffer, ":  Directories");
			break;
		case 1:
			strcat (buffer, ":  Applications");
			break;
		case 2:
			strcat (buffer, ":  All files");
			break;
	}
	CtoPstr (buffer);
	SetWTitle (w, buffer);
	strcpy (w->vName, vname);
	sprintf (buffer, "Volume \"%s\"", vname);
	TAdd (w, 0, buffer);
	sprintf (buffer, "Total files on volume:  %ld", myHParam.volumeParam.ioVFilCnt);
	TAdd (w, 1, buffer);
	sprintf (buffer, "Total directories:      %ld", myHParam.volumeParam.ioVDirCnt);
	TAdd (w, 1, buffer);
	if (myHParam.volumeParam.ioVSigWord == 0xD2D7) {
		sprintf (buffer, "This is an MFS volume");
		TAdd (w, 1, buffer);
	} else {
		sprintf (buffer, "This is an HFS volume");
		TAdd (w, 1, buffer);
	}
	buffer [0] = '\0';
	TAdd (w, 0, buffer);
	
// Scan the total directory, as in a Mac Technical Note #68.
	 
	sprintf (buffer, "%s:", vname);
	TAdd (w, 0, buffer);
	myCPB.fileParam.ioNamePtr = (unsigned char *)fName;
	myCPB.fileParam.ioVRefNum = name;
	myWindow = w;
	curLine = curPLine;
	SetPort (w);
	totFiles = 0;
	totDirect = 0;
	lenMem = EnumerateCatalog (1, 2L);
	AddDirSize (curLine, lenMem);
	
// Prepare for updating window.
	
	EraseRect (& (w->w.port.portRect));
	InvalRect (& (w->w.port.portRect));
	InitCursor ();
}