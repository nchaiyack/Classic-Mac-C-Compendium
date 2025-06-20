#include <Icons.h>

#define kLeftOffset	2
#define kTopOffset	0
#define kIconSpace	2

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
	Handle			iconHandle;
	Rect			iconRect;
	
	hStateList = HGetState((Handle) lHandle);
	HLock((Handle) lHandle);
	listPtr = *lHandle;
	hStateCells = HGetState(listPtr->cells);
	HLock(listPtr->cells);
	cellData = *(listPtr->cells);
	
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
		  		
	  			BlockMove(cellData+lDataOffset, &iconHandle, 4);
	  			if (iconHandle!=0L)
	  			{
					SetRect(&iconRect, leftDraw, topDraw+1, leftDraw+16, topDraw+17);
					PlotIconSuite(&iconRect, atAbsoluteCenter, ttNone, iconHandle);
				}
	  			lDataOffset += 4;
	  			lDataLen -= 4;
		  		leftDraw += 16+kIconSpace;
		  		
				TextFont(geneva);
				TextSize(9);
				TextMode(srcCopy);
				TextFace(0);
				
				GetFontInfo(&fontInfo);
				MoveTo(leftDraw,topDraw+1+fontInfo.ascent);
				
				DrawText(cellData,lDataOffset,lDataLen);
				
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
