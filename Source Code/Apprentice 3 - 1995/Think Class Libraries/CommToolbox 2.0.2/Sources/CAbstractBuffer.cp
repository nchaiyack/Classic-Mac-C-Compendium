/******************************************************************************
 CAbstractBuffer.cp

		Interface for CAbstractBuffer class.
		
	Copyright � 1994-95 Ithran Einhorn. All rights reserved.
 ******************************************************************************/

#pragma once

#include "CAbstractBuffer.h"

CAbstractBuffer::CAbstractBuffer()
{
	TCL_END_CONSTRUCTOR
}

CAbstractBuffer::~CAbstractBuffer()
{
	TCL_START_DESTRUCTOR
}
	
/** Text Specification **/

void	CAbstractBuffer::SetTextPtr(Ptr textPtr, long numChars)
{
}

void	CAbstractBuffer::UseTextHandle(Handle textHandle)
{
}

void	CAbstractBuffer::InsertTextPtr(Ptr insertPtr, long insertLen)
{
}

Handle	CAbstractBuffer::CopyTextRange(long start, long end)
{
	return NULL;
}

void	CAbstractBuffer::ReplaceSelection(Ptr replacePtr, long replaceLen)
{
}

/** Accessing **/
Handle	CAbstractBuffer::GetTextHandle(void)
{
	return NULL;
}

Handle	CAbstractBuffer::GetRawTextHandle(void)
{
	return NULL;
}
	
long	CAbstractBuffer::FindLine(long charPos)
{
	return 0L;
}

long	CAbstractBuffer::GetLineStart(long line)
{
	return 0L;
}

long	CAbstractBuffer::GetLineEnd(long line)
{
	return 0L;
}

short	CAbstractBuffer::GetLineLength(long line)
{
	return 0;
}

long	CAbstractBuffer::GetLength(void)
{
	return 0L;
}

long	CAbstractBuffer::GetNumLines(void)
{
	return 0L;
}

void	CAbstractBuffer::GetSelection(long *selStart, long *selEnd)
{
}

void	CAbstractBuffer::GetCharBefore(long *aPosition, tCharBuf charBuf)
{
}

void	CAbstractBuffer::GetCharAfter(long *aPosition, tCharBuf charBuf)
{
}

long	CAbstractBuffer::GetGapPosition(void)
{
	return 0L;
}

long	CAbstractBuffer::GetGapLength(void)
{
	return 0L;
}

