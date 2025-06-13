/******************************************************************************
 CScrollBuffer.h

		Interface for CScrollBuffer class.
		
	SUPERCLASS = CAbstractBuffer
	
	Copyright © 1994-95 Ithran Einhorn. All rights reserved.
 ******************************************************************************/

#pragma once

#include "CAbstractBuffer.h"

#define		kDefMaxLines	128

class CScrollBuffer : public CAbstractBuffer
{
	private:
	Handle				itsTextHandle;		/* Handle to text to display		*/
	long				itsTextLength;		/* Length of text to display		*/
	long				itsActualLength;	/* Length of actual buffer			*/
	long				itsNumLines;		/* Number of lines in text			*/
	LongHandle			itsLineStarts;		/* Offsets to starts of lines		*/
	long				maxLines;			/* maximum number of lines to store	*/
	
	public:
	
	CScrollBuffer(long maxNumLines = kDefMaxLines);
	~CScrollBuffer();
	
	/** Text Specification **/
	void	SetTextPtr(Ptr textPtr, long numChars);
	void	UseTextHandle(Handle textHandle);
	void	InsertTextPtr(Ptr insertPtr, long insertLen);
	Handle	CopyTextRange(long start, long end);
	void	ReplaceSelection(Ptr replacePtr, long replaceLen);

	/** Accessing **/
	Handle	GetTextHandle(void);
	Handle	GetRawTextHandle(void);
	
	long	FindLine(long charPos);
	long	GetLineStart(long line);
	long	GetLineEnd(long line);
	short	GetLineLength(long line);
	long	GetLength(void);
	long	GetNumLines(void);
	void	GetSelection(long *selStart, long *selEnd);
	void	GetCharAt(long *aPosition, tCharBuf charBuf);
	void	GetCharBefore(long *aPosition, tCharBuf charBuf);
	void	GetCharAfter(long *aPosition, tCharBuf charBuf);
	long	GetGapPosition(void);
	long	GetGapLength(void);
	
	/** new access functions */
	virtual void	GetLineInfo(long line, long *start, long *len);
	virtual void	GetLineInfoX(long line, long *start, long *log_start, long *len);
	virtual long	GetMaxLines(void);
	virtual	void	ReplaceLine(Ptr replacePtr, long replaceLen, long line);
	virtual	void	AddLine(Ptr newLinePtr, long newLineLen);
	virtual	void	DeleteLine(long line);
	virtual	void	GetLine(long line, StringPtr strLine, long maxLen);	
	virtual	Boolean	EnlargeBuffer(long delta);
	virtual	long	GetActualBufferLength(long startLine, long startLine);
	virtual	void	Truncate(void);
	virtual	long	GetTextLength(void);
	virtual void	GetCharInfo(long charPos, long *line, long *physLineBegin, long *physCharOffset);
};