// File "Icon LDEF.c" - Icon Suite LDEF Routine  10/16/93
//   Code is shamelessly based on Apple Snippet by Steven Falkenburg 5/23/91
//   This code is placed in the public domain for free use and distribution! MJS

#include <GestaltEqu.h>
#include <Icons.h>

// * **************************************************************************** * //
// * **************************************************************************** * //

pascal void	main(short message, Boolean hilited, Rect *cellRect, Cell theCell,
		short dataOffset, short dataLen, ListHandle theList) {
	long response;
	Point drawPt;
	Rect iconRect, textRect;
	Ptr cellData;
	Handle iconHdl;
	ListPtr theListPtr;
	SignedByte hStateList, hStateCells;
	FontInfo fontInfo;
	
	if (Gestalt(gestaltSystemVersion, &response) || (response < 0x0700)) return;
	
	// Lock down the handles
	hStateList = HGetState((Handle) theList);
	HLock((Handle) theList);
	theListPtr = *theList;
	hStateList = HGetState((Handle) theListPtr->cells);
	HLock((Handle) theListPtr->cells);
	cellData = *(theListPtr->cells);
	
	GetFontInfo(&fontInfo);
	SetRect(&iconRect, cellRect->left + (theListPtr->cellSize.h >> 1) - 16,
			cellRect->top + ((theListPtr->cellSize.v - fontInfo.ascent) >> 1) - 16,
			cellRect->left + (theListPtr->cellSize.h >> 1) + 16,
			cellRect->top + ((theListPtr->cellSize.v - fontInfo.ascent) >> 1) + 16);
	SetPt(&drawPt, (iconRect.left + iconRect.right) >> 1, 
			iconRect.bottom + fontInfo.ascent + 2);

	switch (message) {
		case lInitMsg:
	  		break;

		case lDrawMsg:
			EraseRect(cellRect);
		case lHiliteMsg:
			
		  	if (dataLen > 0) {
		  		if (dataLen >= 4) {
		  			BlockMove(cellData + dataOffset, &iconHdl, 4);
		  			if (iconHdl)
		  				PlotIconSuite(&iconRect, 0, (hilited) ? ttSelected : 0, iconHdl);
		  			  else if (hilited) PaintRect(&iconRect);
		  			  else EraseRect(&iconRect);
		  			dataLen -= 4;
		  			dataOffset += 4;
		  			}
				
				// Condense if the text doesnt fit
				if (TextWidth(cellData, dataOffset, dataLen) >
						(cellRect->right - cellRect->left)) TextFace(condense);
		
				SetRect(&textRect, drawPt.h - (TextWidth(cellData, dataOffset, dataLen) >> 1) - 2,
						drawPt.v - fontInfo.ascent - 1,
						drawPt.h + (TextWidth(cellData, dataOffset, dataLen) >> 1) + 2,
						drawPt.v + 2);

				EraseRect(&textRect);
		  		MoveTo(drawPt.h - (TextWidth(cellData, dataOffset, dataLen) >> 1), drawPt.v);
				DrawText(cellData, dataOffset, dataLen);

				if (hilited) {			
				  	(* (char *) HiliteMode) ^= (1 << hiliteBit);
				  	InvertRect(&textRect);
				  	}
		  		}
		
		  	break;

		case lCloseMsg:
		  	break;
			}
	
	// Restore the Handles
	HSetState((Handle) theListPtr->cells,hStateCells);
	HSetState((Handle) theList, hStateList);
	}
