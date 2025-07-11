/*	SICN LDEF

	�1991 Apple Computer, Inc.
	written by Steven Falkenburg 5/23/91
	This LDEF displays small icons to the left of text in a list.
	
	The small icon is stored in the first 32 bytes of each cell.
	
	modified by Matt Slot, 9/6/93
		Fixed odd address problem in the SICN plotting
*/


/* constants for spacing */

#define kLeftOffset	2
#define kTopOffset	0
#define kIconSpace	2

/* prototypes */

void DrawSICN(Ptr theSICN,short left,short top,GrafPtr drawPort);

/* main LDEF entry point */

pascal void	main(short lMessage,Boolean lSelect,Rect *lRect,Cell lCell,
				short lDataOffset,short lDataLen,ListHandle lHandle)
{
	FontInfo fontInfo;						/* font information (ascent/descent/etc) */
	ListPtr listPtr;						/* pointer to store dereferenced list */
	SignedByte hStateList,hStateCells;		/* state variables for HGetState/SetState */
	Ptr cellData;							/* points to start of cell data for list */
	Ptr theSICN;							/* points to SICN to be drawn */
	short leftDraw,topDraw;					/* left/top offsets from topleft of cell */
	
	#pragma unused (lCell)
	
	theSICN = NewPtr(32);
	
	/* lock and dereference list mgr handles */
	
	hStateList = HGetState((Handle) lHandle);
	HLock((Handle) lHandle);
	listPtr = *lHandle;
	hStateList = HGetState(listPtr->cells);
	HLock(listPtr->cells);
	cellData = *(listPtr->cells);
	
	switch (lMessage) {
	  case lInitMsg:
	  	/* we don't need any initialization */
	  	break;

	  case lDrawMsg:
		EraseRect(lRect);
		
	  	if (lDataLen > 0) {
	  	
	  		/* determine starting point for drawing */
	  		
	  		leftDraw =	lRect->left+listPtr->indent.h+kLeftOffset;
	  		topDraw =	lRect->top+listPtr->indent.v+kTopOffset;
	  		
	  		/* plot SICN (first 32 bytes) */
	  		
	  		if (lDataLen > 32) {
	  			BlockMove(cellData+lDataOffset, theSICN, 32);
	  			DrawSICN(theSICN,leftDraw,topDraw,listPtr->port);
	  			lDataOffset += 32;
	  			lDataLen -= 32;
	  		}
	  		leftDraw += 16+kIconSpace;
	  		
	  		/* plot text (offset 32 bytes onward) */
	  		
			GetFontInfo(&fontInfo);
			MoveTo(leftDraw,topDraw+fontInfo.ascent);
			
			/* set condensed mode if necessary (if the text doesn't fit otherwise) */
			
			TextFace(0);
			if (TextWidth(cellData,lDataOffset,lDataLen) > (lRect->right - leftDraw))
				TextFace(condense);

			DrawText(cellData,lDataOffset,lDataLen);
	  	}

		if (!lSelect)
	  		break;
		
	  case lHiliteMsg:
	  	/* do hilite color */
	  	(* (char *) HiliteMode) ^= (1 << hiliteBit);
	  	InvertRect(lRect);
	  	break;

	  case lCloseMsg:
	  	break;
	}
	
	DisposePtr(theSICN);
	HSetState(listPtr->cells,hStateCells);
	HSetState((Handle) lHandle,hStateList);
}


/* this procedure draws a small icon using CopyBits */

void DrawSICN(Ptr theSICN,short left,short top,GrafPtr drawPort)
{
	BitMap iconMap;
	Rect destRect;

	iconMap.baseAddr = theSICN;
	iconMap.rowBytes = 2;
	SetRect(&iconMap.bounds,0,0,16,16);
	SetRect(&destRect,0,0,16,16);
	OffsetRect(&destRect,left,top);
	CopyBits(&iconMap,&drawPort->portBits,&iconMap.bounds,&destRect,
			srcCopy,nil);
}
