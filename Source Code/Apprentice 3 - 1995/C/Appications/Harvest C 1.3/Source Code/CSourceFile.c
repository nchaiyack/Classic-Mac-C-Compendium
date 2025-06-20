/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/* CSourceFile.c */

#include "CSourceFile.h"
#include "CHarvestApp.h"
#include "CDataFile.h"
#include "CWindow.h"
#include "CErrorLog.h"
#include "structs.h"

extern CSourceFile *gCurSourceFile;
extern CHarvestApp *gApplication;
extern CErrorLog *gErrs;

int next_pool = 1234;

void CSourceFile::ISourceFile(CDataFile *aFile)
{
	theFile = aFile;
	itsOptions = NULL;
	itsErrorLog = NULL;
	itsHeaders = NULL;
	itsKind = H_SourceFile;
	itsPool = next_pool++;
	new_malloc_pool(itsPool,1024*16);
}



char *CSourceFile::GetPathName(char *s)
{
	CInfoPBRec	block;
	Str255 directoryName;
	OSErr err;
	
	*s = 0;
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = theFile->dirID;
	
	do {
		block.dirInfo.ioVRefNum = theFile->volNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		
		err = PBGetCatInfo(&block,FALSE);

		ConcatPStrings(directoryName,"\p:");
		ConcatPStrings(directoryName,s);
		CopyPString(directoryName,s);
	} while (block.dirInfo.ioDrDirID != 2);
	
	ConcatPStrings(s,theFile->name);
	return s;
}

FILE *CSourceFile::StdOpen(char *mode)
{
	char path[512];
	GetPathName(path);
	p2cstr(path);
	return fopen(path,mode);
}

int CSourceFile::Compile(void)
{
    int             			result = 0;
    SymImageVia_t				gotdec;
    short						filenum;
    FSSpec aFileSpec;
    CursHandle curs;
    Boolean wasLocked;
    CWindow *theWindow;
    char mesg[64];
    
	    wasLocked = Lock(TRUE);
	
		gCurSourceFile = this;
		itsErrorLog = new CErrorLog;
		itsErrorLog->IErrorLog();
		itsErrorLog->BeginDialog();
		gErrs = itsErrorLog;
		
	    gApplication->SpinCursor();
	  
	  	theWindow = gErrs->GetWindow();
		sprintf(mesg,"Compilation of %#s",theFile->name);
		c2pstr(mesg);
		theWindow->SetTitle((unsigned char *) mesg);
	
		set_default_pool(itsPool);

		theFile->GetFSSpec(&aFileSpec);
		p2cstr(aFileSpec.name);
	    if (!ReInit((char *) aFileSpec.name, aFileSpec.vRefNum, aFileSpec.parID)) {
	    	Lock(wasLocked);
			return 0;
		}
	
		if (!gAbortCompile) {
	    	FreeAllRegs();		/* Is this necessary ? */
	    	gotdec = Do_external_declaration(filenum);
	    	if (gotdec) {
	    		result = 1;
	    		FreeSymImage(gotdec);
		    }
	    	if (!gotdec && !gAllDone) {
				SyntaxError("Expected external declaration");
	    	}
	    }
	    while (gotdec && !gAllDone && !gAbortCompile) {
		    gApplication->SpinCursor();
			FreeAllRegs();		/* Is this necessary ? */
			/* Then, we generate code for everything. */
			if ((gotdec = Do_external_declaration(filenum)) != 0) {
				result++;
				FreeSymImage(gotdec);
			}
	    }
	    if (!NumErrors) {
			GenGlobal(GlobalSymbolTable, GlobalCodes, 1);
			GenSegments(GlobalCodes);
			GenStringLits(GlobalCodes);
			GenFloatLits(GlobalCodes);
			GenStatics(GlobalCodes);
			Optimize68(GlobalCodes);
	 	}
	    if (!NumErrors && !gAbortCompile) {
			filenum = InitOMF(GlobalCodes, (char *) aFileSpec.name, aFileSpec.vRefNum, aFileSpec.parID);
			if (GlobalCodes && GlobalRecords)
				DumpCodeList(GlobalCodes, GlobalRecords);
			FinishOMF(filenum);
		}
	    OneFileCleanUp();
	    free_pool_memory();

	    InitCursor();
	    Lock(wasLocked);
	    return (!NumErrors);
}

CDataFile *CSourceFile::GetObjectFile(void)
{
	Str63 objName;
	CDataFile *objFile;

	CopyPString(theFile->name,objName);

	objName[objName[0]] = 'o';
	objFile = new CDataFile;
	objFile->IDataFile();
	objFile->SpecifyHFS(objName,theFile->volNum,theFile->dirID);
	return objFile;
}

static unsigned long
GetFileModDate(Str63 filename, short volrefnum, long dirID)
{
    HFileParam                      k;
    OSErr                           bad;
    k.ioCompletion = NULL;
    k.ioNamePtr = filename;
    k.ioVRefNum = volrefnum;
    k.ioFDirIndex = 0;
    k.ioDirID = dirID;
    bad = PBHGetFInfo((HParmBlkPtr) & k, false);
    if (!bad)
		return k.ioFlMdDat;
    else
		return 0;
}

unsigned long CSourceFile::GetSourceModDate(void)
{	
	return GetFileModDate(theFile->name,theFile->volNum,theFile->dirID);
}

unsigned long CSourceFile::GetObjectModDate(void)
{
	CDataFile *objFile;
	objFile = GetObjectFile();
	if (objFile->ExistsOnDisk())
		return GetFileModDate(objFile->name,objFile->volNum,objFile->dirID);
	else return 0;
}

void CSourceFile::Dispose(void)
{
	theFile->Dispose();
	inherited::Dispose();
}