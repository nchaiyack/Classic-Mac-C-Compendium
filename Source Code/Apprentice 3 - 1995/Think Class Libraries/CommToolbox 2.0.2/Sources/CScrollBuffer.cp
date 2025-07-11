/******************************************************************************
 CScrollBuffer.cp

		Interface for CScrollBuffer class.
		
	SUPERCLASS = CAbstractBuffer
	
	Copyright � 1994-95 Ithran Einhorn. All rights reserved.
 ******************************************************************************/

#pragma once

#include "CScrollBuffer.h"
#include "stringx.h"

/******************************************************************************


******************************************************************************/

CScrollBuffer::CScrollBuffer(long maxNumLines) : CAbstractBuffer()
{
	/* Initialize instance variables */
	
	maxLines = maxNumLines;
	
	itsTextHandle = NewHandle(0);
	
	Truncate();
	
	/*itsLineStarts = (LongHandle) NewHandle(sizeof(long));
	**itsLineStarts = 0;*/
}

/******************************************************************************


******************************************************************************/

CScrollBuffer::~CScrollBuffer(void)
{
	ForgetHandle(itsTextHandle);
	//ForgetHandle(itsLineStarts);
}

/** Text Specification **/

void	CScrollBuffer::SetTextPtr(Ptr textPtr, long numChars)
{
}

void	CScrollBuffer::UseTextHandle(Handle textHandle)
{
}

/******************************************************************************
 InsertTextPtr

 add text to the buffer.
******************************************************************************/

void	CScrollBuffer::InsertTextPtr(Ptr insertPtr, long insertLen)
{
 if (itsNumLines < maxLines)
 {
	Ptr		textP;
 	long	oldTextLen = itsActualLength;
 	
  	if (EnlargeBuffer(insertLen + 1))
  	{
		textP = *itsTextHandle;
		textP[oldTextLen] = (char)insertLen;
		BlockMove(insertPtr, textP + oldTextLen + 1, insertLen);
		
		itsTextLength += insertLen;
		itsActualLength += insertLen + 1;
	}
 }
}

/******************************************************************************
CopyTextRange - copy desired section of buffer as (possibly) multi-
				lined formatted text.

******************************************************************************/

Handle	CScrollBuffer::CopyTextRange(long start, long end)
{
	register long		lineNum, lineLen, beginLine, endLine, lineOffset;
	register long		physLineBegin = 0L, physCharOffset = 0L;
	register long		endPhysCharOffset = 0L, endPhysLineBegin = 0L;
	register Handle		hTextCopy = NULL;
	register long		textOffset = 0L;
	register StringPtr	textP = (unsigned char *)*itsTextHandle;
	
	//	sanity check
	start = Max(start, 0);
	end = Min(end, GetTextLength() - 1);
	
	//	allocate buffer to size of text, it's at least as large as we need.
	hTextCopy = NewHandleCanFail(GetHandleSize(itsTextHandle) + 1);
	FailMemError();
	
	StringPtr	copyTextPtr = (unsigned char *)*hTextCopy;
	
	//	find offset of first and last character	
	GetCharInfo(end, &endLine, &endPhysLineBegin, &endPhysCharOffset);
	GetCharInfo(start, &beginLine, &physLineBegin, &physCharOffset);
	
	lineLen = (long)textP[physLineBegin - 1];
	
	/*
		while the current line does not contain the last character, copy
		the line to buffer and trim trailing white space, add line feed. Apply
		to all lines.
	*/
		
	for (lineNum = beginLine; lineNum <= endLine; lineNum++)
	{
		register long	copyLen = lineLen;
		
		// if first line, start at offset of first character
		if (lineNum == beginLine)
			lineOffset = physCharOffset - physLineBegin;
		else
			lineOffset = 0L;
		
		// if last line, only include characters up to and including last character
		if (lineNum == endLine)
			copyLen = (endPhysCharOffset - endPhysLineBegin) + 1;
			
		// calculate number of characters to copy.
		copyLen -= lineOffset;
			
		BlockMove(textP + physLineBegin + lineOffset, copyTextPtr + textOffset, copyLen);
		copyTextPtr[textOffset + copyLen] = '\0';
		textOffset = strlen(strcat(TrimRight((char *)copyTextPtr),"\r"));
		
		lineLen = (long)textP[lineLen + 1];			// get next line length
		physLineBegin += lineLen + sizeof(char);	// point to beginning of next line.
	}
		
	// shrink handle to actual size, and return it.
	SetHandleSize(hTextCopy, textOffset);
	
	return hTextCopy;
}

void	CScrollBuffer::ReplaceSelection(Ptr replacePtr, long replaceLen)
{
}

