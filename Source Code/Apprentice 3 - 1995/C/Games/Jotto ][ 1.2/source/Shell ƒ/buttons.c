#include "buttons.h"

static	RGBColor		gBlack4Bit={32768, 32768, 32768};
static	RGBColor		gWhite4Bit={60948, 60948, 60948};
static	RGBColor		gBackground4Bit={49152,49152,49152};
static	RGBColor		gBlack8Bit={26214, 26214, 39322};
static	RGBColor		gWhite8Bit={60948, 60948, 60948};
static	RGBColor		gBackground8Bit={52429, 52429, 52429};

#define TRIANGLE_SIZE	6

void Draw3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean isDown, Boolean drawTriangle)
{
	FontInfo		theFontInfo;
	Rect			tempRect;
	RGBColor		oldForeColor, oldBackColor;
	PixPatHandle	backgroundppat;
	short			theLineHeight;
	Rect			iconRect;
	RGBColor		myWhite={65535, 65535, 65535};
	
	TextFont(geneva);
	TextSize(9);
	TextFace(0);
	GetFontInfo(&theFontInfo);
	theLineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
	if (iconHandle!=0L)
	{
		iconRect.left=(buttonRect->right-buttonRect->left)/2+buttonRect->left-16+
			(isDown ? 1 : 0);
		iconRect.right=iconRect.left+32;
		iconRect.top=(buttonRect->bottom-buttonRect->top)/2+buttonRect->top-16+
			(isDown ? 1 : 0)-((theTitle!=0L) ? 5 : 0);
		iconRect.bottom=iconRect.top+32;
	}
	
	if (buttonDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
		
		backgroundppat=NewPixPat();
		MakeRGBPat(backgroundppat, (buttonDepth==4) ? &gBackground4Bit :
			&gBackground8Bit);
		SetRect(&tempRect, buttonRect->left+2, buttonRect->top+2,
			buttonRect->right-2, buttonRect->bottom-2);
		
		FillCRect(&tempRect, backgroundppat);

		RGBBackColor(&myWhite);
		RGBForeColor(isDown ? ((buttonDepth==4) ? &gBlack4Bit : &gBlack8Bit) :
			((buttonDepth==4) ? &gWhite4Bit : &gWhite8Bit));
		MoveTo(buttonRect->left, buttonRect->top);
		LineTo(buttonRect->right-2, buttonRect->top);
		MoveTo(buttonRect->left, buttonRect->top+1);
		LineTo(buttonRect->right-3, buttonRect->top+1);
		MoveTo(buttonRect->left, buttonRect->top+2);
		LineTo(buttonRect->left, buttonRect->bottom-2);
		MoveTo(buttonRect->left+1, buttonRect->top+2);
		LineTo(buttonRect->left+1, buttonRect->bottom-3);
		
		RGBForeColor(isDown ? ((buttonDepth==4) ? &gWhite4Bit : &gWhite8Bit) :
			((buttonDepth==4) ? &gBlack4Bit : &gBlack8Bit));
		MoveTo(buttonRect->right-1, buttonRect->top);
		LineTo(buttonRect->right-1, buttonRect->bottom-1);
		MoveTo(buttonRect->right-2, buttonRect->top+1);
		LineTo(buttonRect->right-2, buttonRect->bottom-1);
		MoveTo(buttonRect->left+2, buttonRect->bottom-2);
		LineTo(buttonRect->right-1, buttonRect->bottom-2);
		MoveTo(buttonRect->left+1, buttonRect->bottom-1);
		LineTo(buttonRect->right-1, buttonRect->bottom-1);
	}
	else
	{
		EraseRect(buttonRect);
		FrameRect(buttonRect);
		if (!isDown)
		{
			MoveTo(buttonRect->left+2, buttonRect->bottom-2);
			LineTo(buttonRect->right-2, buttonRect->bottom-2);
			LineTo(buttonRect->right-2, buttonRect->top+2);
		}
		else
		{
			MoveTo(buttonRect->left+1, buttonRect->bottom-3);
			LineTo(buttonRect->left+1, buttonRect->top+1);
			LineTo(buttonRect->right-3, buttonRect->top+1);
		}
	}
	
	if (theTitle!=0L)
	{
		ForeColor(blackColor);
		MoveTo(buttonRect->left+(buttonRect->right-buttonRect->left)/2-
			(StringWidth(theTitle)/2)+(isDown ? 1 : 0), ((iconHandle!=0L) ?
			buttonRect->bottom-6 : buttonRect->top+
			(buttonRect->bottom-buttonRect->top)/2+theLineHeight/2-2)+
			(isDown ? 1 : 0));
		DrawString(theTitle);
	}
	
	if (iconHandle!=0L)
	{
		if (buttonDepth>2)
			PlotCIcon(&iconRect, (CIconHandle)iconHandle);
		else
		{
			BitMap	iconMap;
			GrafPtr curPort;
			
			GetPort(&curPort);
			HLock( iconHandle);	/* lock data in place */
			
			iconMap.baseAddr = *iconHandle;	/* dereference the handle */
			iconMap.rowBytes = 4;			/* setup other fields */
			SetRect( &iconMap.bounds, 0,0,32,32);
			
			CopyBits( &iconMap, &(curPort->portBits),
						 &iconMap.bounds, &iconRect, srcOr, 0L );
			
			HUnlock(iconHandle);				/* all done; let it float */
		}
	}
	
	if (drawTriangle)
	{
		RgnHandle		triangleRgn;
		
		triangleRgn=NewRgn();
		OpenRgn();
			MoveTo(buttonRect->right-6+(isDown ? 1 : 0), buttonRect->top+5+(isDown ? 1 : 0));
			Line(-TRIANGLE_SIZE, TRIANGLE_SIZE);
			Line(-TRIANGLE_SIZE, -TRIANGLE_SIZE);
			Line(TRIANGLE_SIZE*2, 0);
		CloseRgn(triangleRgn);
		FillRgn(triangleRgn, &qd.black);
		DisposeRgn(triangleRgn);
	}
	
	if (buttonDepth>2)
	{
		DisposPixPat(backgroundppat);
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);	
	}
}

Boolean Track3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean drawTriangle)
{
	Point			mouseLoc;
	Boolean			buttonIsDown;
	
	buttonIsDown=FALSE;
	while (StillDown())
	{
		GetMouse(&mouseLoc);
		if (PtInRect(mouseLoc, buttonRect))
		{
			if (!buttonIsDown)
			{
				buttonIsDown=TRUE;
				Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth,
					TRUE, drawTriangle);
			}
		}
		else
		{
			if (buttonIsDown)
			{
				buttonIsDown=FALSE;
				Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth,
					FALSE, drawTriangle);
			}
		}
	}
	
	if (buttonIsDown)
		Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, FALSE, drawTriangle);
	
	return buttonIsDown;
}

void Hit3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, Boolean drawTriangle)
{
	long			dummy;
	
	Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, TRUE, drawTriangle);
	Delay(8, &dummy);
	Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, FALSE, drawTriangle);
}
