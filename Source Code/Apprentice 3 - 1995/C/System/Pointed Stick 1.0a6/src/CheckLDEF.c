//
// Icon utilities stuff from Technote #306
//
typedef short	IconAlignmentType;
#define atNone				0x00
#define atVerticalCenter	0x01
#define atTop				0x02
#define atBottom			0x03
#define atHorizontalCenter	0x04
#define atAbsoluteCenter	(atVerticalCenter + atHorizontalCenter)
#define atCenterTop			(atTop + atHorizontalCenter)
#define atCenterBottom		(atBottom + atHorizontalCenter)
#define atLeft				0x08
#define atCenterLeft		(atVerticalCenter + atLeft)
#define atTopLeft			(atTop + atLeft)
#define atBottomLeft		(atBottom + atLeft)
#define atRight				0x0C
#define atCenterRight		(atVerticalCenter + atRight)
#define atTopRight			(atTop + atRight)
#define atBottomRight		(atBottom + atRight)

typedef short	IconTransformType;
#define ttNone				0x00
#define ttDisabled			0x01
#define ttOffline			0x02
#define ttOpen				0x03
#define ttSelected			0x4000
#define ttSelectedDisabled	(ttSelected + ttDisabled)
#define ttSelectedOffline	(ttSelected + ttOffline)
#define ttSelectedOpen		(ttSelected + ttOpen)

pascal OSErr	PlotIconID(Rect*, IconAlignmentType, IconTransformType, short)
		= {0x303C, 0x0500, 0xABC9};

void				PlotCheckBox(Rect *lRect, ListHandle lHandle, Boolean lSelect);

//--------------------------------------------------------------------------------

pascal void main(	short		lMessage,		// what operation to do
					Boolean		lSelect,		// draw it selected?
					Rect*		lRect,			// where to draw the item
					Cell		lCell,			// which cell to draw
					short		lDataOffset,	// offset to data for drawing
					short		lDataLen,		// length of that data
					ListHandle	lHandle)		// handle to list record
{
	FontInfo		info;
	ListPtr			listPtr;
	Rect			iconRect;
	Point			textLocation;
	DataHandle		listCells;
	unsigned char	oldState;

	switch (lMessage) {
		case lInitMsg:
			GetFontInfo(&info);
			listPtr = *lHandle;
			if (listPtr->cellSize.v < 16)
				listPtr->cellSize.v = 16;
			listPtr->indent.h = listPtr->cellSize.v - info.leading;
			listPtr->indent.v = listPtr->indent.h - info.descent;
			break;

		case lDrawMsg:

			PenNormal();
			EraseRect(lRect);
		
			PlotCheckBox(lRect, lHandle, lSelect);

			if (lDataLen > 0) {
				textLocation.h = 4 + 16 + 4;
				textLocation.v = lRect->top + (**lHandle).indent.v;
				MoveTo(textLocation.h, textLocation.v);

				listCells = (**lHandle).cells;
				oldState = HGetState(listCells);
				HLock(listCells);
				DrawText((*listCells) + lDataOffset, 0, lDataLen);
				HSetState(listCells, oldState);
			}
			
			break;

		case lHiliteMsg:
			PlotCheckBox(lRect, lHandle, lSelect);
			break;
	}
}


void	PlotCheckBox(Rect* lRect, ListHandle lHandle, Boolean lSelect)
{
	Rect	iconRect;
	long	refCon;
	short	iconID;

	iconRect.left = 4;
	iconRect.bottom = lRect->top + (**lHandle).indent.h;
	iconRect.right = iconRect.left + 16;
	iconRect.top = iconRect.bottom - 16;
	refCon = (**lHandle).refCon;
	iconID = (lSelect ? HiWord(refCon) : (short) refCon);
	PlotIconID(&iconRect, atNone, ttNone, iconID);
}
