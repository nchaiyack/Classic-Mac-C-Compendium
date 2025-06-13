//--------------------------------------------------------------------------------
//
//	INIT notification routine
//	From an idea by Steve Capps
//	Asm version by: Paul Mercer, Darin Adler, and Paul Snively
//	C version by: Keith Rollin
//
//	Displays the icon (from the icon family) specified by iconID and
//	move the pen horizontally by moveX. Pass a -1 in moveX to move the standard
//	amount, moveX should be 40 for most icons’s.
//
//	1/12/92		kaar	If icon can’t be found, draw empty, framed, box (FRG).
//

typedef struct  {
	char privates[76];
	long randSeed;
	BitMap screenBits;
	Cursor arrow;
	Pattern dkGray;
	Pattern ltGray;
	Pattern gray;
	Pattern black;
	Pattern white;
	GrafPtr thePort;
} QDGlobals;

// A GREAT place to store 4 bytes (it was Darin’s idea)
short lmHorzOffset : 0x92C;			//
short lmHChecksum : 0x92E;			// A simple checksum of lmHorzOffset to
									//	determine first-timeness.

#define kHasCQDMask		0x0400	// Mask for testing ROM85 for color
#define kFirstX				8		// X coordinate of first icon to be drawn
#define kBottomEdge		8		// this far from bottom of screen
#define kIconSize			32		// size of icon (square normally)
#define kDefaultMoveX		40		// default amount to move icons
#define kChecksum			0x1021	// constant used for computing checksum
#define kMaskOffset			128		// offset to mask in ICN#
#define kIconRowBytes		(32/8)	// 32/8 bits

//
// Icon utilities stuff from Technote #306
//
typedef short	IconAlignmentType;
#define atNone				0x00
#define atVerticalCenter		0x01
#define atTop				0x02
#define atBottom			0x03
#define atHorizontalCenter	0x04
#define atAbsoluteCenter		(atVerticalCenter + atHorizontalCenter)
#define atCenterTop			(atTop + atHorizontalCenter)
#define atCenterBottom		(atBottom + atHorizontalCenter)
#define atLeft				0x08
#define atCenterLeft			(atVerticalCenter + atLeft)
#define atTopLeft			(atTop + atLeft)
#define atBottomLeft			(atBottom + atLeft)
#define atRight				0x0C
#define atCenterRight		(atVerticalCenter + atRight)
#define atTopRight			(atTop + atRight)
#define atBottomRight		(atBottom + atRight)

typedef short	IconTransformType;
#define ttNone				0x00
#define ttDisabled			0x01
#define ttOffline				0x02
#define ttOpen				0x03
#define ttSelected			0x4000
#define ttSelectedDisabled	(ttSelected + ttDisabled)
#define ttSelectedOffline		(ttSelected + ttOffline)
#define ttSelectedOpen		(ttSelected + ttOpen)

pascal OSErr	PlotIconID(Rect*, IconAlignmentType, IconTransformType, short)
		= {0x303C, 0x0500, 0xABC9};

void				Punt(QDGlobals *qdGlobals, Rect *destRect);

void main(short iconID, short moveX)
{
	long			fakeAppParms;
	long			oldA5;
	Boolean			hasColor;
	CGrafPort		myCPort;
	GrafPort			myPort;
	GrafPtr			myGrafPtr;
	Handle			iconHandle;
	Rect			destRect;
	BitMap			myBitMap;
	QDGlobals		qdGlobals;

	oldA5 = SetA5( (long) (CurrentA5 = (Ptr) &fakeAppParms));

	InitGraf(&qdGlobals.thePort);

	hasColor = ROM85 && kHasCQDMask == 0;
	if (hasColor) {
		OpenCPort(&myCPort);
		myGrafPtr = (GrafPtr) &myCPort;
	} else {
		OpenPort(&myPort);
		myGrafPtr = (GrafPtr) &myPort;
	}

	if (((lmHorzOffset << 1) ^ kChecksum) != lmHChecksum)
		lmHorzOffset = kFirstX;

	destRect.bottom = myPort.portRect.bottom - kBottomEdge;
	destRect.left = myPort.portRect.left + lmHorzOffset;
	destRect.top = destRect.bottom - kIconSize;
	destRect.right = destRect.left + kIconSize;

	if (SysVersion < 0x0700) {
		iconHandle = GetResource('ICN#', iconID);
		if (iconHandle != NULL) {
			HLock(iconHandle);
			myBitMap.rowBytes = kIconRowBytes;
			*(long *) &myBitMap.bounds.top = 0;
			*(long *) &myBitMap.bounds.bottom = kIconSize + kIconSize * 0x10000;
			myBitMap.baseAddr = *iconHandle + kMaskOffset;
			CopyBits(&myBitMap, &myGrafPtr->portBits, &myGrafPtr->portBits.bounds, &destRect,
					srcBic, NULL);

			myBitMap.baseAddr = *iconHandle;
			CopyBits(&myBitMap, &myGrafPtr->portBits, &myGrafPtr->portBits.bounds, &destRect,
					srcOr, NULL);

			HUnlock(iconHandle);
			ReleaseResource(iconHandle);
		} else {
			Punt(&qdGlobals, &destRect);
		}
	} else {
		if (PlotIconID(&destRect, atNone, ttNone, iconID) != noErr)
			Punt(&qdGlobals, &destRect);
	}

	lmHorzOffset += ((moveX == -1) ? kDefaultMoveX : moveX);
	lmHChecksum = (lmHorzOffset << 1) ^ kChecksum;

	if (hasColor)
		CloseCPort(&myCPort);
	else
		ClosePort(&myPort);		// Darin sez: “I think that QuickDraw leaves handles around.
								//			Too bad we can’t get rid of them...”

	SetA5( (long) (CurrentA5 = (Ptr) oldA5));
}

void	Punt(QDGlobals* qdGlobals, Rect* destRect)
{
	BackPat(qdGlobals->ltGray);
	EraseRect(destRect);
	FrameRect(destRect);
	SysBeep(5);
}
