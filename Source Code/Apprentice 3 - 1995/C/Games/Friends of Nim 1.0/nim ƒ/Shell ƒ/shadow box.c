#include "shadow box.h"
#include "offscreen layer.h"

#define kNumColors			8

enum { kWhite=0, kVeryLightGray, kLightGray, kMediumGray, kVeryMediumGray, kMediumDarkGray,
		kVeryDarkGray, kBlack };	/* colors for box */

static	short			gColorPercent[kNumColors]={100,93,66,46,26,13,6,0};
static	RGBColor		gWhiteColor={65535,65535,65535};
static	RGBColor		gBlackColor={0,0,0};
static	RGBColor		gColor[kNumColors];
static	Boolean			gColorsInitted=FALSE;

static	void DrawShadowBoxColor(Rect *boundsRect, Rect *availableRect);
static	void DrawShadowBoxBW(Rect *boundsRect, Rect *availableRect);
static	void InitColors(void);
static	void InitBlueColors(void);

OSErr Draw3DShadowBox(Rect *boundsRect, short theDepth, Rect *availableRect)
{
	if (theDepth>2)
	{
		if (!gColorsInitted)
			InitColors();
		DrawShadowBoxColor(boundsRect, availableRect);
	}
	else
	{
		DrawShadowBoxBW(boundsRect, availableRect);
	}
	
	return noErr;
}

static	void DrawShadowBoxColor(Rect *boundsRect, Rect *availableRect)
{
	short			i;
	short			l, r, t, b, w, h;
	Rect			theRect;
	
	l=boundsRect->left; r=boundsRect->right-1; t=boundsRect->top; b=boundsRect->bottom-1;
	w=r-l; h=b-t;
	
	/* EraseRect(boundsRect); */
	
	RGBForeColor(&gColor[kMediumGray]);
	for (i=0; i<8; i++)
	{
		MoveTo(l+i, t+7-i);
		Line(0, h-7);
	}
	
	RGBForeColor(&gColor[kVeryMediumGray]);
	for (i=0; i<8; i++)
	{
		MoveTo(l+1+i, b-i);
		Line(w-9, 0);
	}
	
	RGBForeColor(&gColor[kMediumDarkGray]);
	theRect.top=t;
	theRect.left=l+8;
	theRect.bottom=b-7;
	theRect.right=r+1;
	for (i=0; i<3; i++)
	{
		FrameRect(&theRect);
		InsetRect(&theRect, 1, 1);
	}
	
	RGBForeColor(&gColor[kLightGray]);
	MoveTo(l+11, b-11);
	Line(0, -(h-14));
	Line(w-14, 0);
	
	RGBForeColor(&gColor[kBlack]);
	Move(0, 1);
	Line(0, h-15);
	Line(-(w-15), 0);
	
	RGBForeColor(&gColor[kVeryMediumGray]);
	theRect.top=t+4;
	theRect.left=l+12;
	theRect.bottom=b-11;
	theRect.right=r-3;
	for (i=0; i<5; i++)
	{
		FrameRect(&theRect);
		InsetRect(&theRect, 1, 1);
	}
	
	RGBForeColor(&gColor[kBlack]);
	MoveTo(l+17, b-17);
	Line(0, -(h-26));
	Line(w-26, 0);
	
	RGBForeColor(&gColor[kLightGray]);
	Move(0, 1);
	Line(0, h-27);
	Line(-(w-27), 0);
	
	RGBForeColor(&gBlackColor);
	RGBBackColor(&gWhiteColor);
	
	SetRect(availableRect, l+18, t+10, r-9, b-17);
}

static	void DrawShadowBoxBW(Rect *boundsRect, Rect *availableRect)
{
	short			i;
	short			l, r, t, b, w, h;
	Rect			theRect;
	
	l=boundsRect->left; r=boundsRect->right-1; t=boundsRect->top; b=boundsRect->bottom-1;
	w=r-l; h=b-t;
	
	/* EraseRect(boundsRect); */
	
	PenPat(&qd.gray);
	for (i=0; i<8; i++)
	{
		MoveTo(l+i, t+7-i);
		Line(0, h-7);
	}
	
	for (i=0; i<8; i++)
	{
		MoveTo(l+1+i, b-i);
		Line(w-9, 0);
	}
	
	PenNormal();
	ForeColor(blackColor);
	theRect.top=t;
	theRect.left=l+8;
	theRect.bottom=b-7;
	theRect.right=r+1;
	for (i=0; i<3; i++)
	{
		FrameRect(&theRect);
		InsetRect(&theRect, 1, 1);
	}
	
	ForeColor(whiteColor);
	MoveTo(l+11, b-11);
	Line(0, -(h-14));
	Line(w-14, 0);
	
	ForeColor(blackColor);
	Move(0, 1);
	Line(0, h-15);
	Line(-(w-15), 0);
	
	ForeColor(blackColor);
	PenPat(&qd.gray);
	theRect.top=t+4;
	theRect.left=l+12;
	theRect.bottom=b-11;
	theRect.right=r-3;
	for (i=0; i<5; i++)
	{
		FrameRect(&theRect);
		InsetRect(&theRect, 1, 1);
	}
	
	PenNormal();
	ForeColor(blackColor);
	MoveTo(l+17, b-17);
	Line(0, -(h-26));
	Line(w-26, 0);
	
	ForeColor(whiteColor);
	Move(0, 1);
	Line(0, h-27);
	Line(-(w-27), 0);
	
	ForeColor(blackColor);
	
	SetRect(availableRect, l+18, t+10, r-9, b-17);
}

static	void InitColors(void)
{
	short			i;
	double			percent, color;
	
	for (i=0; i<kNumColors; i++)
	{
		percent=gColorPercent[i];
		percent/=100;
		color=65535*percent;
		gColor[i].red=gColor[i].green=gColor[i].blue=color;
	}
	gColorsInitted=TRUE;
}
