/* StringUtils.c */

#include "StringUtils.h"
#include "Memory.h"
#include "CSack.h"


static CSack*			StringSack = NIL; /* NIL = operation not in progress */


void				BeginStringOperation(void)
	{
		ERROR(StringSack != NIL,PRERR(ForceAbort,"BeginStringOperation called "
			"while string operation was in progress"));
		StringSack = new CSack;
		StringSack->ISack(sizeof(Handle),4);
		SetTag((Handle)StringSack,"StringSack");
	}


void				EndStringOperation(Handle FinalString)
	{
		Handle		Local;

		ERROR(StringSack == NIL,PRERR(ForceAbort,"EndStringOperation called "
			"when no string operation was in progress"));
		CheckHandleExistence(FinalString);
	 Loop:
		StringSack->ResetScan();
		if (!StringSack->GetNext(&Local))
			{
				/* done */
				delete StringSack;
				StringSack = NIL;
				return;
			}
		if (Local != FinalString)
			{
				ReleaseHandle(Local);
			}
		StringSack->KillElement(&Local);
		goto Loop;
	}


/* concatenate two strings */
Handle			ConStr(Handle LeftStr, Handle RightStr)
	{
		Handle			Temp;
		long				LeftSize;
		long				RightSize;

		LeftSize = HandleSize(LeftStr);
		RightSize = HandleSize(RightStr);
		Temp = AllocHandle(LeftSize + RightSize);
		SetTag(Temp,"StringTemp");
		HRNGCHK(Temp,&((*Temp)[0]),LeftSize);
		MemCpy(&((*Temp)[0]),*LeftStr,LeftSize);
		HRNGCHK(Temp,&((*Temp)[LeftSize]),RightSize);
		MemCpy(&((*Temp)[LeftSize]),*RightStr,RightSize);
		RegisterString(Temp);
		return Temp;
	}


/* take a portion from the middle of the string.  If it would run off */
/* then end, then it is truncated */
Handle			MidStr(Handle String, long Start, long NumChars)
	{
		Handle			Temp;
		long				OldSize;
		long				NewSize;

		if (Start < 0)
			{
				NumChars += Start;
				Start = 0;
			}
		OldSize = HandleSize(String);
		if ((Start >= OldSize) || (NumChars <= 0))
			{
			 ZeroLength:
				Temp = AllocHandle(0); /* empty string, range started past end */
			}
		 else
			{
				if ((Start + NumChars) - OldSize > 0)
					{
						NumChars -= ((Start + NumChars) - OldSize);
						if (NumChars <= 0)
							{
								goto ZeroLength;
							}
					}
				Temp = AllocHandle(NewSize);
				HRNGCHK(Temp,*Temp,NewSize);
				MemCpy(*Temp,&((*String)[Start]),NewSize);
			}
		SetTag(Temp,"StringTemp");
		RegisterString(Temp);
		return Temp;
	}


/* substitutes the Replacement string into String at the first occurrence */
/* of Key.  If Key is not in String, then String is returned unchanged */
Handle			ReplaceStr(Handle String, Handle Key, Handle Replacement)
	{
		long			Scan;
		long			KeySize;
		long			StringSize;
		long			ReplacementSize;
		Handle		Temp;

		KeySize = HandleSize(Key);
		StringSize = HandleSize(String);
		for (Scan = 0; Scan <= StringSize - KeySize; Scan += 1)
			{
				if (MemEqu(&((*String)[Scan]),*Key,KeySize))
					{
						ReplacementSize = HandleSize(Replacement);
						Temp = AllocHandle(StringSize - KeySize + ReplacementSize);
						/* copying over preceding characters */
						HRNGCHK(Temp,&((*Temp)[0]),Scan);
						MemCpy(&((*Temp)[0]),&((*String)[0]),Scan);
						/* copying over replacement */
						HRNGCHK(Temp,&((*Temp)[Scan]),ReplacementSize);
						MemCpy(&((*Temp)[Scan]),*Replacement,ReplacementSize);
						/* copying over remainder */
						HRNGCHK(Temp,&((*Temp)[Scan + ReplacementSize]),StringSize - KeySize - Scan);
						MemCpy(&((*Temp)[Scan + ReplacementSize]),&((*String)[Scan + KeySize]),
							StringSize - KeySize - Scan);
						SetTag(Temp,"StringTemp");
						RegisterString(Temp);
						return Temp;
					}
			}
		return String;
	}


Handle			CString(char* Start)
	{
		Handle		Temp;

		Temp = HandleOf(Start,StrLen(Start));
		SetTag(Temp,"StringTemp");
		RegisterString(Temp);
		return Temp;
	}


Handle			StringOf(char* Start, long Length)
	{
		Handle		Temp;

		Temp = HandleOf(Start,Length);
		SetTag(Temp,"StringTemp");
		RegisterString(Temp);
		return Temp;
	}


Handle			RegisterString(Handle TheString)
	{
		StringSack->PushElement(&TheString);
		return TheString;
	}
