/**********************************************************************\

File:		debinhex dispatch.c

Purpose:	This module handles the main dispatch routine for
			deBinHexing.

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

#include "debinhex dispatch.h"
#include "debinhex.h"
#include "file management.h"
#include "error.h"
#include "progress.h"
#include "util.h"
#include "file interface.h"

typedef struct
{
	long		fileType;
	long		fileCreator;
	short		finderFlags;
	long		dataLen;
	long		resLen;
	short		checksum;
} BinHexHeaderStruct;

#define MAX_HEADER_OFFSET	10240

enum ErrorTypes DeBinHexDispatch(void)
{
	enum ErrorTypes		resultCode;
	
	outputFS=inputFS;
	OpenInputFile();
	InitDeBinHex();
	resultCode=GetBinHexHeader();
	if (resultCode!=allsWell)
		return resultCode;
	resultCode=CreateTempFile();
	if (resultCode!=allsWell)
		return resultCode;
	SetupTempFile();
	resultCode=BinHexDecode();
	EndDeBinHex();
	FinalizeFiles(resultCode==allsWell);
	return resultCode;
}

enum ErrorTypes GetBinHexHeader(void)
{
	unsigned char		namelen;
	unsigned char		Buffy[100];
	unsigned char		*theText;
	enum ErrorTypes		resultCode;
	BinHexHeaderStruct	header;
	unsigned long		realLen;
	unsigned char		oneChar;
	short				iter;
	Boolean				notDoneYet;
	short				offset;
	
	theText=(unsigned char*)NewPtrClear(MAX_HEADER_OFFSET);
	Mymemcpy((Ptr)Buffy, (Ptr)((long)(*GetString(BINHEX_HEADER_STRING))+1), 47);
	iter=1;
	notDoneYet=TRUE;
	resultCode=ReadInputFile(inputRefNum, (Ptr)theText, 10240L, &realLen);
	if (resultCode!=allsWell)
		return resultCode;
	offset=0;
	do
	{
		oneChar=*((unsigned char*)((long)theText+(offset++)));
		if (offset==MAX_HEADER_OFFSET)
		{
			DisposePtr((Ptr)theText);
			return kBinHexErr;
		}
		if (oneChar==Buffy[iter])
		{
			iter++;
			notDoneYet=(iter<41);
		}
		else
		{
			iter=0;
			if (oneChar==Buffy[iter])
				iter++;
		}
	}
	while (notDoneYet);
	
	notDoneYet=TRUE;
	do
	{
		oneChar=*((unsigned char*)((long)theText+(offset++)));
		if (offset==MAX_HEADER_OFFSET)
		{
			DisposePtr((Ptr)theText);
			return kBinHexErr;
		}
		if ((oneChar==0x0a) || (oneChar==0x0d))
			notDoneYet=FALSE;
	}
	while (notDoneYet);
	
	oneChar=*((unsigned char*)((long)theText+(offset++)));
	if (offset==MAX_HEADER_OFFSET)
	{
		DisposePtr((Ptr)theText);
		return kBinHexErr;
	}
	notDoneYet=((oneChar==0x09) || (oneChar==0x0a) || (oneChar==0x0d) ||
		(oneChar==0x20));

	while (notDoneYet)
	{
		oneChar=*((unsigned char*)((long)theText+(offset++)));
		if (offset==MAX_HEADER_OFFSET)
		{
			DisposePtr((Ptr)theText);
			return kBinHexErr;
		}
		notDoneYet=((oneChar==0x09) || (oneChar==0x0a) || (oneChar==0x0d) ||
			(oneChar==0x20));
	}
	
	DisposePtr((Ptr)theText);
	
	if (oneChar!=':')
		return kBinHexErr;
	
	SetFPos(inputRefNum, 1, offset);
	resultCode=GetNBytes(inputRefNum, (Ptr)Buffy, 1);
	if (resultCode!=allsWell)
		return resultCode;
	namelen=Buffy[0];
	resultCode=GetNBytes(inputRefNum, (Ptr)((long)Buffy+1), namelen+21);
	if (resultCode!=allsWell)
		return resultCode;
	Mymemcpy((Ptr)(&header), (Ptr)((long)Buffy+namelen+2), sizeof(header));
	*((char*)((long)Buffy+namelen+20))=0x00;
	*((char*)((long)Buffy+namelen+21))=0x00;
	if (header.checksum!=BinHexChecksum((Ptr)Buffy, namelen+22, TRUE))
		return kBinHexErr;
	Mymemcpy((Ptr)outputFS.name, (Ptr)Buffy, namelen+1);
	outputDFeof=header.dataLen;
	outputRFeof=header.resLen;
	theFileType=header.fileType;
	theFileCreator=header.fileCreator;
	theFileFlags=header.finderFlags;
	
	return allsWell;
}

enum ErrorTypes BinHexDecode(void)
{
	Ptr				outputBuffer;
	enum ErrorTypes	resultCode;
	unsigned long	bufferLen;
	Boolean			notDoneYet;
	unsigned long	curPos, len, forkPos;
	Boolean			useDF;
	DialogPtr		theDlog;
	unsigned short	oldcheck;
	unsigned char	temp1,temp2;
	
	theDlog=OpenProgressDialog(outputDFeof+outputRFeof+4, "\pDeBinHex");
	if (theDlog==0L)
		return kNoMemory;
	SetProgressText("\pConverting ",inputFS.name, "\p from BinHex format...","\p");
	DealWithOtherPeople();
	bufferLen=BINHEX_BUFFER_LENGTH;
	outputBuffer=NewPtr(bufferLen);
	if (outputBuffer==0L)
		return kNoMemory;
	curPos=0L;
	forkPos=0L;
	notDoneYet=TRUE;
	useDF=TRUE;
	resultCode=allsWell;
	BinHexChecksum(outputBuffer, 0, TRUE);
	
	do
	{
		if (useDF)
			len=(outputDFeof+2-forkPos<bufferLen) ?
				outputDFeof+2-forkPos : bufferLen;
		else
			len=(outputRFeof+2-forkPos<bufferLen) ?
				outputRFeof+2-forkPos : bufferLen;
		
		resultCode=GetNBytes(inputRefNum, outputBuffer, len);

		if (resultCode==allsWell)
		{
			if (len!=bufferLen)
			{
				if (useDF)
				{
					temp1=*((unsigned char*)((long)outputBuffer+outputDFeof-forkPos));
					temp2=*((unsigned char*)((long)outputBuffer+outputDFeof-forkPos+1));
					oldcheck=0;
					oldcheck+=temp1;
					oldcheck<<=8;
					oldcheck+=temp2;
					*((unsigned char*)((long)outputBuffer+outputDFeof-forkPos))=0x00;
					*((unsigned char*)((long)outputBuffer+outputDFeof-forkPos+1))=0x00;
					if (oldcheck!=BinHexChecksum(outputBuffer, len, FALSE))
						resultCode=kBinHexErr;
					if ((resultCode==allsWell) && (len>2))
						resultCode=WriteTempFile(outputDFRefNum, outputBuffer, len-2);
					useDF=FALSE;
					forkPos=0L;
					BinHexChecksum(outputBuffer, 0, TRUE);
				}
				else
				{
					temp1=*((unsigned char*)((long)outputBuffer+outputRFeof-forkPos));
					temp2=*((unsigned char*)((long)outputBuffer+outputRFeof-forkPos+1));
					oldcheck=0;
					oldcheck+=temp1;
					oldcheck<<=8;
					oldcheck+=temp2;
					*((unsigned char*)((long)outputBuffer+outputRFeof-forkPos))=0x00;
					*((unsigned char*)((long)outputBuffer+outputRFeof-forkPos+1))=0x00;
					if (oldcheck!=BinHexChecksum(outputBuffer, len, FALSE))
						resultCode=kBinHexErr;
					if ((resultCode==allsWell) && (len>2))
						resultCode=WriteTempFile(outputRFRefNum, outputBuffer, len-2);
					notDoneYet=FALSE;
				}
			}
			else
			{
				forkPos+=len;
				BinHexChecksum(outputBuffer, len, FALSE);
				if (useDF)
					resultCode=WriteTempFile(outputDFRefNum, outputBuffer, len);
				else
					resultCode=WriteTempFile(outputRFRefNum, outputBuffer, len);
			}
			if (resultCode==allsWell)
			{
				curPos+=len;
				UpdateProgressDialog(curPos);
				if (!DealWithOtherPeople())
					resultCode=userCancelErr;
			}
		}
	}
	while ((notDoneYet) && (resultCode==allsWell));

	DisposePtr(outputBuffer);
	DismissProgressDialog();
	
	return resultCode;
}
