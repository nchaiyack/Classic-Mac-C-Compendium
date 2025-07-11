#define FILE_NUM 17
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions to convert files to binhex and back again
 * much of this code is patterned after xbin by Dave Johnson, Brown University
 * Some is lifted straight from xbin.  Our thanks to Dave & Brown.
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment HexBin

/************************************************************************
 * Declarations for private routines
 ************************************************************************/
void HexBinInputChar(Byte c,long estMessageSize);
void HexBinDataChar(short d,long estMessageSize);
int FoundHexBin(void);
int HexBinDecode(Byte c,long estMessageSize);
void AbortHexBin(Boolean error);
void OpenDataFork(void);
int ForkRoll(void);
int FlushBuffer(void);
void ResetHexBin(void);
void comp_q_crc(unsigned short c);
void CrcError(void);

/************************************************************************
 * Private globals
 ************************************************************************/
typedef struct
{
	long type;
	long author;
	short flags;
	long dataLength;
	long rzLength;
	unsigned short hCrc;
} HexBinHead;
typedef struct
{
	Str63 flagString;
	short state;
	long oSpot;
	UHandle buffer;
	long bSize;
	long bSpot;
	short refN;
	short vRefN;
	Str63 name;
	Byte lastData;
	Byte state68;
	Byte b8;
	Byte runCount;
	Boolean run;
	long count;
	long size;
	union
	{
		HexBinHead bxHead;
		Byte bxhBytes[sizeof(HexBinHead)];
	} BHHUnion;
	unsigned long calcCrc;
	unsigned long crc;
} HexBinGlobals, *HBGPtr, **HBGHandle;
HexBinGlobals **HBG;
#define State (*HBG)->state
#define OSpot (*HBG)->oSpot
#define Buffer (*HBG)->buffer
#define BSize (*HBG)->bSize
#define BSpot (*HBG)->bSpot
#define RefN (*HBG)->refN
#define VRefN (*HBG)->vRefN
#define Name (*HBG)->name
#define Type (*HBG)->BHHUnion.bxHead.type
#define Author (*HBG)->BHHUnion.bxHead.author
#define Flags (*HBG)->BHHUnion.bxHead.flags
#define RzLength (*HBG)->BHHUnion.bxHead.rzLength
#define DataLength (*HBG)->BHHUnion.bxHead.dataLength
#define HCrc (*HBG)->BHHUnion.bxHead.hCrc
#define BxhBytes (*HBG)->BHHUnion.bxhBytes
#define LastData (*HBG)->lastData
#define State68 (*HBG)->state68
#define B8 (*HBG)->b8
#define RunCount (*HBG)->runCount
#define Run (*HBG)->run
#define Count (*HBG)->count
#define Size (*HBG)->size
#define CalcCrc (*HBG)->calcCrc
#define Crc (*HBG)->crc

#define RUNCHAR 0x90

#define DONE 0x7F
#define SKIP 0x7E
#define FAIL 0x7D

Byte HexBinTable[256] = {
/* 0*/	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/*							\t   \n								\r								*/
				FAIL, SKIP, SKIP, FAIL, FAIL, SKIP, FAIL, FAIL,
/*      ' '																							*/
/* 2*/	SKIP, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/* 4*/	FAIL, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
				0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, FAIL, FAIL,
/* 6*/	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, FAIL,
/*									: 																	*/
				0x14, 0x15, DONE, FAIL, FAIL, FAIL, FAIL, FAIL,
/* 8*/	0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
				0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, FAIL,
/*10*/	0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, FAIL,
				0x2C, 0x2D, 0x2E, 0x2F, FAIL, FAIL, FAIL, FAIL,
/*12*/	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, FAIL,
				0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, FAIL, FAIL,
/*14*/	0x3D, 0x3E, 0x3F, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
/*16*/	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
				FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
};
	
