#include "rnmac.h"

#define CR		((unsigned char) 0x0D)
#define LF		((unsigned char) 0x0A)
#define TAB		((unsigned char) 0x09)
#define SPACE 	((unsigned char) 0x20)
#define RLFLAG	((unsigned char) 0x90)

#define HEXTYPE			'TEXT'
#define HEXCREATOR		'????'

#define BUFFSIZE	512

enum { PHASE62, PHASE44, PHASE26 };

enum { PHASE06, PHASE24, PHASE42, PHASE60 };



static char				*gStartOfBinhex = "(This file must be converted with BinHex";
static char				*gBinhexHeader = "(This file must be converted with BinHex 4.0)";
static char				*gEndOfPart = "--- end of part";
static char				gBinHexCodes[256],*gBinHexChars = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
static unsigned char	*gNextHexCharPtr;
static unsigned char	gCurrentReadByte,gCurrentWriteByte;
static unsigned char	gPrevChar,gTheByte;
static int				gChar90Count;
static int				gHexPhase,gLeftIndex,gRightIndex;
static unsigned int		gCRC;
static char				gWriteBuffer[BUFFSIZE],gReadBuffer[BUFFSIZE];
static long				gHexBytesLeftToRead,gReadBufferBytesLeft;
static long				gWriteBufferIndex,gBuffer64Count;
static int				gWriteFRefNum,gReadFRefNum;
static char				gSaveToDisk,gReadFromDisk;
static Handle			gBinhexHandle;
static long				gBinhexHandleSize,gBinhexIndex;
static Ptr				gHexReadPtr;



static int GetNextReadByte(void);
static int ShiftReadBuffer(void);
static void CalcCRC(unsigned char theByte);
static int WriteBinhexBuffer(void);
static int PutByteToBinhexBuffer(void);
static int FlushBinhexBuffer(void);
static int SkipBreak(void);
static int SkipJunkHexData(void);
static int ExtractByteFromHexData(void);
static int GetNextLogicalHexDataByte(char doCRC);
static int FindStartOfHexData(void);
static int FindEndOfHexData(void);
static void SetupBinhexDataArray(void);



static int GetNextReadByte(void)
{
register int	errCode;

	errCode = noErr;
	
	
	if (gHexBytesLeftToRead <= 0)
	{
		sprintf(errorMessage,"GetNextReadByte: early end-of-file encountered");
		errCode = EARLYEOF;
		goto EXITPOINT;
	}
	
	if (gReadBufferBytesLeft <= 0)
	{
		if (gHexBytesLeftToRead > BUFFSIZE)
			gReadBufferBytesLeft = BUFFSIZE;
		else
			gReadBufferBytesLeft = gHexBytesLeftToRead;
		
		if (gReadFromDisk)
		{
			errCode = FSRead(gReadFRefNum,&gReadBufferBytesLeft,(Ptr) gReadBuffer);
			
			if (errCode != noErr)
			{
				sprintf(errorMessage,"GetNextReadByte: FSRead() error %d",errCode);
				goto EXITPOINT;
			}
		}
		
		else
		{
			BlockMove(gHexReadPtr,gReadBuffer,gReadBufferBytesLeft);
			
			gHexReadPtr += gReadBufferBytesLeft;
		}
		
		gNextHexCharPtr = (unsigned char *) gReadBuffer;
	}
	
	
	gCurrentReadByte = *gNextHexCharPtr++;
	gReadBufferBytesLeft--;
	gHexBytesLeftToRead--;


EXITPOINT:
	
	return(errCode);
}





