#include "nim grid.h"
#include "nim globals.h"
#include <Icons.h>

void DrawGridPosition(Boolean isLarge, Rect *boundsRect, Point pos, Boolean player, short maxRows)
{
	Rect			gridRect;
	
	RectFromPosition(isLarge, pos, boundsRect, &gridRect, maxRows);
	PlotIconSuite(&gridRect, atAbsoluteCenter, ttNone,
		(player==kFirstPlayer) ? gNimBitFirstSelectedHandle : gNimBitSecondSelectedHandle);
}

void DrawNeutralGridPosition(Boolean isLarge, Rect *boundsRect, Point pos, short maxRows)
{
	Rect			gridRect;
	
	RectFromPosition(isLarge, pos, boundsRect, &gridRect, maxRows);
	PlotIconSuite(&gridRect, atAbsoluteCenter, ttNone, gNimBitIconHandle);
}

void DrawGrid(Boolean isLarge, short theDepth, Rect *boundsRect, short maxRow, short maxCol)
{
	RGBColor		oldForeColor, oldBackColor;
	RGBColor		color8={60947, 60947, 60947};
	RGBColor		color4={49151, 49151, 49151};
	RGBColor		myForeColor={65535, 65535, 65535};
	short			row, col;
	Rect			gridRect;
	short			columnWidth, rowHeight;
	
	columnWidth=isLarge ? kLargeGridColumnWidth : kGridColumnWidth;
	rowHeight=isLarge ? kLargeGridRowHeight : kGridRowHeight;
	
	if (theDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
		myForeColor=(theDepth==4) ? color4 : color8;
		RGBForeColor(&myForeColor);
	}
	
	for (row=0; row<maxRow; row++)
	{
		gridRect.top=boundsRect->top+rowHeight*row;
		gridRect.bottom=gridRect.top+rowHeight-1;
		for (col=0; col<maxCol; col++)
		{
			gridRect.left=boundsRect->left+columnWidth*col;
			gridRect.right=gridRect.left+columnWidth-1;
			MoveTo(gridRect.right, gridRect.top+1);
			LineTo(gridRect.right, gridRect.bottom);
			LineTo(gridRect.left+1, gridRect.bottom);
		}
	}
	
	if (theDepth>2)
	{
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
	}
}

Point PositionFromPoint(Boolean isLarge, Point thePoint, Rect *boundsRect, short maxRows)
{
	Point			badPoint={-1,-1};
	
	thePoint.h-=boundsRect->left;
	thePoint.v-=boundsRect->top;
	if ((thePoint.h<0) || (thePoint.v<0))
		return badPoint;
	thePoint.h=thePoint.h/(isLarge ? kLargeGridColumnWidth : kGridColumnWidth);
	thePoint.v=thePoint.v/(isLarge ? kLargeGridRowHeight : kGridRowHeight);
	thePoint.v=maxRows-thePoint.v-1;
	
	return thePoint;
}

void RectFromPosition(Boolean isLarge, Point pos, Rect *boundsRect, Rect *gridRect, short maxRows)
{
	if (isLarge)
	{
		gridRect->top=boundsRect->top+kLargeGridRowHeight*(maxRows-pos.v-1)-2;
		gridRect->bottom=gridRect->top+32;
		gridRect->left=boundsRect->left+kLargeGridColumnWidth*pos.h-2;
		gridRect->right=gridRect->left+32;
	}
	else
	{
		gridRect->top=boundsRect->top+kGridRowHeight*(maxRows-pos.v-1)+1;
		gridRect->bottom=gridRect->top+16;
		gridRect->left=boundsRect->left+kGridColumnWidth*pos.h+1;
		gridRect->right=gridRect->left+16;
	}
}