#pragma segment POP
/************************************************************************
 * SaveHexBin - save a binhex file, if one is found.	Returns the 
 * state of the converter
 ************************************************************************/
short SaveHexBin(UPtr text,long size,long estMessageSize)
{
	if (HBG==nil)
		return(HexDone);		/* if we have no globals, return text unchanged */
	
	if (State==HexDone)
		State = NotHex; 		/* start the conversion */
		
	for (Count=0;Count<size;Count++)
		HexBinInputChar(text[Count],estMessageSize);
	
	return(State);
}

/************************************************************************
 * EndHexBin - done with a HexBin session
 ************************************************************************/
void EndHexBin(void)
{
	if (HBG)
	{
		if (VRefN && !CommandPeriod) {WarnUser(BINHEX_SHORT,0);BadBinHex=True;}
		AbortHexBin(False);
		DisposHandle(HBG);
		HBG = nil;
	}
	return;
}

/************************************************************************
 * BeginHexBin - ready to begin a HexBin session
 ************************************************************************/
void BeginHexBin(void)
{
	HBG = NewH(HexBinGlobals);
	if (HBG)
		WriteZero(*HBG,sizeof(HexBinGlobals));
}
#pragma segment HexBin

/************************************************************************
 * HexBinInputChar - read a char from the binhex data, decode it, and
 * let HexBinDataChar do (most of) the rest.
 ************************************************************************/
void HexBinInputChar(Byte c,long estMessageSize)
{
	short d;

reSwitch:
	switch (State)
	{
		case HexDone:
			break;
			
		case NotHex:
			if (c==':') State=FoundHexBin();
			break;

		case Excess:
			c = HexBinTable[c];
			if (c==DONE)
			{
				State = HexDone;
				if (BSpot>3)
				{
					WarnUser(BINHEXEXCESS,BSpot-1);
					BadBinHex = True;
				}
				PopProgress(True);
			}
			else if (c!=SKIP)
				BSpot++;
			break;
	
		case CollectName:
			(*Buffer)[BSpot++] = c;
			/* fall-throught to default */
		default:
			if ((d=HexBinDecode(c,estMessageSize))>=0)
				HexBinDataChar(d,estMessageSize);
			break;
	}
}

/************************************************************************
 * HexBinDataChar - the main engine for the de-binhexer
 * Unfortunately, much of the real work happens as side-effects to functions.
 * State and BSpot are almost always manipulated directly in this routine, but
 * the rest of HBG is up for grabs.
 ************************************************************************/
void HexBinDataChar(short d,long estMessageSize)
{
reSwitch:
	switch (State)
	{
		case CollectName:
			comp_q_crc(d);
			Name[OSpot] = d;
			if (Name[OSpot]==0)
			{
				State = CollectInfo;
				BSpot = 0;
				Name[0] = MIN(Name[0],31);
				Name[32] = 0;
			}
			else
			{
				if (++OSpot>sizeof(Name)-2)
				{
					WarnUser(BAD_HEXBIN_FORMAT,State);
					AbortHexBin(True);
				}
			}
			break;
			
		case CollectInfo:
			BxhBytes[BSpot++] = d;
			switch (sizeof(HexBinHead)-BSpot)
			{
				case 0:
					{
						Str31 name;
						short vRef;
						PCopy(name,Name);
						/*
						 * Note:	The length test can't be 100% correct because of
						 * run-length encoding.  I therefore give it some slop and
						 * hope for the best.  Eudora doesn't produce RLE, and neither
						 * does StuffIt, so perhaps this won't be a problem.  It's
						 * better than the alternative, anyway...
						 */
						if ((estMessageSize<GetRLong(HEX_SIZE_THRESH) ||
								DataLength+RzLength < (estMessageSize*100)/GetRLong(HEX_SIZE_PERCENT)) &&
								(AutoWantTheFile(name,&vRef,False) || WantTheFile(name,&vRef)))
						{
							VRefN = vRef;
							PCopy(Name,name);
							Crc = HCrc;
							BSpot = 0;
							CrcError();
							State = DataWrite;
							OpenDataFork();
						}
						else
							AbortHexBin(False);
					}
					break;
				case 1:
					break;
				default:
					comp_q_crc(d);
					break;
			}
			break;
			
		case DataWrite:
		case RzWrite:
			if (OSpot==0)
			{
				State++;
				goto reSwitch;
			}
			else
			{
				(*Buffer)[BSpot++] = d;
				comp_q_crc(d);
				OSpot--;
				if (BSpot==BSize)
				{
					if (FlushBuffer()) AbortHexBin(True);
					BSpot = 0;
				}
			}
			break;
		case DataCrc1:
		case RzCrc1:
			Crc = d << 8;
			State++;
			break;
		case DataCrc2:
		case RzCrc2:
			State++;
			Crc = Crc | d;
			State = ForkRoll();
			BSpot = 0;
			break;
	}
}

