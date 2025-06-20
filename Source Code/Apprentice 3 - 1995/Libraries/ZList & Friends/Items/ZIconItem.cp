/*
	ZIconItem
		: ZListItem
	�1994 Chris K. Thomas.  All Rights Reserved.
	
	ZIconItem draws an icon, with an optional name appended
	to the bottom.
	
	??? Dragging isn't implemented yet
*/

#include "ZIconItem.h"

static void MaxThresholdRect(Rect &r,short hThresh,short vThresh);

ZIconItem *ZIconItem::CreateZIconItemFromData(FlavorType inType,Ptr inData)
{
	if(inType == flavorTypePromiseHFS||
		inType == flavorTypeHFS ||
		inType == flavorTypeDirectory && inData != NULL)
	{
		
	}
	return NULL;
}

void ZIconItem::Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen)
{
	*outOurFlavor = 'NULL'; *outOurData = NULL; *outDataLen = 0L;
}

ZIconItem::ZIconItem(Int16 inIconID,Str31& inIconName,ZList *inList)
{
	mIconID = inIconID;
	if(inIconName[0]!=0)
		BlockMoveData(&inIconName[0],&mIconName[0],inIconName[0]+1);
	else
		mIconName[0]=0;
		
	if(inList)
	{
		inList->AddItem(inList->GetNumItems()+1,this);
	}	
}

void
ZIconItem::Draw(Rect &inRect)
{
	Rect		centRect=inRect;
	RGBColor	foreColor,backColor;
	
	GetForeColor(&foreColor);
	GetBackColor(&backColor);
	
	MaxThresholdRect(centRect,32,32);
	
	if(mIconName[0] > 0)
	{
		OffsetRect(&centRect,0,-6);
		if(itemSelected)
		{
			ForeColor(whiteColor);
			BackColor(blackColor);
		}
		else
		{
			ForeColor(blackColor);
			BackColor(whiteColor);
		}
		
		TextFont(geneva);
		TextMode(srcCopy);
		TextSize(9);
		
		short strWidth = TextWidth(mIconName,1, mIconName[0]);
		short inRectCenter = inRect.left+((inRect.right-inRect.left)/2);
		
		MoveTo(inRectCenter - (strWidth/2),centRect.bottom+10);
		DrawString(mIconName);
	}
	
	PlotIconID(&centRect,atNone,itemSelected ? ttSelected : ttNone, mIconID);
	
	RGBForeColor(&foreColor);
	RGBBackColor(&backColor);
}

void
ZIconItem::Click(EventRecord& /*event*/, Rect& /*r*/)
{
	SysBeep(32);
}

/*
	MaxThresholdRect
	if r is wider/taller than the given thresholds,
	shrink it in place, leaving it centered.
	
	Sep 22 94 CKT Created
*/

static void MaxThresholdRect(Rect &r,short hThresh,short vThresh)
{
	short	hDelta,vDelta;
	
	hDelta = r.right-r.left;
	vDelta = r.bottom-r.top;
	
	if(hDelta > hThresh)
	{
		r.right	-= (hDelta-hThresh)/2 ;//- hDelta;
		r.left	+= (hDelta-hThresh)/2 ;//- hDelta;
	}
	
	if(vDelta > vThresh)
	{
		r.bottom-= (vDelta-vThresh)/2 ;//- vDelta;
		r.top	+= (vDelta-vThresh)/2 ;//- vDelta;
	}
}