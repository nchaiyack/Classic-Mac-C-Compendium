/* TextStorage.c */
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
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "TextStorage.h"
#include "Memory.h"
#include "DataMunging.h"
#include "Array.h"
#include "Files.h"
#include "BufferedFileOutput.h"


/* Interesting note:  Any text document automatically has at least one line:  */
/* an empty document has one line with zero characters on it.  However, this */
/* data structure allows zero lines to be stored.  To get around this problem, */
/* a blank line is automatically allocated.  Unless I've overlooked something, */
/* none of the cut and paste type routines should ever result in a structure */
/* containing fewer than one line, but this is something to watch out for. */


struct TextStorageRec
	{
		ArrayRec*				LineMap;
		MyBoolean				DataHasChanged;
	};


/* allocate a new structure for storing text type data */
TextStorageRec*			NewTextStorage(void)
	{
		TextStorageRec*			Storage;
		char*								Thang;

		Storage = (TextStorageRec*)AllocPtrCanFail(
			sizeof(TextStorageRec),"TextStorageRec");
		if (Storage == NIL)
			{
			 MemOut1:
				return NIL;
			}
		Storage->LineMap = NewArray();
		Thang = AllocPtrCanFail(0,"TextLine");
		if (Thang == NIL)
			{
				/* things are in sad shape if this failed */
			 MemOut2:
				DisposeArray(Storage->LineMap);
				goto MemOut1;
			}
		if (!ArrayAppendElement(Storage->LineMap,Thang))
			{
				ReleasePtr(Thang);
				goto MemOut2;
			}
		Storage->DataHasChanged = False;
		return Storage;
	}


/* dispose the text storage structure and any data it contains */
void								DisposeTextStorage(TextStorageRec* Storage)
	{
		long				Limit;
		long				Scan;

		CheckPtrExistence(Storage);
		Limit = ArrayGetLength(Storage->LineMap);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ReleasePtr((char*)ArrayGetElement(Storage->LineMap,Scan));
			}
		DisposeArray(Storage->LineMap);
		ReleasePtr((char*)Storage);
	}


/* find out how many lines the text storage object contains */
long								TextStorageGetLineCount(TextStorageRec* Storage)
	{
		CheckPtrExistence(Storage);
		ERROR(ArrayGetLength(Storage->LineMap) < 1,
			PRERR(ForceAbort,"TextStorageGetLineCount:  0 lines"));
		return ArrayGetLength(Storage->LineMap);
	}


/* get a copy of the specified line */
char*								TextStorageGetLineCopy(TextStorageRec* Storage, long LineIndex)
	{
		char*				Copy;

		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageGetLine:  Line index out of range"));
		Copy = CopyPtr((char*)ArrayGetElement(Storage->LineMap,LineIndex));
		if (Copy != NIL)
			{
				SetTag(Copy,"TextLineCopy");
			}
		return Copy;
	}


/* get a pointer to the actual line as stored in the array.  this line should */
/* not be modified in any way. */
char*								TextStorageGetActualLine(TextStorageRec* Storage, long LineIndex)
	{
		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageGetLine:  Line index out of range"));
		return (char*)ArrayGetElement(Storage->LineMap,LineIndex);
	}


/* get the length of the specified line */
long								TextStorageGetLineLength(TextStorageRec* Storage, long LineIndex)
	{
		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageGetLineLength:  Line index out of range"));
		return PtrSize((char*)ArrayGetElement(Storage->LineMap,LineIndex));
	}


/* put a new line in the text storage object.  the original contents of the */
/* specified line are deleted.  returns False if it couldn't be completed. */
/* (it could fail since a copy of the line is made) */
MyBoolean						TextStorageChangeLine(TextStorageRec* Storage, long LineIndex,
											char* NewLine)
	{
		char*					Copy;

		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageChangeLine:  Line index out of range"));
		Copy = CopyPtr(NewLine);
		if (Copy == NIL)
			{
				return False;
			}
		SetTag(Copy,"TextLine");
		/* don't commit unless we know the operation will succeed */
		ReleasePtr((char*)ArrayGetElement(Storage->LineMap,LineIndex));
		ArraySetElement(Storage->LineMap,Copy,LineIndex);
		Storage->DataHasChanged = True;
		return True;
	}


