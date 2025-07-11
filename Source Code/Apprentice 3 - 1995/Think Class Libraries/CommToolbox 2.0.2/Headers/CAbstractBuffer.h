/******************************************************************************
 CAbstractBuffer.h

		Interface for CAbstractBuffer class.
		
	Copyright � 1994-95 Ithran Einhorn. All rights reserved.
 ******************************************************************************/

#pragma once

#ifndef tCharBuf
typedef unsigned char  tCharBuf[5];
#endif

class CAbstractBuffer TCL_AUTO_DESTRUCT_OBJECT
{
	private:
	
	public:
	
	CAbstractBuffer();
	virtual ~CAbstractBuffer();
	
	/** Text Specification **/
	virtual void	SetTextPtr(Ptr textPtr, long numChars);
	virtual void	UseTextHandle(Handle textHandle);
	virtual void	InsertTextPtr(Ptr insertPtr, long insertLen);
	virtual Handle	CopyTextRange(long start, long end);
	virtual void	ReplaceSelection(Ptr replacePtr, long replaceLen);

	/** Accessing **/
	virtual Handle	GetTextHandle(void);
	virtual Handle	GetRawTextHandle(void);
	
	virtual long	FindLine(long charPos);
	virtual long	GetLineStart(long line);
	virtual long	GetLineEnd(long line);
	virtual short	GetLineLength(long line);
	virtual long	GetLength(void);
	virtual long	GetNumLines(void);
	virtual void	GetSelection(long *selStart, long *selEnd);
	virtual void	GetCharBefore(long *aPosition, tCharBuf charBuf);
	virtual void	GetCharAfter(long *aPosition, tCharBuf charBuf);
	virtual long	GetGapPosition(void);
	virtual long	GetGapLength(void);
};