static int ShiftReadBuffer(void)
{
register int	errCode;
long			byteCount;

	errCode = noErr;
	
	if (gHexBytesLeftToRead <= 0)
	{
		sprintf(errorMessage,"ShiftReadBuffer: early end-of-file encountered");
		errCode = EARLYEOF;
		goto EXITPOINT;
	}
	
	if (gReadBufferBytesLeft > 0)
		BlockMove((Ptr) gNextHexCharPtr,(Ptr) gReadBuffer,gReadBufferBytesLeft);
	
	if (gHexBytesLeftToRead > BUFFSIZE - gReadBufferBytesLeft)
		byteCount = BUFFSIZE - gReadBufferBytesLeft;
	else
		byteCount = gHexBytesLeftToRead;
	
	
	if (gReadFromDisk)
	{
		errCode = FSRead(gReadFRefNum,&byteCount,(Ptr) gReadBuffer + gReadBufferBytesLeft);
	
		if (errCode != noErr)
		{
			sprintf(errorMessage,"ShiftReadBuffer: FSRead() error %d",errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		BlockMove(gHexReadPtr,gReadBuffer + gReadBufferBytesLeft,byteCount);
		
		gHexReadPtr += byteCount;
	}
	
	gNextHexCharPtr = (unsigned char *) gReadBuffer;
	gReadBufferBytesLeft += byteCount;
	
EXITPOINT:

	return(errCode);
}





static void CalcCRC(unsigned char theByte)
{
register unsigned int	tempCRC;
register unsigned char	tempByte;

	tempCRC = gCRC;
	tempByte = theByte;
	
	asm
	{
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR1
		
		eor.w	#0x1021,tempCRC
		
	@noXOR1
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR2
		
		eor.w	#0x1021,tempCRC
		
	@noXOR2
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR3
		
		eor.w	#0x1021,tempCRC
		
	@noXOR3
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR4
		
		eor.w	#0x1021,tempCRC
		
	@noXOR4
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR5
		
		eor.w	#0x1021,tempCRC
		
	@noXOR5
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR6
		
		eor.w	#0x1021,tempCRC
		
	@noXOR6
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@noXOR7
		
		eor.w	#0x1021,tempCRC
		
	@noXOR7
		lsl.b	#1,tempByte
		roxl.w	#1,tempCRC
		bcc		@done
		
		eor.w	#0x1021,tempCRC
		
	@done
	}
	
	gCRC = tempCRC;
}





static int WriteBinhexBuffer(void)
{
register int	errCode;

	errCode = noErr;
	
	if (gSaveToDisk)
	{
		errCode = FSWrite(gWriteFRefNum,&gWriteBufferIndex,(Ptr) gWriteBuffer);
		
		if (errCode != noErr)
			sprintf(errorMessage,"WriteBinhexBuffer: FSWrite() error %d",errCode);
	}
	
	else if (gBinhexHandle)
	{
		if (gBinhexIndex + gWriteBufferIndex >= gBinhexHandleSize)
		{
			SetHandleSize(gBinhexHandle,gBinhexIndex + gWriteBufferIndex + BUFFSIZE);
			
			errCode = MemError();
			
			gBinhexHandleSize = GetHandleSize(gBinhexHandle);
			
			if (errCode != noErr || gBinhexIndex + gWriteBufferIndex > gBinhexHandleSize)
			{
				if (errCode == noErr)
					errCode = memFullErr;
				
				sprintf(errorMessage,"WriteBinhexBuffer: SetHandleSize(gBinhexHandle,%ld) error %d",gBinhexIndex + gWriteBufferIndex + BUFFSIZE,errCode);
			}
		}
		
		if (gBinhexIndex + gWriteBufferIndex <= gBinhexHandleSize && errCode == noErr)
		{
			HLock(gBinhexHandle);
			BlockMove(gWriteBuffer,*gBinhexHandle + gBinhexIndex,gWriteBufferIndex);
			HUnlock(gBinhexHandle);
			
			gBinhexIndex += gWriteBufferIndex;
		}
	}


	gWriteBufferIndex = 0;
	
	return(errCode);
}




static int PutByteToBinhexBuffer(void)
{
register int			errCode;
register unsigned char	origByte;

	errCode = noErr;
	origByte = gCurrentWriteByte;
	
	CalcCRC(gCurrentWriteByte);
		
	do
	{
		do
		{
			if (gWriteBufferIndex >= BUFFSIZE - 2)
			{
				errCode = WriteBinhexBuffer();
				
				if (errCode != noErr)
					goto EXITPOINT;
			}
			
			
			if (gHexPhase == PHASE06)
			{
				gLeftIndex = (gCurrentWriteByte >> 2) & 0x003F;
				gRightIndex = gCurrentWriteByte;
				
				gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
				
				gHexPhase = PHASE24;
			}
			
			else if (gHexPhase == PHASE24)
			{
				gLeftIndex = ((gRightIndex << 4) & 0x0030) | ((gCurrentWriteByte >> 4) & 0x000F);
				gRightIndex = gCurrentWriteByte;
				
				gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
				
				gHexPhase = PHASE42;
			}
			
			else if (gHexPhase == PHASE42)
			{
				gLeftIndex = ((gRightIndex << 2) & 0x003C) | ((gCurrentWriteByte >> 6) & 0x0003);
				gRightIndex = gCurrentWriteByte;
				
				gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
				
				gHexPhase = PHASE60;
			}
			
			else if (gHexPhase == PHASE60)
			{
				gLeftIndex = gRightIndex & 0x003F;
				
				gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
				
				gHexPhase = PHASE06;
			}
			
			if (++gBuffer64Count >= 64)
			{
				gWriteBuffer[gWriteBufferIndex++] = '\r';
				gBuffer64Count = 0;
			}
			
		} while (gHexPhase == PHASE06);
		
		if (origByte == RLFLAG)
		{
			if (gCurrentWriteByte == RLFLAG)
				gCurrentWriteByte = 0;
			else
				origByte = 0;
		}
		
	} while (origByte == RLFLAG);
	
EXITPOINT:

	return(errCode);
}





static int FlushBinhexBuffer(void)
{
int		errCode;

	errCode = 0;

	if (gWriteBufferIndex >= BUFFSIZE - 2)
	{
		errCode = WriteBinhexBuffer();
	}
	
	gTheByte = 0;
	
	if (gHexPhase == PHASE24)
	{
		gLeftIndex = ((gRightIndex << 4) & 0x0030) | ((gTheByte >> 4) & 0x000F);
		gRightIndex = gTheByte;
		
		gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
	}
	
	else if (gHexPhase == PHASE42)
	{
		gLeftIndex = ((gRightIndex << 2) & 0x003C) | ((gTheByte >> 6) & 0x0003);
		gRightIndex = gTheByte;
		
		gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
	}
	
	else if (gHexPhase == PHASE60)
	{
		gLeftIndex = gRightIndex & 0x003F;
		
		gWriteBuffer[gWriteBufferIndex++] = gBinHexChars[gLeftIndex];
	}
	
	
	errCode = WriteBinhexBuffer();
	

EXITPOINT:

	return(errCode);
}








static int SkipBreak(void)
{
register int			errCode,dashCount;
register unsigned char 	*s1,*s2;
	
	
	errCode = noErr;
	
	if (gCurrentReadByte == '-')
	{
		if (gReadBufferBytesLeft < strlen(gEndOfPart))
		{
			errCode = ShiftReadBuffer();
			
			if (errCode != noErr)
				goto EXITPOINT;
		}
		
		s1 = (unsigned char *) gEndOfPart + 1L;
		s2 = gNextHexCharPtr;
		
		while (*s1 == *s2 && *s1)
		{
			s1++;
			s2++;
		}
		
		if (*s1 == 0)
		{
			gNextHexCharPtr += strlen(gEndOfPart);
			gReadBufferBytesLeft -= strlen(gEndOfPart);
			gHexBytesLeftToRead -= strlen(gEndOfPart);
			
			if ((errCode = GetNextReadByte()) != noErr)
				goto EXITPOINT;
			
			dashCount = 0;
			
			while (dashCount < 3)
			{
				if (gCurrentReadByte == '-')
					dashCount++;
				else
					dashCount = 0;
				
				if ((errCode = GetNextReadByte()) != noErr)
					goto EXITPOINT;
			}
			
			dashCount = 0;
			
			while (dashCount < 3)
			{
				if (gCurrentReadByte == '-')
					dashCount++;
				else
					dashCount = 0;
				
				if ((errCode = GetNextReadByte()) != noErr)
					goto EXITPOINT;
			}
			
			errCode = SkipJunkHexData();
		}
	}

EXITPOINT:

	return(errCode);
}




static int SkipJunkHexData(void)
{
register int	errCode;
register unsigned char 	ch;

	errCode = noErr;
	
	ch = gCurrentReadByte;
	
	if (ch == CR || ch == LF || ch == SPACE || ch == TAB)
	{
		do
		{
			if ((errCode = GetNextReadByte()) != noErr)
				goto EXITPOINT;
			
			ch = gCurrentReadByte;
			
		} while (ch == CR || ch == LF || ch == SPACE || ch == TAB);
		
		if (gCurrentReadByte == '-')
			errCode = SkipBreak();
	}

EXITPOINT:
	
	return(errCode);
}






static int ExtractByteFromHexData(void)
{
register int		errCode;


	gLeftIndex = gRightIndex;
	
	if ((errCode = GetNextReadByte()) != noErr)
		goto EXITPOINT;
	
	if ((errCode = SkipJunkHexData()) != noErr)
		goto EXITPOINT;
	
	
	if (gCurrentReadByte == ':')
	{
		sprintf(errorMessage,"ExtractByteFromHexData: found early ':' end-of-file marker");
		errCode = EARLYEOF;
		goto EXITPOINT;
	}
	
	
	gRightIndex = gBinHexCodes[gCurrentReadByte];

	if (gRightIndex >= 64)
	{
		sprintf(errorMessage,"ExtractByteFromHexData: found bad binhex character '%c'",gCurrentReadByte);
		errCode = BADHEXCHAR;
		goto EXITPOINT;
	}
	
	
	if (gHexPhase == PHASE62)
	{
		if ((errCode = GetNextReadByte()) != noErr)
			goto EXITPOINT;
		
		if ((errCode = SkipJunkHexData()) != noErr)
			goto EXITPOINT;
		
		gLeftIndex = gRightIndex;
		
		if (gCurrentReadByte == ':')
		{
			sprintf(errorMessage,"ExtractByteFromHexData: found early ':' end-of-file marker");
			errCode = EARLYEOF;
			goto EXITPOINT;
		}
		
		
		gRightIndex = gBinHexCodes[gCurrentReadByte];
	
		if (gRightIndex >= 64)
		{
			sprintf(errorMessage,"ExtractByteFromHexData: found bad binhex character '%c'",gCurrentReadByte);
			errCode = BADHEXCHAR;
			goto EXITPOINT;
		}
		
		gTheByte = ((gLeftIndex << 2) & 0xFC) | ((gRightIndex >> 4) & 0x03);
		
		gHexPhase = PHASE44;
	}
	
	else if (gHexPhase == PHASE44)
	{
		gTheByte = ((gLeftIndex << 4) & 0xF0) | ((gRightIndex >> 2) & 0x0F);
		
		gHexPhase = PHASE26;
	}
	
	else if (gHexPhase == PHASE26)
	{
		gTheByte = ((gLeftIndex << 6) & 0xC0) | (gRightIndex & 0x3F);
		
		gHexPhase = PHASE62;
	}


EXITPOINT:
	
	return(errCode);
}




static int GetNextLogicalHexDataByte(char doCRC)
{
register int		errCode;

	errCode = noErr;
	
	if (gChar90Count > 0)
	{
		gTheByte = gPrevChar;
		gChar90Count--;
		
		if (doCRC)
			CalcCRC(gTheByte);
		else
			CalcCRC(0);
	}
	
	else
	{
		do
		{
			errCode = ExtractByteFromHexData();
			
			if (gTheByte != RLFLAG)
				break;
			
			else
			{
				errCode = ExtractByteFromHexData();
				
				if (errCode != noErr)
					goto EXITPOINT;
				
				if (gTheByte == 0)
				{
					gTheByte = RLFLAG;
					break;
				}
				
				else if (gTheByte >= 2)
				{
					gChar90Count = gTheByte - 2;
					gTheByte = gPrevChar;
					break;
				}
			}
		
		} while (TRUE);
		
		
		gPrevChar = gTheByte;
		
		if (doCRC)
			CalcCRC(gTheByte);
		else
			CalcCRC(0);
	}
	
EXITPOINT:

	return(errCode);
}




static int FindStartOfHexData(void)
{
register int	errCode,i;
char			foundStart;

	errCode = noErr;
	
	foundStart = FALSE;
	
	if ((errCode = GetNextReadByte()) != noErr)
		goto EXITPOINT;
	
	while (!foundStart)
	{
		while (gCurrentReadByte != '(')
		{
			if ((errCode = GetNextReadByte()) != noErr)
				goto EXITPOINT;
		}
		
		i = 0;
		
		while (gStartOfBinhex[i] && gCurrentReadByte == gStartOfBinhex[i])
		{
			if ((errCode = GetNextReadByte()) != noErr)
				goto EXITPOINT;
			
			i++;
		}
		
		
		if (gStartOfBinhex[i] == 0)
		{
			while (gCurrentReadByte != ':')
			{
				if ((errCode = GetNextReadByte()) != noErr)
					goto EXITPOINT;
			}
			
			foundStart = TRUE;
		}
	}
	
	gHexPhase = PHASE62;
	gPrevChar = 0;

EXITPOINT:

	if (errCode == EARLYEOF)
		sprintf(errorMessage,"FindStartOfHexData: couldn't find start of binhex data, '%s'",gBinhexHeader);
	
	return(errCode);
}




static int	FindEndOfHexData(void)
{
register int	errCode;

	errCode = noErr;

	SkipJunkHexData();
	
	if ((errCode = GetNextReadByte()) != noErr)
		goto EXITPOINT;
	
	
	while (gCurrentReadByte == '!')
	{
		SkipJunkHexData();
		
		if ((errCode = GetNextReadByte()) != noErr)
			goto EXITPOINT;
	}
	
	if (gCurrentReadByte != ':')
	{
		sprintf(errorMessage,"FindEndOfHexData: can't find ':' end-of-file marker");
		errCode = BADEOF;
	}

EXITPOINT:

	if (errCode == EARLYEOF)
		sprintf(errorMessage,"FindEndOfHexData: can't find ':' end-of-file marker");
	
	return(errCode);
}



static void SetupBinhexDataArray(void)
{
register int		i;

	for (i=0;i<256;i++)
		gBinHexCodes[i] = 64;
	
	for (i=0;i<64;i++)
		gBinHexCodes[gBinHexChars[i]] = i;
}





int ConvertFromBinhex(char readFromDisk,Ptr theBinhexPtr,long dataLength)
{
register unsigned long	i;
int						errCode;
unsigned int			crc;
unsigned int			theFlags;
unsigned long			dataForkLength,resForkLength;
char					fileName[256];
unsigned char			nameLength;
long					theType,theCreator;
sys67SFReply 			inSys67SFReply,outSys67SFReply;
SFTypeList				theSFTypes;

	gReadFromDisk = readFromDisk;
	
	gReadFRefNum = 0;
	gWriteFRefNum = 0;
	gReadBufferBytesLeft = 0L;
	gHexBytesLeftToRead = 0L;
	gChar90Count = 0;
	gPrevChar = 0;
	
	SetupBinhexDataArray();
	
	
	if (gReadFromDisk)
	{
		theSFTypes[0] = 'TEXT';
		
		errCode = SelectInputFile(&inSys67SFReply,&theSFTypes,1);
		
		if (errCode != noErr)
			goto NOBINHEXERR;
		
		
		errCode = GetDataForkLength(&inSys67SFReply,&gHexBytesLeftToRead);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		
		errCode = OpenDataFork(&inSys67SFReply,&gReadFRefNum);
		
		if (errCode != noErr)
			goto BINHEXERR;
	}
	
	else
	{
		if (theBinhexPtr == 0L)
		{
			sprintf(errorMessage,"ConvertFromBinhex: Bad parameter, theBinhexPtr == 0L");
			errCode = BADPARAM;
			goto BINHEXERR;
		}
		
		gHexReadPtr = theBinhexPtr;
		
		gHexBytesLeftToRead = dataLength;
	}
	
	
	errCode = FindStartOfHexData();
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	gCRC = 0;
	
	errCode = GetNextLogicalHexDataByte(TRUE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	nameLength = gTheByte;
	
	for (i=0;i<nameLength && errCode == noErr;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		fileName[i] = gTheByte;
	}
	
	errCode = GetNextLogicalHexDataByte(TRUE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	if (gTheByte != 0)
	{
		sprintf(errorMessage,"ConvertFromBinhex: can't find 0x00 name terminator in header");
		errCode = NOZERO;
		goto BINHEXERR;
	}
	
	fileName[i] = gTheByte;

	theType = 0L;
	
	for (i=0;i<4 && errCode == noErr;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		theType = (theType << 8) | gTheByte;
	}
	

	theCreator = 0L;
	
	for (i=0;i<4 && errCode == noErr;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		theCreator = (theCreator << 8) | gTheByte;
	}
	
	
	errCode = GetNextLogicalHexDataByte(TRUE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	theFlags = gTheByte;
	
	errCode = GetNextLogicalHexDataByte(TRUE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	theFlags = (theFlags << 8) | gTheByte;
	
	
	dataForkLength = 0L;
	
	for (i=0;i<4 && errCode == noErr;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		dataForkLength = (dataForkLength << 8) | gTheByte;
	}
	
	
	resForkLength = 0L;
	
	for (i=0;i<4 && errCode == noErr;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		resForkLength = (resForkLength << 8) | gTheByte;
	}
	
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = gTheByte;
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = (crc << 8) | gTheByte;
	
	if (crc != gCRC)
	{
		sprintf(errorMessage,"ConvertFromBinhex: bad header CRC 0x%04X <> 0x%04X",crc,gCRC);
		goto BINHEXERR;
	}
	
	
	
	errCode = SelectOutputFile(&outSys67SFReply,fileName);
	
	if (errCode != noErr)
		goto NOBINHEXERR;
	
	
	errCode = OpenAndZeroDataFork(theType,theCreator,&outSys67SFReply,&gWriteFRefNum);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	gWriteBufferIndex = 0;
	gCRC = 0;
	
	for (i=0;i<dataForkLength;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		
		if (gWriteBufferIndex >= BUFFSIZE)
		{
			if (gWriteBufferIndex > 0)
				errCode = FSWrite(gWriteFRefNum,&gWriteBufferIndex,(Ptr) gWriteBuffer);
			
			if (errCode != noErr)
			{
				sprintf(errorMessage,"ConvertFromBinhex: data fork FSWrite() error %d",errCode);
				goto BINHEXERR;
			}
			
			gWriteBufferIndex = 0;
		}
		
		gWriteBuffer[gWriteBufferIndex++] = gTheByte;
	}
	
	
	if (gWriteBufferIndex > 0)
	{
		errCode = FSWrite(gWriteFRefNum,&gWriteBufferIndex,(Ptr) gWriteBuffer);
		
		if (errCode != noErr)
		{
			sprintf(errorMessage,"ConvertFromBinhex: data fork FSWrite() error %d",errCode);
			goto BINHEXERR;
		}
		
		gWriteBufferIndex = 0;
	}
	
	errCode = FSClose(gWriteFRefNum);
	gWriteFRefNum = 0;
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"ConvertFromBinhex: FSClose() error %d",errCode);
		goto BINHEXERR;
	}
	
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = gTheByte;
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = (crc << 8) | gTheByte;
	
	if (crc != gCRC)
	{
		sprintf(errorMessage,"ConvertFromBinhex: bad data fork CRC 0x%04X <> 0x%04X",crc,gCRC);
		goto BINHEXERR;
	}
	
	
	

	errCode = OpenAndZeroResourceFork(theType,theCreator,&outSys67SFReply,&gWriteFRefNum);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	gWriteBufferIndex = 0;
	gCRC = 0;
	
	for (i=0;i<resForkLength;i++)
	{
		errCode = GetNextLogicalHexDataByte(TRUE);
		
		if (errCode != noErr)
			goto BINHEXERR;
		
		
		if (gWriteBufferIndex >= BUFFSIZE)
		{
			if (gWriteBufferIndex > 0)
				errCode = FSWrite(gWriteFRefNum,&gWriteBufferIndex,(Ptr) gWriteBuffer);
			
			if (errCode != noErr)
			{
				sprintf(errorMessage,"ConvertFromBinhex: resource fork FSWrite() error %d",errCode);
				goto BINHEXERR;
			}
			
			gWriteBufferIndex = 0;
		}
		
		gWriteBuffer[gWriteBufferIndex++] = gTheByte;
	}
	
	
	if (gWriteBufferIndex > 0)
	{
		errCode = FSWrite(gWriteFRefNum,&gWriteBufferIndex,(Ptr) gWriteBuffer);
		
		if (errCode != noErr)
		{
			sprintf(errorMessage,"ConvertFromBinhex: resource fork FSWrite() error %d",errCode);
			goto BINHEXERR;
		}
		
		gWriteBufferIndex = 0;
	}
	
	errCode = FSClose(gWriteFRefNum);
	gWriteFRefNum = 0;
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"ConvertFromBinhex: FSClose() error %d",errCode);
		goto BINHEXERR;
	}
	
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = gTheByte;
	
	errCode = GetNextLogicalHexDataByte(FALSE);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	crc = (crc << 8) | gTheByte;
	
	if (crc != gCRC)
	{
		sprintf(errorMessage,"ConvertFromBinhex: bad resource fork CRC 0x%04X <> 0x%04X",crc,gCRC);
		goto BINHEXERR;
	}
	
	errCode = FindEndOfHexData();
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	errCode = SetSFTypeCreatorFlags(&outSys67SFReply,theType,theCreator,theFlags);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	
	if (errCode == noErr)
		goto NOBINHEXERR;
	
	
	
	
BINHEXERR:
	

NOBINHEXERR:

	if (gReadFRefNum)
	{
		if (errCode == noErr)
		{
			errCode = FSClose(gReadFRefNum);
			
			if (errCode != noErr)
			{
				ExtractSFFileName(&inSys67SFReply,fileName);
				
				sprintf(errorMessage,"ConvertFromBinhex: FSClose('%s') error %d",fileName,errCode);
			}
		}
		
		else
			FSClose(gReadFRefNum);
		
		gReadFRefNum = 0;
	}
	
	if (gWriteFRefNum)
	{
		if (errCode == noErr)
		{
			errCode = FSClose(gWriteFRefNum);
			
			if (errCode != noErr)
			{
				ExtractSFFileName(&outSys67SFReply,fileName);
				
				sprintf(errorMessage,"ConvertFromBinhex: FSClose('%s') error %d",fileName,errCode);
			}
		}
		
		else
			FSClose(gWriteFRefNum);
			
		gWriteFRefNum = 0;
	}
	
	if (errCode == USERCANCEL)
		errCode = noErr;
	
	return(errCode);
}




int ConvertToBinhex(char saveToDisk,Handle	*theBinhexHandle)
{
long				i;
long				dataForkLength,resForkLength;
int					errCode;
sys67SFReply 		inSys67SFReply,outSys67SFReply;
unsigned int		crc;
unsigned int		theFlags;
char				fileName[256];
unsigned char		nameLength;
long				theType,theCreator;
SFTypeList			theSFTypes;

	
	gReadFRefNum = 0;
	gWriteFRefNum = 0;
	
	gSaveToDisk = saveToDisk;
	
	gReadFromDisk = TRUE;
	
	if (!saveToDisk)
	{
		if (theBinhexHandle == 0L)
		{
			sprintf(errorMessage,"ConvertToBinhex: Bad parameter, theBinhexHandle == 0L");
			errCode = BADPARAM;
			goto BINHEXERR;
		}
		
		else
		{
			*theBinhexHandle = 0L;
			
			gBinhexHandle = NewHandle(BUFFSIZE);
			
			errCode = MemError();
			
			if (gBinhexHandle == 0L || errCode != noErr)
			{
				if (errCode == noErr)
					errCode = memFullErr;
				
				sprintf(errorMessage,"ConvertToBinhex: NewHandle() error %d",errCode);
						
				goto BINHEXERR;
			}
			
			gBinhexHandleSize = GetHandleSize(gBinhexHandle);
			gBinhexIndex = 0L;
		}
	}

	errCode = SelectInputFile(&inSys67SFReply,&theSFTypes,-1);
	
	if (errCode != noErr)
		goto NOBINHEXERR;
	
	errCode = GetSFTypeCreatorFlags(&inSys67SFReply,&theType,&theCreator,&theFlags);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	ExtractSFFileName(&inSys67SFReply,fileName);
	
	
	if (saveToDisk)
	{
		if (strlen(fileName) >= 27)
			fileName[27] = 0;
		
		mystrncat(fileName,".hqx",32);
		
		errCode = SelectOutputFile(&outSys67SFReply,fileName);
		
		if (errCode != noErr)
			goto NOBINHEXERR;
		
		
		errCode = OpenAndZeroDataFork(HEXTYPE,HEXCREATOR,&outSys67SFReply,&gWriteFRefNum);
		
		if (errCode != noErr)
			goto BINHEXERR;
	}
	
	gWriteBufferIndex = 0;
	
	mystrncpy(gWriteBuffer,gBinhexHeader,BUFFSIZE);
	gWriteBufferIndex = strlen(gWriteBuffer);
	
	gWriteBuffer[gWriteBufferIndex++] = '\r';
	gWriteBuffer[gWriteBufferIndex++] = ':';
	
	errCode = WriteBinhexBuffer();
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	gWriteBufferIndex = 0;
	gCRC = 0;
	gBuffer64Count = 1;
	gHexPhase = PHASE06;
	
	ExtractSFFileName(&inSys67SFReply,fileName);
	nameLength = strlen(fileName);
	
	gCurrentWriteByte = (char) nameLength;
	
	if ((errCode = PutByteToBinhexBuffer()) != noErr)
		goto BINHEXERR;
	
	
	for (i=0;i<=nameLength;i++)
	{
		gCurrentWriteByte = fileName[i];
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
	}
	
	
	errCode = GetSFTypeCreatorFlags(&inSys67SFReply,&theType,&theCreator,&theFlags);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	for (i=0;i<4;i++)
	{
		gCurrentWriteByte = (theType >> 24) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		theType <<= 8;
	}
	
	
	for (i=0;i<4;i++)
	{
		gCurrentWriteByte = (theCreator >> 24) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		theCreator <<= 8;
	}
	
	for (i=0;i<2;i++)
	{
		gCurrentWriteByte = (theFlags >> 8) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		theFlags <<= 8;
	}
	
	
	errCode = GetDataForkLength(&inSys67SFReply,&dataForkLength);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	for (i=0;i<4;i++)
	{
		gCurrentWriteByte = (dataForkLength >> 24) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		dataForkLength <<= 8;
	}
	
	
	errCode = GetResourceForkLength(&inSys67SFReply,&resForkLength);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	for (i=0;i<4;i++)
	{
		gCurrentWriteByte = (resForkLength >> 24) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		resForkLength <<= 8;
	}
	
	CalcCRC(0);
	CalcCRC(0);
	
	crc = gCRC;
	
	for (i=0;i<2;i++)
	{
		gCurrentWriteByte = (crc >> 8) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		crc <<= 8;
	}
	
	
	errCode = GetDataForkLength(&inSys67SFReply,&dataForkLength);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	errCode = OpenDataFork(&inSys67SFReply,&gReadFRefNum);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	gCRC = 0;
	gReadBufferBytesLeft = 0L;
	gHexBytesLeftToRead = dataForkLength;
	
	for (i=0;i<dataForkLength;i++)
	{
		if ((errCode = GetNextReadByte()) != noErr)
			goto BINHEXERR;
		
		gCurrentWriteByte = gCurrentReadByte;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
	}
	
	if ((errCode = FSClose(gReadFRefNum)) != noErr)
	{
		gReadFRefNum = 0;
		sprintf(errorMessage,"ConvertFromBinhex: FSClose() error %d",errCode);
		goto BINHEXERR;
	}
	
	gReadFRefNum = 0;
	
	
	CalcCRC(0);
	CalcCRC(0);
	
	crc = gCRC;
	
	for (i=0;i<2;i++)
	{
		gCurrentWriteByte = (crc >> 8) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		crc <<= 8;
	}
	
	
	errCode = GetResourceForkLength(&inSys67SFReply,&resForkLength);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	errCode = OpenResourceFork(&inSys67SFReply,&gReadFRefNum);
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	gCRC = 0;
	gReadBufferBytesLeft = 0L;
	gHexBytesLeftToRead = resForkLength;
	
	for (i=0;i<resForkLength;i++)
	{
		if ((errCode = GetNextReadByte()) != noErr)
			goto BINHEXERR;
		
		gCurrentWriteByte = gCurrentReadByte;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
	}
	
	
	if ((errCode = FSClose(gReadFRefNum)) != noErr)
	{
		gReadFRefNum = 0;
		sprintf(errorMessage,"ConvertFromBinhex: FSClose() error %d",errCode);
		goto BINHEXERR;
	}
	
	gReadFRefNum = 0;
	
	
	CalcCRC(0);
	CalcCRC(0);
	
	crc = gCRC;
	
	for (i=0;i<2;i++)
	{
		gCurrentWriteByte = (crc >> 8) & 0xFF;
		
		if ((errCode = PutByteToBinhexBuffer()) != noErr)
			goto BINHEXERR;
		
		crc <<= 8;
	}
	
	
	errCode = FlushBinhexBuffer();
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	
	gWriteBufferIndex = 0;
	
	gWriteBuffer[gWriteBufferIndex++] = ':';
	gWriteBuffer[gWriteBufferIndex++] = '\r';
	
	
	errCode = WriteBinhexBuffer();
	
	if (errCode != noErr)
		goto BINHEXERR;
	
	if (errCode == noErr)
		goto NOBINHEXERR;
	
	
	
	
BINHEXERR:
	if (gBinhexHandle)
	{
		DisposHandle(gBinhexHandle);
	}
	
	gBinhexHandle = 0L;
	
	
NOBINHEXERR:
	if (gWriteFRefNum)
	{
		if (errCode == noErr)
		{
			errCode = FSClose(gWriteFRefNum);
			
			if (errCode != noErr)
			{
				ExtractSFFileName(&outSys67SFReply,fileName);
				
				sprintf(errorMessage,"ConvertFromBinhex: FSClose('%s') error %d",fileName,errCode);
			}
		}
		
		else
			FSClose(gWriteFRefNum);
		
		gWriteFRefNum = 0;
	}
	
	if (gReadFRefNum)
	{
		if (errCode == noErr)
		{
			errCode = FSClose(gReadFRefNum);
			
			if (errCode != noErr)
			{
				ExtractSFFileName(&inSys67SFReply,fileName);
				
				sprintf(errorMessage,"ConvertFromBinhex: FSClose('%s') error %d",fileName,errCode);
			}
		}
		
		else
			FSClose(gReadFRefNum);
			
		gReadFRefNum = 0;
	}
	
	if (!saveToDisk && theBinhexHandle)
	{
		SetHandleSize(gBinhexHandle,gBinhexIndex);
		
		*theBinhexHandle = gBinhexHandle;
	}
	
	if (errCode == USERCANCEL)
		errCode = noErr;
	
	return(errCode);
}