/* insert a new empty line at the specified position */
MyBoolean						TextStorageInsertLine(TextStorageRec* Storage, long LineIndex)
	{
		char*					Empty;

		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >/*NB*/ TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageInsertLine:  Line index out of range"));
		Empty = AllocPtrCanFail(0,"TextLine");
		if (Empty == NIL)
			{
			 FailurePoint1:
				return False;
			}
		if (!ArrayInsertElement(Storage->LineMap,Empty,LineIndex))
			{
			 FailurePoint2:
				ReleasePtr(Empty);
				goto FailurePoint1;
			}
		Storage->DataHasChanged = True;
		return True;
	}


/* delete the line at the specified position */
void								TextStorageDeleteLine(TextStorageRec* Storage, long LineIndex)
	{
		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageDeleteLine:  Line index out of range"));
		ReleasePtr((char*)ArrayGetElement(Storage->LineMap,LineIndex));
		ArrayDeleteElement(Storage->LineMap,LineIndex);
		Storage->DataHasChanged = True;
	}


/* break the line at the specified character index.  this is used for inserting */
/* carriage returns. */
MyBoolean						TextStorageBreakLine(TextStorageRec* Storage, long LineIndex,
											long CharIndex)
	{
		char*				OldLine;
		char*				FirstHalf;
		long				OldLength;
		char*				SecondHalf;

		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageBreakLine:  Line index out of range"));
		OldLine = (char*)ArrayGetElement(Storage->LineMap,LineIndex);
		OldLength = PtrSize(OldLine);
		ERROR((CharIndex < 0) || (CharIndex > OldLength),
			PRERR(ForceAbort,"TextStorageBreakLine:  Character break index out of range"));
		FirstHalf = MidBlockCopy(OldLine,0,CharIndex);
		if (FirstHalf == NIL)
			{
			 FailurePoint1:
				return False;
			}
		SetTag(FirstHalf,"TextLine");
		SecondHalf = MidBlockCopy(OldLine,CharIndex,OldLength - CharIndex);
		if (SecondHalf == NIL)
			{
			 FailurePoint2:
				ReleasePtr(FirstHalf);
				goto FailurePoint1;
			}
		SetTag(SecondHalf,"TextLine");
		if (!ArrayInsertElement(Storage->LineMap,SecondHalf,LineIndex + 1))
			{
			 FailurePoint3:
				ReleasePtr(SecondHalf);
				goto FailurePoint2;
			}
		ReleasePtr(OldLine);
		/* secondhalf already inserted; need to update firsthalf */
		ArraySetElement(Storage->LineMap,FirstHalf,LineIndex);
		Storage->DataHasChanged = True;
		return True;
	}