/** Accessing **/
Handle	CScrollBuffer::GetTextHandle(void)
{
	return itsTextHandle;
}

Handle	CScrollBuffer::GetRawTextHandle(void)
{
	return itsTextHandle;
}
	
/******************************************************************************
FindLine

	find line that given character belongs to.

******************************************************************************/

long	CScrollBuffer::FindLine(long charPos)
{
	long lineNum, physLineBegin, physCharOffset;
	
	GetCharInfo(charPos, &lineNum, &physLineBegin, &physCharOffset);
	
	return Min(lineNum,itsNumLines - 1);
}

/******************************************************************************
GetLineStart

******************************************************************************/

long	CScrollBuffer::GetLineStart(long line)
{
	long	lineOffset = 0L, lineLen = 0L, logOffset = 0L;
	
	GetLineInfoX (line, &lineOffset, &logOffset, &lineLen);

	return logOffset;
}

/******************************************************************************
GetLineEnd

******************************************************************************/

long	CScrollBuffer::GetLineEnd(long line)
{
	long	lineOffset = 0L, lineLen = 0L, logOffset = 0L;
	
	GetLineInfoX (line, &lineOffset, &logOffset, &lineLen);

	return logOffset + lineLen - 1;
}

/******************************************************************************
GetLineLength

******************************************************************************/

short	CScrollBuffer::GetLineLength(long line)
{
	long	lineOffset = 0L, lineLen = 0L, logOffset = 0L;
	
	GetLineInfoX (line, &lineOffset, &logOffset, &lineLen);

	return lineLen;
}

/******************************************************************************
GetLength

******************************************************************************/

long	CScrollBuffer::GetLength(void)
{
	long	lineOffset = 0L, lineLen = 0L, logOffset = 0L;
	
	GetLineInfoX (itsNumLines - 1, &lineOffset, &logOffset, &lineLen);

	return lineOffset + lineLen;
}

/******************************************************************************
GetNumLines

******************************************************************************/

long	CScrollBuffer::GetNumLines(void)
{
	return itsNumLines;
}

void	CScrollBuffer::GetSelection(long *selStart, long *selEnd)
{
}

void	CScrollBuffer::GetCharAt(long *aPosition, tCharBuf charBuf)
{
	if ((itsTextLength > 0) && (*aPosition < itsTextLength))
	{	
		long		lineNum, physLineBegin, physCharOffset;
		StringPtr	textP = (unsigned char *)*itsTextHandle;
	
		GetCharInfo(*aPosition, &lineNum, &physLineBegin, &physCharOffset);
		
		Length(charBuf) = 1;
		charBuf[1] = textP[physCharOffset];
		
	}
	else Length(charBuf) = 0;
}

void	CScrollBuffer::GetCharBefore(long *aPosition, tCharBuf charBuf)
{
	if ((itsTextLength > 0) && (*aPosition <= itsTextLength))
	{
		(*aPosition)--;
		GetCharAt(aPosition, charBuf);
	}
}

void	CScrollBuffer::GetCharAfter(long *aPosition, tCharBuf charBuf)
{
	if ((itsTextLength > 0) && (*aPosition < itsTextLength - 1))
	{
		(*aPosition)++;
		GetCharAt(aPosition, charBuf);
	}
}

long	CScrollBuffer::GetGapPosition(void)
{
	return 0L;
}

long	CScrollBuffer::GetGapLength(void)
{
	return 0L;
}

/******************************************************************************
 EnlargeBuffer

 like the sign says: make it bigger.
******************************************************************************/

void	CScrollBuffer::ReplaceLine(Ptr replacePtr, long replaceLen, long line)
{
}

/******************************************************************************
 AddLine

 add a new line.
******************************************************************************/

void	CScrollBuffer::AddLine(Ptr newLinePtr, long newLineLen)
{
	InsertTextPtr(newLinePtr, newLineLen);
	itsNumLines++;
}

/******************************************************************************
 DeleteLine

 delete line from buffer.
******************************************************************************/

void	CScrollBuffer::DeleteLine(long line)
{
	long		lineOffset = 0L, lineLen = 0L, logOffset = 0L, remainderLen;
	StringPtr	textP = (unsigned char *)*itsTextHandle;
	
	GetLineInfoX (line, &lineOffset, &logOffset, &lineLen);
	remainderLen = GetActualBufferLength(line,-1);
		
	BlockMove(textP + lineOffset + lineLen,
			  textP + lineOffset - sizeof(char),
			  remainderLen);

	itsTextLength -= lineLen;
	itsActualLength -= lineLen + 1;
	
	itsNumLines--;
}

