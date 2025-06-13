
/*----------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	Standard File Sample Application
#
#	StdFile
#
#	StdFile.c	-	C Source
#
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:
#				1.00				04/89
------------------------------------------------
*/

#include "PB_sync.h"

void pStrcat(unsigned char *,unsigned char *);
void pStrcpy(unsigned char *,unsigned char *);
OSErr PathNameFromDirID(long,short,char*);
void doForceDirectory(WDPBRec*);
void SetRadioButton(DialogPtr,short,short);
void error_message(char *);

void pStrcat(s,t)
	unsigned char *s;
	register unsigned char *t;
{
	register unsigned char *s2;
	register short tLen;

	
	s2 = s + *s;
	*s += (tLen = *t);
	if(tLen>255) error_message("Error: Pascal string overflow\n");
	for (++tLen; --tLen; s2[tLen] = t[tLen]);
}

void pStrcpy(s,t)
	register unsigned char *s, *t;
{
	register short	tLen;

	for (tLen = *t + 1; tLen--; s[tLen] = t[tLen]);
}

/** PathNameFromDirID *********************************************************/
/*
/*	Given a DirID and real vRefnum, this routine will create and return the
/*	full pathname that corresponds to it. It does this by calling PBGetCatInfo
/*	for the given directory, and finding out its name and the DirID of its
/*	parent. It the performs the same operation on the parent, sticking ITS
/*	name onto the beginning of the first directory. This whole process is
/*	carried out until we have processed the root directory (identified with
/*	a DirID of 2.
/*
/******************************************************************************/

#if 0			/* no more used... */
OSErr PathNameFromDirID(DirID, vRefNum, s)
	long	DirID;
	short	vRefNum;
	char	*s;
{
	CInfoPBRec	block;
	Str255		directoryName;
	OSErr err;

	*s = 0;
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = DirID;

	do {
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;

		err = PBGetCatInfoSync(&block);
		if(err!=noErr) return err;
			/* Append a Macintosh style colon (':') */
		pStrcat(directoryName,"\p:");
		pStrcat(directoryName,s);
		pStrcpy(s,directoryName);
	} while (block.dirInfo.ioDrDirID != 2);

	return noErr;
}
#endif

/** doForceDirectory **********************************************************/
/*
/*	This is a quick sample that shows how to set the initial directory that
/*	Standard File comes up with. Basically, this is done by storing appropriate
/*	values into SFSaveDisk and CurDirStore. 
/*
/******************************************************************************/


void doForceDirectory(pb)
WDPBRec	*pb;
{
	/* Str255	s; */

	pb->ioWDIndex = 0;
	pb->ioWDProcID = 0;
	PBGetWDInfoSync(pb);

	CurDirStore = pb->ioWDDirID;
	SFSaveDisk = -pb->ioWDVRefNum;
}

/** SetRadioButton ************************************************************/
/*
/*	Handy routine for setting the value of a radio button. Given a dialog
/*	pointer, and item number, and a state, this routine will take care of
/*	the rest.
/*
/******************************************************************************/

void SetRadioButton(dialog,item,state)
	DialogPtr	dialog;
	short		item;
	short		state;
{
	short		kind;
	Handle		h;
	Rect		r;

	GetDItem(dialog,item,&kind,&h,&r);
	SetCtlValue((ControlHandle)h,state);
}
