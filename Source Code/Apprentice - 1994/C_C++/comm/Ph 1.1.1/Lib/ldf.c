/*______________________________________________________________________

	ldf.c - List Definition Procedure for the Report Module.
	
	Copyright © 1988-1991 Northwestern University.

	This LDEF is used by type 1 reports.  It ignores all but draw
	messages.  The cell data specifies the index in the auxiliary array
	of a handle to an STR# resource, and the offset of the line within
	the resource.
_____________________________________________________________________*/

#pragma load "precompile"
#include "doc.h"

pascal void main (short lMessage, Boolean lSelect, 
	Rect *lRect, Cell *lCell, short lDataOffset,
	short lDataLen, ListHandle lHandle)
	
{
#pragma unused (lSelect, lCell, lDataLen)

	auxInfo			**aux;			/* handle to auxiliary info */
	Handle			theCells;		/* handle to cell data */
	unsigned char	*p;				/* pointer to cell data */
	Handle			theStrings;		/* handle to STR# resource */
	unsigned short	offset;			/* offset of line in STR# resource */
	unsigned char	*theLine;		/* pointer to the line */
	unsigned char	*q;				/* pointer to cur pos in the line */
	unsigned char	*qEnd;			/* pointer to end of line */
	short				nchar;			/* number of chars to draw */
	short				baseLine;		/* base line for text */
	Boolean			escStyle;		/* true if style escape sequence */
	Boolean			escJust;			/* true if just escape sequence */
	Boolean			escPict;			/* true if pict escape sequence */
	unsigned char	styleCode;		/* style */
	unsigned char	justCode;		/* justification */
	short				cellHeight;		/* cell height */
	short				cellWidth;		/* cell width */
	short				picID;			/* pict resource id */
	short				picBand;			/* pict band number */
	PicHandle		picHandle;		/* handle to pict */
	short				picWidth;		/* pict width */
	short				picHeight;		/* pict height */
	Rect				picRect;			/* pict offscren rectangle */
	Rect				picSrcRect;		/* CopyBits source rect */
	Rect				picDstRect;		/* CopyBits dest rect */
	GrafPort			picPort;			/* grafport for offscreen PICT drawing */
	BitMap			picMap;			/* bitmap for offscreen PICT drawing */
	
	/* Get pointer to the line to be drawn. */
	
	if (lMessage != lDrawMsg) return;
	aux = (auxInfo**)(**lHandle).userHandle;
	theCells = (**lHandle).cells;
	p = (unsigned char*)*theCells + lDataOffset;
	theStrings = (**aux).auxArray[*p++];
	offset = (*p << 8) | *(p+1);
	if (!*theStrings) LoadResource(theStrings);
	HLock(theStrings);
	theLine = (unsigned char*)*theStrings + offset;
	
	/* Get escape sequence info. */
	
	escStyle = escJust = escPict = false;
	q = theLine+1;
	qEnd = q + *theLine;
	while (q < qEnd && *q < 31) {
		switch (*q) {
			case docStyle:
				escStyle = true;
				styleCode = *(q+2);
				break;
			case docJust:
				escJust = true;
				justCode = *(q+2);
				break;
			case docPict:
				escPict = true;
				picID = *(q+2)<<8 | *(q+3);
				picBand = *(q+4)<<8 | *(q+5);
				break;
		}
		q += *(q+1);
	}
					
	if (escPict) {
	
		/* Draw a picture. */
		
		cellWidth = lRect->right - lRect->left;
		cellHeight = (**lHandle).cellSize.v;
		
		if ((**aux).cachedPictID != picID) {
		
			/* This picture is not cached - we must cache it in an offscreen
				bitmap. */
			
			/* Dispose of any previously cached bitmap. */
			
			if ((**aux).cachedPictID) DisposPtr((**aux).cachedBitMap.baseAddr);
			
			/* Compute picRect = the bounds rectangle for the cached
				picture. */
			
			picHandle = GetPicture(picID);
			if (!picHandle) {
				HUnlock(theStrings);
				return;
			}
			if (!*picHandle) LoadResource((Handle)picHandle);
			HLock((Handle)picHandle);
			picWidth = (**picHandle).picFrame.right -
				(**picHandle).picFrame.left;
			picHeight = (**picHandle).picFrame.bottom -
				(**picHandle).picFrame.top;
			if (!escJust) justCode = docCenter;
			switch (justCode) {
				case docLeft:
					picRect.left = lRect->left + 4;
					break;
				case docCenter:
					picRect.left = lRect->left + 
						((cellWidth - picWidth)>>1);
					break;
				case docRight:
					picRect.left = lRect->right - 4 - picWidth;
					break;
			}
			picRect.right = picRect.left + picWidth;
			picRect.top = 0;
			picRect.bottom = picHeight;
	
			/* Allocate and initialize the offscreen  bitmap. */
			
			(**aux).cachedPictID = picID;
			picMap.bounds = picRect;
			picMap.rowBytes = (((picWidth+7)>>3) + 1) & 0xfffe;
			picMap.baseAddr = NewPtr(picMap.rowBytes*picHeight);
			(**aux).cachedBitMap = picMap;
			
			/* Draw the picture in the offscreen bitmap. */
			
			OpenPort(&picPort);
			SetPortBits(&picMap);
			picPort.portRect = picRect;
			RectRgn(picPort.visRgn, &picRect);
			ClipRect(&picRect);
			EraseRect(&picRect);
			DrawPicture(picHandle, &picRect);
			HUnlock((Handle)picHandle);
			ClosePort(&picPort);
			SetPort((**lHandle).port);
		}

		/* CopyBits the proper band from the offscreen cached bitmap to
			the cell. */
			
		picMap = (**aux).cachedBitMap;
		picSrcRect = picMap.bounds;
		picSrcRect.top = picBand*cellHeight;
		picSrcRect.bottom = picSrcRect.top + cellHeight;
		if (picSrcRect.bottom > picMap.bounds.bottom) 
			picSrcRect.bottom = picMap.bounds.bottom;
		picDstRect = picSrcRect;
		picDstRect.top = lRect->top;
		picDstRect.bottom = picDstRect.top + picSrcRect.bottom - picSrcRect.top;
		CopyBits(&picMap, &(**lHandle).port->portBits, &picSrcRect, &picDstRect, 
			srcCopy, nil);
		
	} else {
	
		/* Draw a text line. */
	
		if (!escStyle) styleCode = normal;
		if (!escJust) justCode = docLeft;
		TextFace(styleCode);
		nchar = *theLine - (q - theLine - 1);
		if (nchar && *(q+nchar-1) == docEop) nchar--;
		baseLine = lRect->bottom - 2;
		switch (justCode) {
			case docLeft:
				MoveTo(lRect->left + 4, baseLine);
				break;
			case docCenter:
				MoveTo((lRect->left + lRect->right - TextWidth(q, 0, nchar)) >> 1,
					baseLine);
				break;
			case docRight:
				MoveTo(lRect->right - 4 - TextWidth(q, 0, nchar), baseLine);
				break;
		}
		DrawText(q, 0, nchar);
		TextFace(normal);
	}
	
	HUnlock(theStrings);
}
