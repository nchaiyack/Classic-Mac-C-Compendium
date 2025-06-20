/**********************************************************************\

File:		file management.c

Purpose:	This module handles files for deBinHexing: opening the
			input file, creating/opening the temp file, reading and
			writing files, and finalizing them when we're done.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "Script.h"
#include "file management.h"
#include "environment.h"
#include "util.h"

unsigned long	theFileType, theFileCreator;
short			theFileFlags;
unsigned long	theCreationDate, theModificationDate;
FSSpec			inputFS, outputFS, tempFS;
Boolean			deleteBinHexFile;
short			inputRefNum, outputDFRefNum, outputRFRefNum;
unsigned long	outputDFeof, outputRFeof;

void InitFiles(void)
{
	inputRefNum=outputDFRefNum=outputRFRefNum=0;
	outputDFeof=outputRFeof=0L;
	deleteBinHexFile=TRUE;
}

enum ErrorTypes OpenInputFile(void)
{
	OSErr			isHuman;
	HParamBlockRec	paramBlock;
	
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&inputFS, fsRdPerm, &inputRefNum);
	else
		isHuman=HOpen(inputFS.vRefNum, inputFS.parID, inputFS.name,
						fsRdPerm, &inputRefNum);
	if (!isHuman)
	{
		SetFPos(inputRefNum, 1, 0L);
		
		paramBlock.fileParam.ioCompletion=0L;
		paramBlock.fileParam.ioNamePtr=inputFS.name;
		paramBlock.fileParam.ioVRefNum=inputFS.vRefNum;
		paramBlock.fileParam.ioFDirIndex=0;
		paramBlock.fileParam.ioDirID=inputFS.parID;
		PBHGetFInfo(&paramBlock, FALSE);
		theCreationDate=paramBlock.fileParam.ioFlCrDat;
		theModificationDate=paramBlock.fileParam.ioFlMdDat;
	}
	else return kCantOpenInputFile;
}

enum ErrorTypes CreateTempFile(void)
{
	short				i;
	Str255				timeStr;
	OSErr				isHuman;
	enum ErrorTypes		resultCode;
	
	tempFS=outputFS;
	tempFS.name[0]=0x04;
	tempFS.name[1]='H';
	tempFS.name[2]='Q';
	tempFS.name[3]='X';
	tempFS.name[4]='.';
	NumToString(Time&0x7fffffff, timeStr);
	for (i=1; i<=timeStr[0]; i++)
		tempFS.name[++(tempFS.name[0])]=timeStr[i];
	
	if (gHasFSSpecs)
		isHuman=FSpCreate(&tempFS, CREATOR, 'TeMp', smSystemScript);
	else
		isHuman=HCreate(tempFS.vRefNum, tempFS.parID, tempFS.name, CREATOR, 'TeMp');
	
	return (isHuman==noErr) ? allsWell : kCantCreateTempFile;
}

enum ErrorTypes SetupTempFile(void)
{
	OSErr				isHuman;
	
	isHuman=noErr;
	
	if (outputDFeof!=0)
	{
		if (gHasFSSpecs)
			isHuman=FSpOpenDF(&tempFS, fsRdWrPerm, &outputDFRefNum);
		else
			isHuman=HOpen(tempFS.vRefNum, tempFS.parID,
							tempFS.name, fsRdWrPerm, &outputDFRefNum);
	}
	
	if (isHuman!=noErr)
		return kDiskWriteErr;
	
	if (outputRFeof!=0)
	{
		if (gHasFSSpecs)
			isHuman=FSpOpenRF(&tempFS, fsRdWrPerm, &outputRFRefNum);
		else
			isHuman=HOpenRF(tempFS.vRefNum, tempFS.parID,
							tempFS.name, fsRdWrPerm, &outputRFRefNum);
	}
	
	if (isHuman!=noErr)
		return kDiskWriteErr;
	
	if (outputDFeof!=0)
		isHuman=SetEOF(outputDFRefNum, outputDFeof);
	if ((!isHuman) && (outputRFeof!=0))
		isHuman=SetEOF(outputRFRefNum, outputRFeof);

	if (!isHuman)
	{
		FlushVol(0L, tempFS.vRefNum);
		if (outputDFeof!=0)
			SetFPos(outputDFRefNum, 1, 0L);
		if (outputRFeof!=0)
			SetFPos(outputRFRefNum, 1, 0L);
	}
	
	return (isHuman ? kDiskWriteErr : allsWell);
}

void FinalizeFiles(Boolean good)
{
	HParamBlockRec		paramBlock;
	FInfo				theInfo;
	
	if (inputRefNum)
		FSClose(inputRefNum);
	
	if (outputDFRefNum)
		FSClose(outputDFRefNum);
	if (outputRFRefNum)
		FSClose(outputRFRefNum);
	
	FlushVol(0L, tempFS.vRefNum);
	
	if (!good)
	{
		if(gHasFSSpecs)
			FSpDelete(&tempFS);
		else
			HDelete(tempFS.vRefNum, tempFS.parID, tempFS.name);
		
		return;
	}
	
	if (deleteBinHexFile)
	{
		if (gHasFSSpecs)
			FSpDelete(&outputFS);
		else
			HDelete(outputFS.vRefNum, outputFS.parID, outputFS.name);
	}
	FlushVol(0L, tempFS.vRefNum);
	
	
	if (gHasFSSpecs)
		FSpGetFInfo(&tempFS, &theInfo);
	else
		HGetFInfo(tempFS.vRefNum, tempFS.parID, tempFS.name,
					&theInfo);

	Mymemset((Ptr)(&theInfo), 0, sizeof(theInfo));

	theInfo.fdType=theFileType;
	theInfo.fdCreator=theFileCreator;
	theInfo.fdFlags=theFileFlags;
	theInfo.fdFlags&=~0x0100;	/* clear Inited bit */
	
	if (gHasFSSpecs)
		FSpSetFInfo(&tempFS, &theInfo);
	else
		HSetFInfo(tempFS.vRefNum, tempFS.parID, tempFS.name,
					&theInfo);
	FlushVol(0L, tempFS.vRefNum);
	
	paramBlock.fileParam.ioCompletion=0L;
	paramBlock.fileParam.ioNamePtr=tempFS.name;
	paramBlock.fileParam.ioVRefNum=tempFS.vRefNum;
	paramBlock.fileParam.ioFDirIndex=0;
	paramBlock.fileParam.ioDirID=tempFS.parID;
	PBHGetFInfo(&paramBlock, FALSE);
	paramBlock.fileParam.ioFlCrDat=theCreationDate;
	paramBlock.fileParam.ioFlMdDat=theModificationDate;
	PBHSetFInfo(&paramBlock, FALSE);	
	FlushVol(0L, tempFS.vRefNum);	
	
	if (gHasFSSpecs)
		FSpRename(&tempFS, outputFS.name);
	else
		HRename(tempFS.vRefNum, tempFS.parID,
				tempFS.name, outputFS.name);
	
	FlushVol(0L, tempFS.vRefNum);
}

enum ErrorTypes ReadInputFile(short thisFile, Ptr buffer, unsigned long count,
	unsigned long *actualCount)
{
	OSErr			isHuman;
	
	*actualCount=count;
	isHuman=FSRead(thisFile, (long*)actualCount, buffer);
	return ((isHuman==eofErr) || (isHuman==noErr)) ? allsWell : kDiskReadErr;
}

enum ErrorTypes WriteTempFile(short thatFile, Ptr buffer, unsigned long theLength)
{
	return (FSWrite(thatFile, (long*)(&theLength), buffer)==noErr ? allsWell : kDiskWriteErr);
}
