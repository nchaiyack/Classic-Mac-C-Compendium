// File "Icon LDEF.c" - Icon Plotting LDEF Routine that is System 6 and 7 compatible
//   Code is shamelessly based on Apple Snippet by Steven Falkenburg 5/23/91
//   This code is placed in the public domain for free use and distribution! MJS
 
#include <GestaltEqu.h>
#include <Icons.h>

// * **************************************************************************** * //
// * **************************************************************************** * //

pascal void	main(short message, Boolean hilited, Rect *cellRect, Cell theCell,
		short dataOffset, short dataLen, ListHandle theList) {
	short iconID, hasSys7;
	long response;
	Point drawPt;
	Rect iconRect, textRect;
	RgnHandle iconRgn;
	BitMap iconMap;
	Ptr cellData;
	Handle iconHdl;
	ListPtr theListPtr;
	SignedByte hStateList, hStateCells;
	FontInfo fontInfo;

	if (Gestalt(gestaltSystemVersion, &response)) return;
	hasSys7 = (response >= 0x0700);
	
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
		  		if (dataLen >= 2) {
		  			BlockMove(cellData + dataOffset, &iconID, sizeof(iconID));
		  			if (iconID == 0) EraseRect(&iconRect);
					  else if ((hasSys7 == 0) || PlotIconID(&iconRect, 0,
							(hilited) ? ttSelected : 0, iconID)) {
						if (iconHdl = GetResource('ICN#', iconID)) {
							HLock(iconHdl);

							iconMap.baseAddr = *iconHdl;
							iconMap.rowBytes = 4;
							SetRect(&iconMap.bounds,0,0,32,32);
							CopyBits(&iconMap, &theListPtr->port->portBits,
									&iconMap.bounds, &iconRect, srcCopy, 0);
							
							if (hilited) {		
								iconMap.baseAddr += 128;
								CopyBits(&iconMap, &theListPtr->port->portBits,
										&iconMap.bounds, &iconRect, srcXor, 0);
								}
								
							HUnlock(iconHdl);
							ReleaseResource(iconHdl);
							}
						  else EraseRect(&iconRect);
						}
		  			dataLen -= sizeof(iconID);
		  			dataOffset += sizeof(iconID);
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
