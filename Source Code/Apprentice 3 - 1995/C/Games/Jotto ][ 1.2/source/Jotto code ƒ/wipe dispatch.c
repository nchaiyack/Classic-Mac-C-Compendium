#include "wipe dispatch.h"
#include "wipe headers.h"
#include "reversed wipe headers.h"

short			gWhichWipe;
short			gLastWipe;

void DoTheWipe(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr)
{
	Rect			theRect;
	
	theRect=sourceGrafPtr->portRect;
	OffsetRect(&theRect, -theRect.left, -theRect.top);
	ObscureCursor();
	switch(gWhichWipe)
	{
		case 1:		BoxOutWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 2:		RandomWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 3:		SpiralGyra(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 4:		CircularWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 5:		CasteWipeRL(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 6:		FourCorner(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 7:		BoxInWipe(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 8:		RippleWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 9:		MrDoOutdoneReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 10:	DiagonalWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 11:	CasteWipe(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 12:	HilbertWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 13:	FullScrollLR(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 14:	Skipaline(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 15:	SkipalineLR(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 16:	QuadrantWipe(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 17:	RescueRaiders(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 18:	QuadrantScroll(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 19:	HGR(sourceGrafPtr, destGrafPtr, theRect);					break;
		case 20:	CircleOut(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 21:	CircleIn(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 22:	CircleSerendipity(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 23:	CircleBulge(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 24:	QuadrantWipe2(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 25:	SlideWipe(sourceGrafPtr, destGrafPtr, theRect);				break;
		case 26:	FullScrollUD(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 27:	MrDo(sourceGrafPtr, destGrafPtr, theRect);					break;
		case 28:	MrDoOutdone(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 29:	QuadrantScroll2(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 30:	HGR2(sourceGrafPtr, destGrafPtr, theRect);					break;
		case 31:	HalvesScroll(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 32:	FullScrollUp(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 33:	CircularWipeReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 34:	CasteWipeRight(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 35:	FourCornerReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 36:	RippleWipeReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 37:	DiagonalWipeDownRight(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 38:	CasteWipeDown(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 39:	HilbertWipeReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 40:	SlideWipeReversed(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 41:	SkipalineLR2Pass(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 42:	QuadrantScroll2Reversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 43:	RescueRaidersReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 44:	QuadrantWipe2Reversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 45:	Skipaline2Pass(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 46:	FullScrollLeft(sourceGrafPtr, destGrafPtr, theRect);		break;
		case 47:	HalvesScrollReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 48:	QuadrantScrollReversed(sourceGrafPtr, destGrafPtr, theRect);break;
		case 49:	Skipaline2PassReversed(sourceGrafPtr, destGrafPtr, theRect);break;
		case 50:	HGRReversed(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 51:	MrDoReversed(sourceGrafPtr, destGrafPtr, theRect);			break;
		case 52:	QuadrantWipeReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 53:	SkipalineLR2PassReversed(sourceGrafPtr, destGrafPtr, theRect);	break;
		case 54:	HGR2Reversed(sourceGrafPtr, destGrafPtr, theRect);			break;
	}
	gLastWipe=gWhichWipe;
	gWhichWipe=0;
}