/******************************************************************************
 GetLine

 get a line as a C string.
******************************************************************************/

void	CScrollBuffer::GetLine(long line, StringPtr strLine, long maxLen)
{
	long		lineOffset = 0L, lineLen = 0L, logOffset = 0L, remainderLen;
	StringPtr	textP = (unsigned char *)*itsTextHandle;
	
	GetLineInfoX (line, &lineOffset, &logOffset, &lineLen);
	lineLen = Min(lineLen,maxLen-1);
		
	BlockMove(textP + lineOffset,strLine,lineLen);
	strLine[lineLen] = 0;
}
	
/******************************************************************************
 EnlargeBuffer

 like the sign says: make it bigger.
******************************************************************************/
 
Boolean	CScrollBuffer::EnlargeBuffer(long delta)
{
 	long	amountNeeded, oldTextLen = GetHandleSize(itsTextHandle);
 	Boolean	itWorked = true;
 	
 	// see how much we really need
 	amountNeeded = (itsActualLength + delta) - oldTextLen;
 	
 	if (amountNeeded > 0)
 	{
	  	TRY
	  	{
 			ResizeHandleCanFail(itsTextHandle, oldTextLen + amountNeeded);
			FailMemError();
		}
		CATCH
		{
			SetHandleSize(itsTextHandle, oldTextLen);
			itWorked = false;
		}
		ENDTRY
	}
	return itWorked;
}

/******************************************************************************
 GetLineInfo

 get position and length of line.
******************************************************************************/

void	CScrollBuffer::GetLineInfo(long line, long *start, long *len)
{
	long	physical_start;
	
	GetLineInfoX(line, &physical_start, start, len);
}

/******************************************************************************
 GetLineInfoX

 get position and length of line.
******************************************************************************/

void	CScrollBuffer::GetLineInfoX(long line, long *start, long *log_start, long *len)
{
	StringPtr	textP = (unsigned char *)*itsTextHandle;
	long		lineNum = 0L;
	
	*start = *len = 0L;
	
	while (lineNum < itsNumLines && lineNum <= line)
	{	 
	 *start += sizeof(char) + *len;
	 *log_start += *len;
	 
	 *len = (long)*(char *)textP;
	 textP += sizeof(char) + *len;
	 
	 lineNum++;
	}

}

/******************************************************************************
 GetActualBufferLength

 get length of all or part of the actual buffer.
******************************************************************************/

long	CScrollBuffer::GetActualBufferLength(long startLine, long endLine)
{
	StringPtr	textP = (unsigned char *)*itsTextHandle;
	long		lineNum, bufLen = 0L;
	
	if (startLine < 0)
		startLine = 0;
	
	if (endLine < 0)
		endLine = itsNumLines - 1;
		
	for (lineNum = 0L; lineNum < itsNumLines && lineNum <= endLine; lineNum++)
	{	 
	 long len = sizeof(char) + (long)*(char *)textP;
	 
	 if (lineNum >= startLine)
	 	bufLen += len;
	 	
	 textP += len;
	}
	
	return bufLen;
}

/******************************************************************************
 GetMaxLines

 get max number of lines.
******************************************************************************/

long	CScrollBuffer::GetMaxLines(void)
{
	return maxLines;
}

/******************************************************************************
 GetMaxLines

 get max number of lines.
******************************************************************************/

void	CScrollBuffer::Truncate(void)
{
	SetHandleSize(itsTextHandle,0);
	itsTextLength = itsActualLength = 0;
	itsNumLines = 0;
}

/******************************************************************************
 GetTextLength

 return test length.
******************************************************************************/

long	CScrollBuffer::GetTextLength(void)
{
	return itsTextLength;
}

/******************************************************************************
 GetCharInfo

 get information about position of physical location of given character.
******************************************************************************/

void	CScrollBuffer::GetCharInfo(long charPos, long *line, long *physLineBegin, long *physCharOffset)
{
	StringPtr	textP, saveTextP;
	long		lineNum, bufLen = 0L;
		
	// sanity check
	charPos = Max(charPos,0);
	charPos = Min(charPos,itsTextLength - 1);
	
	textP = saveTextP = (unsigned char *)*itsTextHandle;
		
	for (lineNum = 0L; lineNum < itsNumLines; lineNum++)
	{	 
		long len = (long)*(char *)textP;
	 	 
		textP += sizeof(char);
			
		if (charPos < bufLen + len)
		{		
			*line = lineNum;
			*physLineBegin = (long)(textP - saveTextP);
			*physCharOffset = (long)(&textP[charPos - bufLen] - saveTextP);
			break;
		}
		 	
		bufLen += len;
		textP += len;
	}
}



