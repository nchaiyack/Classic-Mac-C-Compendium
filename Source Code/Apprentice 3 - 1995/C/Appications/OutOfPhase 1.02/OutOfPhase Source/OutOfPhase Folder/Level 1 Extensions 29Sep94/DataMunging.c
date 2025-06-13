/* DataMunging.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "DataMunging.h"
#include "Memory.h"


/* Make a copy of the supplied Ptr.  If there is not enough memory to copy */
/* the Ptr, NIL is returned. */
char*					CopyPtr(char* ThePtr)
	{
		char*				Dup;
		long				Size;

		Size = PtrSize(ThePtr);
		Dup = AllocPtrCanFail(Size,"CopyPtr");
		if (Dup != NIL)
			{
				CopyData(ThePtr,Dup,Size);
			}
		return Dup;
	}


/* return the length of a null-terminated string */
long					StrLen(char* String)
	{
		long				Scan;

		Scan = 0;
		while (String[Scan] != 0)
			{
				Scan += 1;
			}
		return Scan;
	}


/* make a new copy of the Ptr containing a null-terminated string.  The */
/* returned Ptr will NOT contain a null at the end.  Returns NIL if the */
/* routine failed */
char*					StringToBlockCopy(char* String)
	{
		long				Index;
		char*				Block;

		Index = StrLen(String);
		Block = AllocPtrCanFail(Index,"StringToBlockCopy");
		if (Block != NIL)
			{
				CopyData(String,Block,Index);
			}
		return Block;
	}


/* Make a copy of the specified block Ptr, appending a null to the end */
/* to make it into a null-terminated string.  Returns NIL if it failed */
char*					BlockToStringCopy(char* Block)
	{
		long					Index;
		char*					String;

		Index = PtrSize(Block);
		String = AllocPtrCanFail(Index + 1,"BlockToStringCopy");
		if (String != NIL)
			{
				CopyData(Block,String,Index);
				String[Index] = 0;
			}
		return String;
	}


/* allocate a block and copy the raw data to it.  Returns NIL if it failed. */
char*					BlockFromRaw(char* Data, long Length)
	{
		char*				Stuff;

		Stuff = AllocPtrCanFail(Length,"BlockFromRaw");
		if (Stuff != NIL)
			{
				CopyData(Data,Stuff,Length);
			}
		return Stuff;
	}


/* allocate a block and copy the null-terminated string (including the null) */
/* to it.  Returns NIL if the routine failed. */
char*					StringFromRaw(char* Data)
	{
		char*				Stuff;
		long				Length;

		Length = StrLen(Data);
		Stuff = AllocPtrCanFail(Length + 1,"StringFromRaw");
		if (Stuff != NIL)
			{
				CopyData(Data,Stuff,Length);
				Stuff[Length] = 0;
			}
		return Stuff;
	}


/* allocate a copy of the specified Ptr, appending the character at the end */
/* returns NIL if the routine failed */
char*					AppendCharToBlockCopy(char* ThePtr, char TheChar)
	{
		long				Index;
		char*				Copy;

		Index = PtrSize(ThePtr);
		Copy = AllocPtrCanFail(Index + 1,"AppendCharToBlockCopy");
		if (Copy != NIL)
			{
				CopyData(ThePtr,Copy,Index);
				Copy[Index] = TheChar;
			}
		return Copy;
	}


/* compare memory ranges and return True if they are equal or False if not. */
MyBoolean			MemEqu(char* Left, char* Right, long NumBytes)
	{
		ERROR(NumBytes < 0,PRERR(ForceAbort,"MemEqu:  Comparison of negative bytes"));
		while (NumBytes > 0)
			{
				if (Left[0] != Right[0])
					{
						return False; /* not equal */
					}
				Left += 1;
				Right += 1;
				NumBytes -= 1;
			}
		return True; /* no differences found, must be the same */
	}