/************************************************************************
 * FoundHexBin - the second level of initialization.
 * This function is called when we have detected a HexBin file.  Its
 * major purpose is to allocate a buffer for data
 * If it has allocated a buffer, it continues the HexBin process by
 * returning the next state, CollectHeader; otherwise, it returns HexDone,
 * effectively aborting the HexBin process.
 ************************************************************************/
int FoundHexBin(void)
{
	BSize = GetRLong(BUFFER_SIZE);
	if (!Buffer) Buffer = NuHandle(BSize);
	if (!Buffer)
	{
		WarnUser(BINHEX_MEM,MemError());
		BadBinHex = True;
		return(HexDone);
	}
	else
	{
		ResetHexBin();
		return(CollectName);
	}
}

/************************************************************************
 * HexBinDecode - decode a data byte.
 * The binhex data format encodes 3 data bytes into four encoded bytes.
 * There are some "magic" values for encoded bytes:
 *			newline, cr 		ignore
 *			: 							end of binhex data
 * There is also a magic data byte, 0x90.  This repeats the PREVIOUS
 * data byte n times, where n is the value of the NEXT data byte.
 * If n is zero, 0x90 itself is output.
 ************************************************************************/
int HexBinDecode(Byte c,long estMessageSize)
{
	Byte b6;
	short data;
		
	if ((b6=HexBinTable[c])>64)
	{
		switch (b6)
		{
			case SKIP:
				return(-1);
			case DONE:
				WarnUser(BINHEX_SHORT,0);
				AbortHexBin(True);
				return(-1);
			default:
				if (!PrefIsSet(PREF_HEX_PERMISSIVE))
				{
					WarnUser(BINHEX_BADCHAR,c);
					AbortHexBin(True);
				}
				return(-1);
		}
	}
	else
	{
		switch(State68++)
		{
			case 0:
				B8 = b6<<2;
				return(-1);
			case 1:
				data = B8 | (b6 >> 4);
				B8 = (b6 & 0xf) << 4;
				break;
			case 2:
				data = B8 | (b6>>2);
				B8 = (b6 & 0x3) << 6;
				break;
			case 3:
				data = B8 | b6;
				State68 = 0;
				break;
		}
		if (!Run)
		{
			if (data == RUNCHAR)
			{
				Run = 1;
				RunCount = 0;
				return(-1);
			}
			else
				return(LastData = data);
		}
		else
		{
			Run = False;
			if (!data)
				return(LastData = RUNCHAR);
			while (--data > 0) HexBinDataChar(LastData,estMessageSize);
			return(-1);
		}
	}
}


/************************************************************************
 * AbortHexBin - something bad has happened.	Get rid of loose ends.
 ************************************************************************/
void AbortHexBin(Boolean error)
{
	if (Buffer) {DisposHandle(Buffer); Buffer=0;}
	if (RefN) {FSClose(RefN); RefN=0;}
	if (VRefN)
	{
		FSDelete(Name,VRefN);
		VRefN = 0;
	}
	State = HexDone;
	BadBinHex = BadBinHex || error;
	PopProgress(True);
}

