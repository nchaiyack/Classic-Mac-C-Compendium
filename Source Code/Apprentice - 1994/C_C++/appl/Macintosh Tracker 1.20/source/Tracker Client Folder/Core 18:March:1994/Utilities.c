/* Utilities.c */

#include "Utilities.h"
#include <SANE.h>
#include "Memory.h"

/* Machine language note:  */
/*  SUB.W  X,Y  means y=y-x */
/* in other words, the result of an operation is stored always in the second */
/* operand specified */


/* MemCpy which does overlapping regions and calls BlockMove for large blocks */
void			MemCpy(char* Target, char* Source, signed long NumBytes)
	{
		ERROR(NumBytes < 0,PRERR(ForceAbort,"MemCpy asked to copy negative number of bytes"));
		if ((NumBytes > 128) && (NumBytes < 8388607L))
			{
				/* for large blocks, we use the efficient operating system routine */
				BlockMove(Source,Target,NumBytes);
			}
		 else
			{
				if ((Target - Source >= 0) && (Target - Source < NumBytes))
					{
						/* if target starts within source, we have to copy in reverse. */
						Target += NumBytes;
						Source += NumBytes;
						while (NumBytes > 0)
							{
								Target -= 1;
								Source -= 1;
								NumBytes -= 1;
								Target[0] = Source[0];
							}
					}
				 else
					{
						/* we can do normal copy */
						while (NumBytes > 0)
							{
								NumBytes -= 1;
								Target[0] = Source[0];
								Target += 1;
								Source += 1;
							}
					}
			}
	}


/* Memory compare: returns True if both regions are identical */
MyBoolean	MemEqu(char* First, char* Second, long NumBytes)
	{
		ERROR(NumBytes < 0,PRERR(ForceAbort,"MemEqu asked to test negative number of bytes"));
		while (NumBytes > 0)
			{
				if (First[0] != Second[0])
					{
						return False; /* not equal */
					}
				First += 1;
				Second += 1;
				NumBytes -= 1;
			}
		return True; /* no differences found, must be the same */
	}


/* the labs function */
long		LAbs(long value)
	{
		if (value < 0)
			{
				return -value;
			}
		 else
			{
				return value;
			}
	}


/* number of chars in a C string */
long		StrLen(register char* It)
	{
		long		Count;

		Count = 0;
		while (It[Count] != 0)
			{
				Count += 1;
			}
		return Count;
	}


/* get a C string from the resource file. */
/* high word of StringID = ID of resource */
/* low word of StringID = ID of string within that resource */
Handle		GetCString(ulong StringID)
	{
		Handle	StrRes;
		Ptr			StrList;
		ulong		Scan;  /* scans the list by word (2 bytes) */
		ushort	Count;
		ushort	IDToFind;

		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		StrRes = GetResource('�Cst',(StringID >> 16) & 0x0000ffff);
		if (StrRes == NIL)
			{
				PRERR(ForceAbort,"GetCString couldn't find string resource.");
			}
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		HLock(StrRes);
		StrList = *StrRes;  /* dereference */
		IDToFind = 0x0000ffff & StringID;  /* isolate local ID */
		Scan = 2;  /* start with the 2nd word */
		Count = ((uchar*)StrList)[0] * 256 + ((uchar*)StrList)[1]; /* get number of items to do */
		while (Count > 0)
			{
				long		NumChars;

				/* finding length of current string */
				NumChars = 0;
				while ( ((char*)StrList)[Scan + sizeof(short) + NumChars] != 0)
					{
						NumChars += 1;
					}
				if (((uchar*)StrList)[Scan] * 256 + ((uchar*)StrList)[Scan + 1] == IDToFind)
					{
						Handle		Temp;

						/* we've found it! */
						/* Make a new handle to put it in */
						Temp = AllocHandle(NumChars);
						SetTag(Temp,"GetCString Result");
						HUnlock(StrRes);
						MemCpy(*Temp,&(((char*)StrList)[Scan+2]),NumChars);
						return Temp;
					}
				/* if length = 0, then skip 2+1 (+1) = 4 */
				/* if length = 1, then skip 2+2 = 4 */
				/* if length = 2, then skip 2+3 (+1) = 6... */
				Scan += (NumChars & 0xfffffffe /* trunc */) + 4;
				Count -= 1;  /* decrement the counter */
			}
		HUnlock(StrRes);
		PRERR(ForceAbort,"GetCString couldn't find specified string in resource.");
	}