/* compare memory ranges, but treat uppercase and lowercase letters as equal */
MyBoolean			MemEquNoCase(char* First, char* Second, long NumBytes)
	{
		ERROR(NumBytes < 0,PRERR(ForceAbort,"MemEquNoCase:  negative number of bytes"));
		while (NumBytes > 0)
			{
				char				F,S;

				F = First[0];
				S = Second[0];
				if ((F >= 'A') && (F <= 'Z'))
					{
						F = F - 'A' + 'a';
					}
				if ((S >= 'A') && (S <= 'Z'))
					{
						S = S - 'A' + 'a';
					}
				if (F != S)
					{
						return False; /* not equal */
					}
				First += 1;
				Second += 1;
				NumBytes -= 1;
			}
		return True; /* no differences found, must be the same */
	}


/* return true if the null terminated strings are equal, or false if not */
MyBoolean			StrEqu(char* Left, char* Right)
	{
	 LoopPoint:
		if (*Left == 0)
			{
				return (*Right == 0);
			}
		if (*Right == 0)
			{
				return False;
			}
		if (*Left != *Right)
			{
				return False;
			}
		Left += 1;
		Right += 1;
		goto LoopPoint;
	}


/* insert the specified block of raw data into the Ptr and return the new copy */
/* returns NIL if the routine failed */
char*					InsertBlockIntoBlockCopy(char* Block, char* NewData,
								long Where, long Length)
	{
		long				BlockLen;
		char*				Copy;

		BlockLen = PtrSize(Block);
		Copy = AllocPtrCanFail(BlockLen + Length,"InsertBlockIntoBlockCopy");
		if (Copy != NIL)
			{
				/* copy first part (before insertion) of data over */
				PRNGCHK(Block,&(Block[0]),Where);
				PRNGCHK(Copy,&(Copy[0]),Where);
				CopyData(&(Block[0]),&(Copy[0]),Where);
				/* copy new data over */
				PRNGCHK(Copy,&(Copy[Where]),Length);
				CopyData(NewData,&(Copy[Where]),Length);
				/* copy rest (after insertion) of data over */
				PRNGCHK(Block,&(Block[Where]),BlockLen - Where);
				PRNGCHK(Copy,&(Copy[Where + Length]),BlockLen - Where);
				CopyData(&(Block[Where]),&(Copy[Where + Length]),BlockLen - Where);
			}
		return Copy;
	}


/* remove the specified area from the Ptr and return the copy or NIL if it failed */
char*					RemoveBlockFromBlockCopy(char* Block, long Where, long Length)
	{
		long				BlockLen;
		char*				Copy;

		BlockLen = PtrSize(Block);
		ERROR((Where < 0) || (Where > PtrSize(Block)) || (Length < 0)
			|| (Length + Where > PtrSize(Block)),PRERR(ForceAbort,
			"RemoveBlockFromBlockCopy:  Block specifier out of range"));
		Copy = AllocPtrCanFail(BlockLen - Length,"RemoveBlockFromBlockCopy");
		if (Copy != NIL)
			{
				PRNGCHK(Copy,&(Copy[0]),Where);
				CopyData(&(Block[0]),&(Copy[0]),Where);
				PRNGCHK(Copy,&(Copy[Where]),BlockLen - Where - Length);
				CopyData(&(Block[Where + Length]),&(Copy[Where]),
					BlockLen - Where - Length);
			}
		return Copy;
	}


/* identical to InsertBlockIntoBlockCopy but allows element size to be specified */
char*					InsertEntryIntoArrayCopy(char* Array, char* NewEntry,
								long Where, long ElementSize)
	{
		ERROR((PtrSize(Array) % ElementSize) != 0,
			PRERR(ForceAbort,"InsertEntryIntoArray:  array contains an incomplete object"));
		return InsertBlockIntoBlockCopy(Array,NewEntry,Where * ElementSize,ElementSize);
	}


/* Identical to RemoveBlockFromBlockCopy but allows element size to be specified */
char*					RemoveEntryFromArrayCopy(char* Array, long EntryIndex, long ElementSize)
	{
		ERROR((PtrSize(Array) % ElementSize) != 0,
			PRERR(ForceAbort,"RemoveEntryFromArray:  array contains an incomplete object"));
		return RemoveBlockFromBlockCopy(Array,EntryIndex * ElementSize,ElementSize);
	}