/************************************************************************
 * OpenDataFork - open the data fork of the file
 ************************************************************************/
void OpenDataFork(void)
{
	int err;
	short refN;
	FInfo info;
	
	LDRef(HBG);
	if (err=MakeResFile(Name,VRefN,0,Author,Type))
	{
		FileSystemError(BINHEX_CREATE,Name,err);
		VRefN = 0;
		AbortHexBin(True);
	}
	else if (err=GetFInfo(Name,VRefN,&info))
	{
		FileSystemError(BINHEX_CREATE,Name,err);
		AbortHexBin(True);
	}
	else
	{
		info.fdFlags = Flags;
		info.fdFlags &= ~fOnDesk;
		info.fdFlags &= ~fInvisible;
		info.fdFlags &= ~fInited;
		if (err=SetFInfo(Name,VRefN,&info))
		{
			FileSystemError(BINHEX_OPEN,Name,err);
			AbortHexBin(True);
		}
		else if (err=FSOpen(Name,VRefN,&refN))
		{
			FileSystemError(BINHEX_OPEN,Name,err);
			AbortHexBin(True);
		}
		else
			RefN = refN;
	}
	OSpot = DataLength;
	UL(HBG);
}

/************************************************************************
 * ForkRoll - roll from the data fork to the resource fork, or else from
 * the resource fork to cleanup.	Returns the next state.
 ************************************************************************/
int ForkRoll(void)
{
	int err=0;
	short refN;
	long pos;
	
	/*
	 * flush and close the current file
	 */
	if (RefN)
	{
		CrcError();
		if (err=FlushBuffer())
			AbortHexBin(True);
		else
		{
			if (!GetFPos(RefN,&pos)) SetEOF(RefN,pos);
			if (err=FSClose(RefN))
			{
				AbortHexBin(True);
				LDRef(HBG);
				FileSystemError(BINHEX_WRITE,Name,err);
				UL(HBG);
			}
		}
	}
	if (err) return(HexDone);
	
	/*
	 * open the new one if necessary
	 */
	if (State==RzWrite)
	{
		LDRef(HBG);
		if (err=OpenRF(Name,VRefN,&refN))
		{
			FileSystemError(BINHEX_OPEN,Name,err);
			AbortHexBin(True);
		}
		else
			RefN = refN;
		UL(HBG);
		OSpot = RzLength;
		return(err ? HexDone : RzWrite);
	}
	else
	{
		Str31 fileName;
		PCopy(fileName,Name);
		AddAttachNote(VRefN,fileName,Author,Type);
		FlushVol(nil,VRefN);
		VRefN = 0;
		if (Buffer) {DisposHandle(Buffer); Buffer=0;}
		return(Excess);
	}
}

/************************************************************************
 * FlushBuffer - write out what we have so far
 ************************************************************************/
int FlushBuffer(void)
{
	long writeBytes = BSpot;
	int err;
	
	if (err=FSWrite(RefN,&writeBytes,LDRef(Buffer)))
		FileSystemError(BINHEX_WRITE,Name,err);
	UL(Buffer);
	BSpot = 0;
	return(err);
}

/************************************************************************
 * WantTheFile - see if the user really wants the file
 ************************************************************************/
