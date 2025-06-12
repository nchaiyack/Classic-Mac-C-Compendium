/* *****************************************************************************
	FILE: 			CursorLDEF.c
	
	DESCRIPTION: 	LDEF unit in C.

	AUTHOR:			Michael J. Conrad
		
	Copyright © 1994 Michael J. Conrad, All Rights Reserved.

	
	Revision History:
	==========================================================
	04.24.94	-	Original draft.
	==========================================================
	
	Here is an LDEF I wrote which displays cursors. It will display the cursor
	and, if its available, text.
	
	If you find a purpose for it, use it freely!

   ***************************************************************************** */

#include "CursorLDEF.h"

#define	cellsPtr	(*(*theList)->cells)
#define listPtr		(*theList)

pascal void	main(	short 		message,
					Boolean 	select,
					Rect 		*rect,
					Cell 		cell,
					short 		dataOffset,
					short 		dataLen,
					ListHandle 	theList		)
{
	FontInfo 	fontInfo;
	SignedByte 	listState, cellState;
	Ptr 		theCursor;
	short 		leftDraw, topDraw;
	
	listState = HGetState(theList);
	HLock(theList);
	
	cellState = HGetState((*theList)->cells);
	HLock((*theList)->cells);
	
	switch (message) 
	{
		case lInitMsg:
	  		break;

		case lDrawMsg:
			EraseRect(rect);
		
	  		if (dataLen > 0) 
	  		{
	  			leftDraw = rect->left + listPtr->indent.h + kLeftOffset;
	  			topDraw  = rect->top + listPtr->indent.v + kTopOffset;
	  			  		
	  		if (dataLen >= sizeof(Cursor)) 
	  		{
	  			theCursor = cellsPtr + dataOffset;
	  			DrawCursor(theCursor, leftDraw, topDraw, listPtr->port);
	  			
	  			dataOffset += sizeof(Cursor);
	  			dataLen -= sizeof(Cursor);
	  		}
	  		
	  		leftDraw += 16 + kCursorSpace;
	  			  		
			GetFontInfo(&fontInfo);
			MoveTo(leftDraw, topDraw + fontInfo.ascent);
						
			TextFace(0);
			if (TextWidth(cellsPtr, dataOffset, dataLen) > (rect->right - leftDraw))
				TextFace(condense);

			DrawText(cellsPtr, dataOffset, dataLen);
	  	}

		if (!select)	break;
		
		case lHiliteMsg:
			BitClr(&HiliteMode, pHiliteBit);
			InvertRect(rect);
	  		break;

		case lCloseMsg:
		  	break;
	  	
		default:
			break;
	}
	
	HSetState(listPtr->cells, cellState);
	HSetState(theList, listState);
}


void DrawCursor(Ptr theCursor, short left, short top, GrafPtr drawPort)
{
	BitMap 	curMap;
	Rect 	destRect;

	curMap.baseAddr = (&((CursPtr)theCursor)->data);
	curMap.rowBytes = 2;
	
	SetRect(&curMap.bounds,0,0,16,16);
	
	SetRect(&destRect,0,0,16,16);
	OffsetRect(&destRect,left,top);
	
	CopyBits(&curMap, &drawPort->portBits, &curMap.bounds, &destRect, srcCopy, NULL);
}