/* convert a hexadecimal character into a byte value */
char		Hex2Byte(char Hex)
	{
		if ((Hex >= '0') && (Hex <= '9'))
			{
				return Hex-'0';
			}
		if ((Hex >= 'a') && (Hex <= 'f'))
			{
				return Hex-'a'+10;
			}
		if ((Hex >= 'A') && (Hex <= 'F'))
			{
				return Hex-'A'+10;
			}
		return -1;  /* conversion error */
	}


/* convert a byte to a hexadecimal character */
char		Byte2Hex(char Value)
	{
		static char	Table[16] = "0123456789abcdef";

		return Table[Value];
	}


/* convert a real number to a string handle (with no length delimitors) */
Handle	Real2String(long double Value, char Mode, short Digits)
	{
		PString		TempString;
		extended	TempReal;
		decform		TempDecForm;
		Handle		RetStr;

		TempDecForm.style = Mode;
		TempDecForm.digits = Digits;
		#if !__option(mc68881) && __option(native_fp)
			TempReal = Value;
		#else
			x96tox80(&Value,&TempReal); /* coerce the long double to extended format */
		#endif
		num2str(&TempDecForm,TempReal,TempString);
		RetStr = AllocHandle(TempString[0]); /* return length of string */
		SetTag(RetStr,"Real2String Return");
		MemCpy(*RetStr,(char*)&(TempString[1]),TempString[0]);
		return RetStr;
	}


/* convert a string to a real number */
long double		String2Real(Handle StringHandle)
	{
		PString		TempStr;
		decimal		TempDecimal;
		Boolean		TempBoolean; /* has to be operating system's Boolean typedef */
		short			TempShort;
		long double	TempReal;
		extended	TempExtended;

		Handle2PString(StringHandle,TempStr);
		str2dec(TempStr,&TempShort,&TempDecimal,&TempBoolean);
		TempExtended = dec2num(&TempDecimal);
		#if !__option(mc68881) && __option(native_fp)
			return TempExtended;
		#else
			x80tox96(&TempExtended,&TempReal); /* coerce to long double format */
			return TempReal;
		#endif
	}


Handle	HDuplicate(Handle Original)
	{
		Handle		Temp;

		if (Original == NIL)
			{
				Temp = AllocHandle(0); /* return an empty handle */
				SetTag(Temp,"HDuplicate Result");
				return Temp;
			}
		CheckHandleExistence(Original);
		Temp = AllocHandle(HandleSize(Original));
		SetTag(Temp,"HDuplicate Result");
		MemCpy(*Temp,*Original,HandleSize(Original));
		return Temp;
	}


void		Handle2PString(Handle Source, PString PlaceToPut)
	{
		long		Length;
		short		Scan;

		CheckHandleExistence(Source);
		Length = HandleSize(Source);
		if (Length > (sizeof(PString)-1))
			{
				Length = sizeof(PString) - 1;
			}
		PlaceToPut[0] = Length;
		for (Scan = 0; Scan < Length; Scan += 1)
			{
				PlaceToPut[Scan+1] = ((char*)*Source)[Scan];
			}
	}


Handle	PString2Handle(PString String)
	{
		Handle	Temp;

		Temp = AllocHandle((uchar)(String[0]));
		SetTag(Temp,"PString2Handle Return");
		MemCpy(*Temp,(char*)&(String[1]),String[0]);
		return Temp;
	}


