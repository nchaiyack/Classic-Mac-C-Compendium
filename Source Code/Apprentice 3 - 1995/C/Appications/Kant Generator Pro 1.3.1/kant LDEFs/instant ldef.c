#define kLeftOffset	2
#define kRightOffset 2
#define kTopOffset	1

pascal void	main(short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle);

pascal void	main(short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle)
{
	FontInfo		fontInfo;
	ListPtr			listPtr;
	SignedByte		hStateList, hStateCells;
	Ptr				cellData;
	short			leftDraw, topDraw;
	Rect			fullCellRect;
	Ptr				tempTextPtr;
	
	hStateList = HGetState((Handle) lHandle);
	HLock((Handle) lHandle);
	listPtr = *lHandle;
	hStateCells = HGetState(listPtr->cells);
	HLock(listPtr->cells);
	cellData = *(listPtr->cells);
	LRect(&fullCellRect, lCell, lHandle);
	
	switch (lMessage)
	{
		case lInitMsg:
	  		break;
		case lDrawMsg:
			EraseRect(lRect);
			
		  	if (lDataLen > 0)
		  	{
		  		leftDraw =	lRect->left+listPtr->indent.h+kLeftOffset;
		  		topDraw =	lRect->top+listPtr->indent.v+kTopOffset;
		  		
				TextFont(geneva);
				TextSize(9);
				TextMode(srcCopy);
				TextFace(0);
				
				GetFontInfo(&fontInfo);
				MoveTo(leftDraw,topDraw+fontInfo.ascent);
				
				tempTextPtr=NewPtr(lDataLen);
				BlockMove(cellData+lDataOffset, tempTextPtr, lDataLen);
				TruncText(fullCellRect.right-fullCellRect.left-kLeftOffset-kRightOffset,
					tempTextPtr, &lDataLen, smTruncEnd);
	  			DrawText(tempTextPtr, 0, lDataLen);
	  			DisposePtr(tempTextPtr);
	  			
				TextFont(GetSysFont());
				TextSize(GetDefFontSize());
		  	}
	
			if (!lSelect)
		  		break;
			
	  case lHiliteMsg:
	  	/* do hilite color */
	  	LMSetHiliteMode(LMGetHiliteMode()^(1<<hiliteBit));
	  	InvertRect(lRect);
	  	break;

	  case lCloseMsg:
	  	break;
	}
	
	HSetState(listPtr->cells, hStateCells);
	HSetState((Handle)lHandle, hStateList);
}