/* replace the line and the line after it with a single line containing the */
/* second line concatenated onto the end of the first line */
MyBoolean						TextStorageFoldLines(TextStorageRec* Storage, long LineIndex)
	{
		char*				FirstLine;
		char*				SecondLine;
		char*				Resultant;

		CheckPtrExistence(Storage);
		ERROR((LineIndex < 0) || (LineIndex + 1/*NB*/ >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageFoldLines:  Line index out of range"));
		FirstLine = (char*)ArrayGetElement(Storage->LineMap,LineIndex);
		SecondLine = (char*)ArrayGetElement(Storage->LineMap,LineIndex + 1);
		Resultant = ConcatBlockCopy(FirstLine,SecondLine);
		if (Resultant == NIL)
			{
				return False;
			}
		SetTag(Resultant,"TextLine");
		ArrayDeleteElement(Storage->LineMap,LineIndex + 1);
		ReleasePtr(FirstLine);
		ReleasePtr(SecondLine);
		/* second line already deleted; update first line */
		ArraySetElement(Storage->LineMap,Resultant,LineIndex);
		Storage->DataHasChanged = True;
		return True;
	}


/* extract part of the stored data in the form of another text storage object */
TextStorageRec*			TextStorageExtractSection(TextStorageRec* Storage,
											long StartLine, long StartChar, long EndLine, long EndChar)
	{
		TextStorageRec*		Copy;

		CheckPtrExistence(Storage);
		ERROR((StartLine < 0) || (StartLine >= TextStorageGetLineCount(Storage))
			|| (EndLine < 0) || (EndLine >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageExtractSection:  Line index out of range"));
		ERROR((StartLine > EndLine) || ((StartLine == EndLine) && (StartChar > EndChar)),
			PRERR(ForceAbort,"TextStorageExtractSection:  Inconsistent range specified"));
		ERROR((StartChar < 0) || (StartChar > TextStorageGetLineLength(Storage,StartLine))
			|| (EndChar < 0) || (EndChar > TextStorageGetLineLength(Storage,EndLine)),
			PRERR(ForceAbort,
			"TextStorageExtractSection:  Character ranges for lines exceeded"));
		Copy = NewTextStorage();
		if (Copy == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		if (StartLine == EndLine)
			{
				char*							SubsetCopy;
				MyBoolean					Flag;
				char*							Line;

				/* special case */
				Line = (char*)ArrayGetElement(Storage->LineMap,StartLine);
				SubsetCopy = MidBlockCopy(Line,StartChar,EndChar - StartChar);
				if (SubsetCopy == NIL)
					{
					 FailurePoint2:
						DisposeTextStorage(Copy);
						goto FailurePoint1;
					}
				SetTag(SubsetCopy,"TextLine");
				/* No need to insert line into Copy because new TextStorage's always */
				/* contain one line */
				Flag = TextStorageChangeLine(Copy,0,SubsetCopy);
				ReleasePtr(SubsetCopy);
				if (!Flag)
					{
						goto FailurePoint2;
					}
			}
		 else
			{
				long							LineScan;
				long							TargetLineTracker;
				char*							Line;
				char*							SubsetCopy;
				MyBoolean					Flag;

				TargetLineTracker = 0;
				/* munch start line */
				Line = (char*)ArrayGetElement(Storage->LineMap,StartLine);
				SubsetCopy = MidBlockCopy(Line,StartChar,PtrSize(Line) - StartChar);
				if (SubsetCopy == NIL)
					{
						goto FailurePoint2;
					}
				/* No need to insert first line into Copy because new TextStorage's always */
				/* contain one line */
				Flag = TextStorageChangeLine(Copy,TargetLineTracker,SubsetCopy);
				ReleasePtr(SubsetCopy);
				if (!Flag)
					{
						goto FailurePoint2;
					}
				TargetLineTracker += 1;
				/* munch middle lines (if any) */
				for (LineScan = StartLine + 1; LineScan <= EndLine - 1; LineScan += 1)
					{
						if (!TextStorageInsertLine(Copy,TargetLineTracker))
							{
								goto FailurePoint2;
							}
						if (!TextStorageChangeLine(Copy,TargetLineTracker,
							(char*)ArrayGetElement(Storage->LineMap,LineScan)))
							{
								goto FailurePoint2;
							}
						TargetLineTracker += 1;
					}
				/* munch last line */
				Line = (char*)ArrayGetElement(Storage->LineMap,EndLine);
				SubsetCopy = MidBlockCopy(Line,0,EndChar);
				if (SubsetCopy == NIL)
					{
						goto FailurePoint2;
					}
				if (!TextStorageInsertLine(Copy,TargetLineTracker))
					{
						ReleasePtr(SubsetCopy);
						goto FailurePoint2;
					}
				Flag = TextStorageChangeLine(Copy,TargetLineTracker,SubsetCopy);
				ReleasePtr(SubsetCopy);
				if (!Flag)
					{
						goto FailurePoint2;
					}
				/* TargetLineTracker += 1; -- not needed at end */
			}
		return Copy;
	}


/* delete the specified range of data from the storage.  returns False if it */
/* failed.  if this routine fails, it may have left the task partially finished */
MyBoolean						TextStorageDeleteSection(TextStorageRec* Storage,
											long StartLine, long StartChar, long EndLine, long EndChar)
	{
		long							LineScan;
		char*							Line;

		CheckPtrExistence(Storage);
		ERROR((StartLine < 0) || (StartLine >= TextStorageGetLineCount(Storage))
			|| (EndLine < 0) || (EndLine >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageDeleteSection:  Line index out of range"));
		ERROR((StartLine > EndLine) || ((StartLine == EndLine) && (StartChar > EndChar)),
			PRERR(ForceAbort,"TextStorageDeleteSection:  Inconsistent range specified"));
		ERROR((StartChar < 0) || (StartChar > TextStorageGetLineLength(Storage,StartLine))
			|| (EndChar < 0) || (EndChar > TextStorageGetLineLength(Storage,EndLine)),
			PRERR(ForceAbort,
			"TextStorageDeleteSection:  Character ranges for lines exceeded"));
		Storage->DataHasChanged = True;
		if (StartLine == EndLine)
			{
				char*							LineCopy;

				/* special case */
				Line = (char*)ArrayGetElement(Storage->LineMap,StartLine);
				LineCopy = RemoveBlockFromBlockCopy(Line,StartChar,EndChar - StartChar);
				if (LineCopy == NIL)
					{
						return False;
					}
				SetTag(LineCopy,"TextLine");
				ReleasePtr(Line);
				ArraySetElement(Storage->LineMap,LineCopy,StartLine);
			}
		 else
			{
				char*							LineCopy;

				/* munch start line */
				Line = (char*)ArrayGetElement(Storage->LineMap,StartLine);
				LineCopy = RemoveBlockFromBlockCopy(Line,StartChar,
					PtrSize(Line) - StartChar);
				if (LineCopy == NIL)
					{
						return False;
					}
				SetTag(LineCopy,"TextLine");
				ReleasePtr(Line);
				ArraySetElement(Storage->LineMap,LineCopy,StartLine);
				/* munch middle lines (if any) */
				for (LineScan = StartLine + 1; LineScan <= EndLine - 1; LineScan += 1)
					{
						TextStorageDeleteLine(Storage,StartLine + 1);/* keep deleting line after */
					}
				/* munch last line */
				Line = (char*)ArrayGetElement(Storage->LineMap,StartLine + 1); /* line after */
				LineCopy = RemoveBlockFromBlockCopy(Line,0,EndChar);
				if (LineCopy == NIL)
					{
						return False;
					}
				SetTag(LineCopy,"TextLine");
				ReleasePtr(Line);
				ArraySetElement(Storage->LineMap,LineCopy,StartLine + 1);
				/* finally, fold the two remaining lines */
				if (!TextStorageFoldLines(Storage,StartLine))
					{
						return False;
					}
			}
		return True;
	}


/* insert a storage block at the specified position into this storage block. */
/* returns False if it failed.  if it fails, then it may have actually inserted */
/* some of the data into the storage record */
/* special note:  there are (number of lines) - 1 line breaks */
MyBoolean						TextStorageInsertSection(TextStorageRec* Storage,
											long WhereLine, long WhereChar, TextStorageRec* Stuff)
	{
		long							TotalNumLines;
		long							Scan;

		CheckPtrExistence(Storage);
		ERROR((WhereLine < 0) || (WhereLine >= TextStorageGetLineCount(Storage)),
			PRERR(ForceAbort,"TextStorageInsertSection:  Line position out of range"));
		ERROR((WhereChar < 0) || (WhereChar > TextStorageGetLineLength(Storage,WhereLine)),
			PRERR(ForceAbort,"TextStorageInsertSection:  Character position out of range"));
		/* first, we break the line we're inserting into, to make 2 lines which */
		/* we can insert between */
		Storage->DataHasChanged = True;
		if (!TextStorageBreakLine(Storage,WhereLine,WhereChar))
			{
				return False;
			}
		TotalNumLines = TextStorageGetLineCount(Stuff);
		for (Scan = 0; Scan < TotalNumLines; Scan += 1)
			{
				char*							LineCopy;

				/* for each line, insert a copy into ourself */
				LineCopy = CopyPtr((char*)ArrayGetElement(Stuff->LineMap,Scan));
				if (LineCopy == NIL)
					{
					 FailurePoint1:
						return False;
					}
				if (!ArrayInsertElement(Storage->LineMap,LineCopy,WhereLine + 1 + Scan))
					{
						ReleasePtr(LineCopy);
						goto FailurePoint1;
					}
			}
		/* now we just have to fold the first and last lines back together */
		if (!TextStorageFoldLines(Storage,WhereLine + TotalNumLines))
			{
				return False;
			}
		if (!TextStorageFoldLines(Storage,WhereLine))
			{
				return False;
			}
		return True;
	}


/* if the end of line sequence is of the specified length, then calculate how */
/* many characters a packed buffer of text would contain */
long								TextStorageTotalNumChars(TextStorageRec* Storage, long EOLNSize)
	{
		long				Accumulator;
		long				LineScan;
		long				Limit;

		CheckPtrExistence(Storage);
		Accumulator = 0;
		Limit = TextStorageGetLineCount(Storage);
		for (LineScan = 0; LineScan < Limit; LineScan += 1)
			{
				Accumulator += TextStorageGetLineLength(Storage,LineScan);
				if (LineScan != Limit - 1)
					{
						/* all but last line have an eoln marker */
						Accumulator += EOLNSize;
					}
			}
		return Accumulator;
	}


/* create a packed buffer of lines separated by the specified end of line sequence. */
/* the end of line sequence is null terminated */
char*								TextStorageMakeRawBuffer(TextStorageRec* Storage, char* EOLNChar)
	{
		long							TotalNumChars;
		char*							Buffer;
		long							Index;
		long							LineScan;
		long							Limit;
		char*							Line;
		long							LineLength;
		long							EOLNLength;

		CheckPtrExistence(Storage);
		EOLNLength = StrLen(EOLNChar);
		TotalNumChars = TextStorageTotalNumChars(Storage,EOLNLength);
		Buffer = AllocPtrCanFail(TotalNumChars,"TextStorageRawBuffer");
		if (Buffer == NIL)
			{
				return NIL;
			}
		Index = 0;
		Limit = TextStorageGetLineCount(Storage);
		for (LineScan = 0; LineScan < Limit; LineScan += 1)
			{
				Line = TextStorageGetActualLine(Storage,LineScan);
				LineLength = PtrSize(Line);
				PRNGCHK(Buffer,&(Buffer[Index]),LineLength);
				CopyData(Line,&(Buffer[Index]),LineLength);
				Index += LineLength;
				if (LineScan != Limit - 1)
					{
						/* all but last line get eoln */
						PRNGCHK(Buffer,&(Buffer[Index]),EOLNLength);
						CopyData(EOLNChar,&(Buffer[Index]),EOLNLength);
						Index += EOLNLength;
					}
			}
		return Buffer;
	}


/* decode the packed buffer of lines and create a text storage record from it. */
TextStorageRec*			TextStorageFromRawBuffer(char* Buffer, char* EOLNChar)
	{
		TextStorageRec*		Storage;
		long							LineTracking;
		long							Index;
		long							BufferSize;
		long							CurrentLineLength;
		long							EOLNLength;
		char*							TempLine;
		MyBoolean					LastLineEndedWithCR;

		Storage = NewTextStorage();
		if (Storage == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		BufferSize = PtrSize(Buffer);
		EOLNLength = StrLen(EOLNChar);
		Index = 0;
		LineTracking = 0;
		LastLineEndedWithCR = False;
		while (Index < BufferSize)
			{
				CurrentLineLength = 0;
				while ((CurrentLineLength + Index < BufferSize)
					&& ((EOLNLength == 0) || !MemEqu(EOLNChar,
					&(Buffer[CurrentLineLength + Index]),EOLNLength)))
					{
						CurrentLineLength += 1;
					}
				if (CurrentLineLength + Index < BufferSize)
					{
						LastLineEndedWithCR = (EOLNLength != 0) && MemEqu(EOLNChar,
							&(Buffer[CurrentLineLength + Index]),EOLNLength);
					}
				TempLine = BlockFromRaw(&(Buffer[Index]),CurrentLineLength);
				if (TempLine == NIL)
					{
					 FailurePoint2:
						DisposeTextStorage(Storage);
						goto FailurePoint1;
					}
				if (LineTracking != 0)
					{
						/* if it isn't the first line, then we need to append a line */
						if (!TextStorageInsertLine(Storage,LineTracking))
							{
							 FailurePoint3:
								ReleasePtr(TempLine);
								goto FailurePoint2;
							}
					}
				/* store the line in */
				if (!TextStorageChangeLine(Storage,LineTracking,TempLine))
					{
						goto FailurePoint3;
					}
				ReleasePtr(TempLine);
				Index = Index + CurrentLineLength + EOLNLength;
				LineTracking += 1;
			}
		/* if the block ended with a carriage return, then add a blank line on the end */
		if (LastLineEndedWithCR)
			{
				if (!TextStorageInsertLine(Storage,LineTracking))
					{
						goto FailurePoint2;
					}
			}
		return Storage;
	}


/* find out if the data has been changed since the last call to */
/* TextStorageDataIsUpToDate */
MyBoolean						TextStorageHasDataChanged(TextStorageRec* Storage)
	{
		CheckPtrExistence(Storage);
		return Storage->DataHasChanged;
	}


/* indicate that any changes in the data have been recognized */
void								TextStorageDataIsUpToDate(TextStorageRec* Storage)
	{
		CheckPtrExistence(Storage);
		Storage->DataHasChanged = False;
	}


/* write the entire buffer to a file using the specified end of line sequence. */
/* returns True if successful */
MyBoolean						TextStorageWriteDataToFile(TextStorageRec* Storage,
											FileType* FileRefNum, char* EOLN)
	{
		BufferedOutputRec*	Output;
		long								Limit;
		long								Scan;
		long								EOLNLength;
		MyBoolean						Success;

		CheckPtrExistence(Storage);
		CheckPtrExistence(FileRefNum);
		EOLNLength = StrLen(EOLN);
		Output = NewBufferedOutput(FileRefNum);
		Success = False;
		if (Output != NIL)
			{
				Success = True;
				Limit = ArrayGetLength(Storage->LineMap);
				for (Scan = 0; (Scan < Limit) && Success; Scan += 1)
					{
						char*							DataTemp;

						DataTemp = (char*)ArrayGetElement(Storage->LineMap,Scan);
						CheckPtrExistence(DataTemp);
						if (!WriteBufferedOutput(Output,PtrSize(DataTemp),DataTemp))
							{
								Success = False;
							}
						 else
							{
								if (Scan != Limit - 1)
									{
										if (!WriteBufferedOutput(Output,EOLNLength,EOLN))
											{
												Success = False;
											}
									}
							}
					}
				if (!EndBufferedOutput(Output))
					{
						Success = False;
					}
			}
		return Success;
	}