Handle	HStrCat(Handle First, Handle Second)
	{
		Handle	Temp;

		CheckHandleExistence(First);
		CheckHandleExistence(Second);
		Temp = AllocHandle(HandleSize(First) + HandleSize(Second));
		SetTag(Temp,"HStrCat Return");
		MemCpy(*Temp,*First,HandleSize(First));
		MemCpy(&((*Temp)[HandleSize(First)]),*Second,HandleSize(Second));
		return Temp;
	}


Handle	HandleOf(char* Data, long NumBytes)
	{
		Handle	Temp;

		Temp = AllocHandle(NumBytes);
		SetTag(Temp,"HandleOf Return");
		MemCpy(*Temp,Data,NumBytes);
		return Temp;
	}


/* convert a long integer to a string handle */
Handle	Int2String(long TheInt)
	{
		char		Buffer[20];
		short		BufPtr;
		char		Buf[20];
		short		BPtr;
		short		Scan;

		BufPtr = 1;
		if (TheInt == -2147483648)
			{
				return PString2Handle("\p-2147483648");
			}
		 else
			{
				if (TheInt < 0)
					{
						Buffer[BufPtr++] = '-';
						TheInt = -TheInt;
					}
				BPtr = 16;
				Buf[--BPtr] = 0;
				do
					{
						Buf[--BPtr] = (TheInt % 10) + '0';
						TheInt = TheInt / 10;
					} while (TheInt != 0);
				for (Scan = BPtr; Scan < 16; Scan += 1)
					{
						Buffer[BufPtr++] = Buf[Scan];
					}
			}
		Buffer[0] = BufPtr - 1;
		return PString2Handle((uchar*)Buffer);
	}


/* convert a string to an integer */
long		String2Int(Handle StringHand)
	{
		MyBoolean		Sign = False;
		long				Accr;
		long				Scan;

		Scan = 0;
		Accr = 0;
		while (((*StringHand)[Scan] == 32) && (Scan < HandleSize(StringHand)))
			{
				Scan += 1;
			}
		while (Scan < HandleSize(StringHand))
			{
				if ((*StringHand)[Scan] == '-')
					{
						Sign = !Sign;
					}
				if (((*StringHand)[Scan] >= '0') && ((*StringHand)[Scan] <= '9'))
					{
						Accr = (10 * Accr) + ((*StringHand)[Scan] - '0');
					}
				Scan += 1;
			}
		return Accr;
	}


/* compare 2 strings.  -1 = first is ahead of second, 0 = identical, 1 = second */
/* is ahead of first */
short		HStrCmp(Handle First, Handle Second)
	{
		register char*	FScan;
		register char*	SScan;
		register long		Count;
		long						SecondSize;
		MyBoolean				Flag;
		MyBoolean				SecondLongerThanFirst;

		CheckHandleExistence(First);
		CheckHandleExistence(Second);
		Count = HandleSize(First);
		SecondSize = HandleSize(Second);
		Flag = (SecondSize == Count);
		SecondLongerThanFirst = (SecondSize > Count);
		if (SecondSize < Count)
			{
				Count = SecondSize;
			}
		FScan = *First;
		SScan = *Second;
		while (Count > 0)
			{
				register char		Temp1;
				register char		Temp2;

				Temp1 = *(FScan++);
				Temp2 = *(SScan++);
				if (Temp1 < Temp2)
					{
						return -1; /* String1 < String2 */
					}
				if (Temp1 > Temp2)
					{
						return 1; /* String1 > String2 */
					}
				Count -= 1;
			}
		if (Flag)
			{
				return 0; /* if same length, then identical */
			}
		 else
			{
				if (SecondLongerThanFirst)
					{
						return -1; /* smaller string extended with nulls < larger */
					}
				 else
					{
						return 1;
					}
			}
	}