Boolean WantTheFile(UPtr name,short *vRef)
{
	SFReply sfr;
	Point where;
	Str63 prompt;
	Str63 dfName;
	Str255 message;
	
	MommyMommy(ATTENTION,nil);
	if (InBG) return(AutoWantTheFile(name,vRef,True));
	StdFileSpot(&where,SFPUTFILE_ID);
	GetRString(prompt,BINHEX_PROMPT);
	BlockMove(name,dfName,*name+1);
	PushProgress();
	GetRString(message,BINHEX_HELP);
	Progress(NoChange,message);
	AlertTicks=GetRLong(ALERT_TIMEOUT)*60+TickCount();
	SFPPutFile(where,prompt,dfName,nil,&sfr,SFPUTFILE_ID,DlgFilter);
	AlertTicks = 0;
	if (!sfr.good) return (False);
	BlockMove(sfr.fName,name,*sfr.fName+1);
	*vRef = sfr.vRefNum;
	ComposeRString(message,BINHEX_PROG_FMT,sfr.fName);
	ByteProgress(message,0,4*(RzLength+DataLength)/3);
	return(True); 
}

/************************************************************************
 * AutoWantTheFile - see if we can auto-receive the file
 ************************************************************************/
Boolean AutoWantTheFile(UPtr name,short *vRef,Boolean ohYesYouDo)
{
	Str31 dfName;
	long dirId;
	Str31 volName;
	Str63 volPath;
	Str15 iAscii;
	short i,wdRef;
	HFileInfo hfi;
	Str63 message;
	
	if (ohYesYouDo)
	{
		dirId = 2;
		GetMyVolName(MyVRef,volName);
	}
	else
	{
		if (!PrefIsSet(PREF_AUTOHEX)) return(False);
		if (!GetVolpath(GetPref(volPath,PREF_AUTOHEX_VOLPATH),volName,&dirId))
			return(False);
	}
	PCopy(dfName,name);
	SetVol(volName,0);
	wdRef = GetMyWD(0,dirId);
	for (i=1;i<5000 && !HGetFileInfo(wdRef,dirId,dfName,&hfi);i++)
	{
		PCopy(dfName,name);
		*iAscii = 0;
		PLCat(iAscii,i);
		if (*dfName + *iAscii < 31)
			PCat(dfName,iAscii);
		else
		{
			BlockMove(iAscii+1,dfName+32-*iAscii,*iAscii);
			*dfName = 31;
		}
	}
	if (i==5000) return(False);
	PCopy(name,dfName);
	*vRef = wdRef;
	ComposeRString(message,BINHEX_PROG_FMT,dfName);
	ByteProgress(message,0,4*(RzLength+DataLength)/3);
	return(True); 
}

/************************************************************************
 * ResetHexBin - ready ourselves for a new HexBin file.
 ************************************************************************/
void ResetHexBin(void)
{
	OSpot = 0;
	BSpot = 0;
	RefN = 0;
	VRefN = 0;
	*Name = 0;
	Crc = 0;
	LastData = 0;
	State68 = 0;
	B8 = 0;
	Run = False;
	CalcCrc = 0;
}

/************************************************************************
 * comp_q_crc - lifted from xbin
 ************************************************************************/
#define BYTEMASK 0xff
#define BYTEBIT 0x100
#define WORDMASK 0xffff
#define WORDBIT 0x10000
#define CRCCONSTANT 0x1021

void comp_q_crc(unsigned short c)
{
				register unsigned long temp = CalcCrc;

/* Never mind why I call it WOP... */
#define WOP { \
								c <<= 1; \
								if ((temp <<= 1) & WORDBIT) \
												temp = (temp & WORDMASK) ^ CRCCONSTANT; \
								temp ^= (c >> 8); \
								c &= BYTEMASK; \
				}
				WOP;
				WOP;
				WOP;
				WOP;
				WOP;
				WOP;
				WOP;
				WOP;
				CalcCrc = temp;
}

/************************************************************************
 * CrcError - does the computed crc (CalcCrc) match the given crc (Crc)
 ************************************************************************/
void CrcError(void)
{
	comp_q_crc(0);
	comp_q_crc(0);
	if ((Crc&WORDMASK) != (CalcCrc&WORDMASK))
	{
		WarnUser(CRC_ERROR,CalcCrc);
		BadBinHex = True;
	}
	CalcCrc = 0;
}