/* search for the first occurrence of Key in Block and replace the data with */
/* Replacement, returning a copy of the data or NIL if it failed */
char*					ReplaceBlockCopy(char* Block, char* Key, char* Replacement)
	{
		long					Scan;
		long					KeySize;
		long					BlockSize;
		long					ReplacementSize;

		KeySize = PtrSize(Key);
		BlockSize = PtrSize(Block);
		for (Scan = 0; Scan <= BlockSize - KeySize; Scan += 1)
			{
				if (MemEqu(&(Block[Scan]),Key,KeySize))
					{
						char*					Temp;

						/* here we found a replacement */
						ReplacementSize = PtrSize(Replacement);
						Temp = AllocPtrCanFail(BlockSize - KeySize
							+ ReplacementSize,"ReplaceBlock");
						if (Temp != NIL)
							{
								/* copying over preceding characters */
								PRNGCHK(Temp,&(Temp[0]),Scan);
								CopyData(&(Block[0]),&(Temp[0]),Scan);
								/* copying over replacement */
								PRNGCHK(Temp,&(Temp[Scan]),ReplacementSize);
								CopyData(Replacement,&(Temp[Scan]),ReplacementSize);
								/* copying over remainder */
								PRNGCHK(Temp,&(Temp[Scan + ReplacementSize]),
									BlockSize - KeySize - Scan);
								CopyData(&(Block[Scan + KeySize]),
									&(Temp[Scan + ReplacementSize]),
									BlockSize - KeySize - Scan);
							}
						return Temp;
					}
			}
		/* we return a new copy if we could replace.  Since we always have to return */
		/* a copy, we make a copy here */
		return CopyPtr(Block);
	}


/* concatenate two blocks, returning the new block or NIL if it failed */
char*					ConcatBlockCopy(char* Left, char* Right)
	{
		long				TotalSize;
		char*				Accumulator;
		long				LeftSize;
		long				RightSize;

		LeftSize = PtrSize(Left);
		RightSize = PtrSize(Right);
		TotalSize = LeftSize + RightSize;
		Accumulator = AllocPtrCanFail(TotalSize,"ConcatBlock");
		if (Accumulator != NIL)
			{
				PRNGCHK(Accumulator,Accumulator,LeftSize);
				CopyData(Left,Accumulator,LeftSize);
				PRNGCHK(Accumulator,&(Accumulator[LeftSize]),RightSize);
				CopyData(Right,&(Accumulator[LeftSize]),RightSize);
			}
		return Accumulator;
	}


/* return a middle section of the block or NIL if it failed */
char*					MidBlockCopy(char* Block, long Start, long NumChars)
	{
		long				TotalLength;
		char*				New;

		if (Start < 0)
			{
				NumChars = NumChars - (-Start);
				Start = 0;
			}
		if (NumChars < 0)
			{
				NumChars = 0;
			}
		TotalLength = PtrSize(Block);
		if (Start >= TotalLength)
			{
				/* degenerate result */
				NumChars = 0;
			}
		 else
			{
				if (TotalLength - Start < NumChars)
					{
						NumChars = TotalLength - Start;
					}
			}
		New = AllocPtrCanFail(NumChars,"MidBlock");
		if (New != NIL)
			{
				PRNGCHK(New,New,NumChars);
				CopyData(&(Block[Start]),New,NumChars);
			}
		return New;
	}


/* return a block from which a section has been stripped, or NIL if it failed. */
char*					ReduceBlockCopy(char* Block, long Start, long NumChars)
	{
		long				TotalLength;
		char*				Result;

		if (Start < 0)
			{
				NumChars = NumChars - (-Start);
				Start = 0;
			}
		if (NumChars < 0)
			{
				NumChars = 0;
			}
		TotalLength = PtrSize(Block);
		if (Start >= TotalLength)
			{
				return Block;
			}
		if (TotalLength - Start < NumChars)
			{
				NumChars = TotalLength - Start;
			}
		Result = AllocPtrCanFail(TotalLength - NumChars,"ReduceBlock");
		if (Result != NIL)
			{
				PRNGCHK(Result,Result,Start);
				CopyData(Block,Result,Start); /* perform leading copy */
				PRNGCHK(Result,&(Result[Start]),TotalLength - NumChars - Start);
				CopyData(&(Block[Start + NumChars]),&(Result[Start]),
					TotalLength - NumChars - Start);
			}
		return Result;
	}
