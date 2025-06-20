// File "indent ldef.c" - Snippet for LDEF that automatically indents text 9/6/93
//   Code is shamelessly based on Apple Snippet by Steven Falkenburg 5/23/91
//   This code is placed in the public domain for free use and distribution! MJS

// * **************************************************************************** * //
// * **************************************************************************** * //
// Spacing Constants

#define kLeftOffset	   2
#define kTopOffset	   0

// * **************************************************************************** * //

pascal void	main(short message, Boolean hilited, Rect *cellRect, Cell theCell,
		short dataOffset, short dataLen, ListHandle theList) {
	short leftDraw,topDraw;
	Ptr cellData;
	ListPtr theListPtr;
	SignedByte hStateList, hStateCells;
	FontInfo fontInfo;
	
	// Lock down the handles
	hStateList = HGetState((Handle) theList);
	HLock((Handle) theList);
	theListPtr = *theList;
	hStateList = HGetState((Handle) theListPtr->cells);
	HLock((Handle) theListPtr->cells);
	cellData = *(theListPtr->cells);
	
	switch (message) {
	  case lInitMsg:
	  	break;

	  case lDrawMsg:
		EraseRect(cellRect);
		
	  	if (dataLen > 0) {
	  		leftDraw = cellRect->left + theListPtr->indent.h + kLeftOffset;
	  		topDraw = cellRect->top + theListPtr->indent.v + kTopOffset;
	  		
	  		// Determine the indent and add it...
	  		while (cellData[dataOffset] == '\t') {
	  			dataOffset += 1;
	  			dataLen -= 1;
	  			leftDraw += 12;
	  			}

			GetFontInfo(&fontInfo);
			MoveTo(leftDraw, topDraw + fontInfo.ascent);
			
			// Condense if the text doesnt fit
			TextFace(0);
			if (TextWidth(cellData, dataOffset, dataLen) > (cellRect->right - leftDraw))
				TextFace(condense);

			DrawText(cellData, dataOffset, dataLen);
	  		}

		if (!hilited) break;
		
	  case lHiliteMsg:
	  	(* (char *) HiliteMode) ^= (1 << hiliteBit);
	  	InvertRect(cellRect);
	  	break;

	  case lCloseMsg:
	  	break;
		}
	
	HSetState((Handle) theListPtr->cells,hStateCells);
	HSetState((Handle) theList, hStateList);
	}
