/**********************************************************************\

File:		debinhex.c

Purpose:	This module handles all the blood and guts of actual
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

#include "file management.h"
#include "debinhex dispatch.h"
#include "debinhex.h"
#include "util.h"

/* all these globals are internal to debinhex.c */
unsigned char	bstr[64];			/* table of binhex characters with 6-bit index */
unsigned short	crc;				/* binhex checksum */
unsigned char	bufferChar;			/* last char read */
unsigned char	runLenLength;		/* run-len encoding current length */
char			slop;				/* bits we haven't dealt with yet */
short			phase;				/* how many bits we need */

Ptr				dbhRaw;				/* raw binhexed data, read in from disk */
unsigned long	dbhSize;			/* actual size of dbhRaw buffer */
unsigned long	dbhPos;				/* where we are in dbhRaw in processing */
unsigned long	dbhTotal;			/* total # of bytes in dbhRaw that contain data */
unsigned char	dbhValues[256];		/* table of values for debinhexing */
Boolean			lookingForRunLen;	/* whether we can handle run-len right now */
Boolean			nowGettingRunLen;	/* whether we are getting a run-len length byte now */

enum
{
	no_bits = 0,
	two_bits,
	four_bits,
	six_bits
};

void InitDeBinHex(void)
{
	short			i;
	
	Mymemcpy((Ptr)bstr, (Ptr)((long)(*GetString(BINHEX_VALUES_STRING))+1), 64);
	Mymemset((Ptr)dbhValues, 0xff, 256); /* init all values to bad ones */
	for (i=0; i<64; i++)
		dbhValues[bstr[i]]=i; /* put in valid binhex values */
	dbhValues[9]=dbhValues[10]=dbhValues[13]=dbhValues[23]=0x80; /* whitespace */
	
	dbhSize=BINHEX_BUFFER_LENGTH;
	dbhRaw=NewPtr(dbhSize);
	dbhPos=-1L;					/* need more binhex data from disk */
	lookingForRunLen=FALSE;
	nowGettingRunLen=FALSE;
	phase=no_bits;
	crc=0;
	runLenLength=0x00;
}

void EndDeBinHex(void)
{
	DisposePtr(dbhRaw);
}

enum ErrorTypes GetNBytes(short inputRefNum, Ptr outBuf, unsigned long n)
{
	unsigned long	off;
	enum ErrorTypes	resultCode;
	
	off=0L;
	while (off<n)
	{
		if (dbhPos==-1L)
		{
			resultCode=ReadInputFile(inputRefNum, dbhRaw, dbhSize, &dbhTotal);
			if (resultCode!=allsWell)
				return resultCode;
			dbhPos=0L;
		}
		resultCode=GetByteFromBuffer((unsigned char*)((long)outBuf+off));
		if (resultCode==allsWell)
			off++;
		else if ((resultCode==kBinHexErr) && (off==n-1))
			resultCode=FlushBuffer((unsigned char*)((long)outBuf+(off++)));
		if (resultCode==kBinHexErr)
			return resultCode;
		if (resultCode==kNeedMoreBinHexErr)
			dbhPos=-1L;
	}
	return allsWell;
}

enum ErrorTypes FlushBuffer(unsigned char *outputChar)
{
	*outputChar=bufferChar;
	if (runLenLength!=0x00)
		runLenLength--;
	return allsWell;
}

enum ErrorTypes GetByteFromBuffer(unsigned char *outputChar)
{
	unsigned char		thisByte;
	enum ErrorTypes		resultCode;
	
	if (runLenLength!=0x00)
	{
		*outputChar=bufferChar;
		runLenLength--;
		return allsWell;
	}

	if (!lookingForRunLen)
	{
		if (!nowGettingRunLen)
		{
			resultCode=ExtractByteFromBinHex(&thisByte);
			if (resultCode!=allsWell)
				return resultCode;
		}
		else
			thisByte=0x90;
		
		if (thisByte==0x90)
		{
			nowGettingRunLen=TRUE;
			resultCode=ExtractByteFromBinHex(&runLenLength);
			if (resultCode!=allsWell)
				return resultCode;
			nowGettingRunLen=FALSE;
		}
		bufferChar=thisByte;
		lookingForRunLen=TRUE;
	}

	if (!nowGettingRunLen)
	{
		resultCode=ExtractByteFromBinHex(&thisByte);
		if (resultCode!=allsWell)
			return resultCode;
	}
	else
		thisByte=0x90;
	
	if (thisByte==0x90)
	{
		nowGettingRunLen=TRUE;
		resultCode=ExtractByteFromBinHex(&runLenLength);
		if (resultCode!=allsWell)
			return resultCode;
		nowGettingRunLen=FALSE;
		if (runLenLength!=0x00)
		{
			lookingForRunLen=FALSE;
			*outputChar=bufferChar;
			runLenLength--;
			return allsWell;
		}
		else
		{
			*outputChar=bufferChar;
			bufferChar=thisByte;
			return allsWell;
		}
	}
	else
	{
		*outputChar=bufferChar;
		bufferChar=thisByte;
		return allsWell;
	}
}

enum ErrorTypes ExtractByteFromBinHex(unsigned char *outputChar)
{
	unsigned char	thisChar;
	
	while (1)
	{
		if (dbhPos==dbhTotal)
			return kNeedMoreBinHexErr;
		thisChar=dbhValues[*((unsigned char*)((long)dbhRaw+(dbhPos++)))];
		if (thisChar==0xff) /* invalid binhex */
			return kBinHexErr;
		if (thisChar!=0x80) /* only process non-whitespace */
		{
			switch (phase)
			{
				case no_bits:
					slop=thisChar&0x3f;
					phase=two_bits;
					break;
				case two_bits:
					*outputChar=(slop<<2)|(thisChar&0x30)>>4;
					slop=thisChar&0x0f;
					phase=four_bits;
					return allsWell;
					break;
				case four_bits:
					*outputChar=(slop<<4)|(thisChar&0x3c)>>2;
					slop=thisChar&0x03;
					phase=six_bits;
					return allsWell;
					break;
				case six_bits:
					*outputChar=(slop<<6)|(thisChar&0x3f);
					phase=no_bits;
					return allsWell;
			}
		}
	}
}

short BinHexChecksum(Ptr buffer, unsigned long len, Boolean StartOver)
{
	unsigned long	off;
	
	if (StartOver)
		crc=0;
	
	off=0L;
	while (off<len)
		CalcCRC(&crc, *((unsigned char*)((long)buffer+(off++))));

	return crc;
}

void CalcCRC(unsigned short *crc, unsigned short v)
{
	short			i;
	Boolean		temp;
	
	for (i=1; i<=8; i++)
	{
		temp=((*crc)&0x8000)!=0;
		*crc=((*crc)<<1)|((v>>7)&0x0001);
		if (temp)
			(*crc)=(*crc)^0x1021;
		v=(v<<1)&0x00ff;
	}
}
