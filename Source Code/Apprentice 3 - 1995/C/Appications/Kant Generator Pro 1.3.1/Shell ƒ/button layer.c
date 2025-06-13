#include "button layer.h"
#include "text layer.h"
#include <Icons.h>

static	RGBColor		gDkGray4={16384, 16384, 16384};
static	RGBColor		gLtGray4={43691, 43691, 43691};
static	RGBColor		gMediumGray4={32767, 32767, 32767};
static	RGBColor		gMediumGray8={26214, 26214, 39322};
static	RGBColor		gVeryLtGray8={60948, 60948, 60948};
static	RGBColor		gLtGray8={52428, 52428, 52428};
static	RGBColor		gLtBlue8={52428, 52428, 65535};
static	RGBColor		gDkGray8={26214, 26214, 26214};
static	RGBColor		gDkBlue8={39321, 39321, 52428};

static	RGBColor		gBlack={0, 0, 0};
static	RGBColor		gWhite={65535, 65535, 65535};

#define TRIANGLE_SIZE	6

void Draw3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType)
{
	Rect			tempRect, defaultRect;
	RGBColor		oldForeColor, oldBackColor;
	short			theLineHeight;
	Rect			iconRect;
	Boolean			isDown, isDimmed;
	
	isDown=(buttonType & kButtonIsDown) ? TRUE : FALSE;
	isDimmed=(buttonType & kButtonIsDimmed) ? TRUE : FALSE;
	if (buttonType & kButtonUseDefaultFont)
	{
		theLineHeight=CalculateFontHeight(MyGetDefaultFont(), MyGetDefaultFontSize());
		UseDefaultDrawingFont();
	}
	else
	{
		theLineHeight=CalculateFontHeight(geneva, 9);
		SetTheDrawingFont(geneva, 9, 0, 0);
	}
	
	TextMode(srcOr);
	
	if (iconHandle!=0L)
	{
		iconRect.left=(buttonRect->right-buttonRect->left)/2+buttonRect->left-8+
			(isDown ? 1 : 0);
		iconRect.right=iconRect.left+16;
		iconRect.top=(buttonRect->bottom-buttonRect->top)/2+buttonRect->top-8+
			(isDown ? 1 : 0)-((theTitle!=0L) ? 5 : 0);
		iconRect.bottom=iconRect.top+16;
	}
	
	if (buttonDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
		RGBBackColor(&gWhite);
		
		if (buttonType & kButtonIsDimmed)
			SetRect(&tempRect, buttonRect->left+1, buttonRect->top+1,
				buttonRect->right-1, buttonRect->bottom-1);
		else
			SetRect(&tempRect, buttonRect->left+2, buttonRect->top+2,
				buttonRect->right-2, buttonRect->bottom-2);
		
		if (buttonType & kRoundRectButton)
		{
			if (buttonType & kButtonIsDefault)
			{
				defaultRect=*buttonRect;
				PenSize(3, 3);
				InsetRect(&defaultRect, -4, -4);
				FrameRoundRect(&defaultRect, 16, 16);
				PenNormal();
			}
			FrameRoundRect(buttonRect, 10, 10);
			InsetRect(&tempRect, 1, 1);
			RGBForeColor((buttonDepth==4) ? &gLtGray4 : &gLtGray8);
			PaintRect(&tempRect);
			if (buttonType & kButtonIsDimmed)
			{
// more here
			}
			else
			{
				RGBForeColor(isDown ? ((buttonDepth==4) ? &gDkGray8 : &gMediumGray4) : &gWhite);
				MoveTo(buttonRect->left+1, buttonRect->top+3);
				LineTo(buttonRect->left+1, buttonRect->bottom-4);
				MoveTo(buttonRect->left+2, buttonRect->top+2);
				Line(0, 0);
				MoveTo(buttonRect->left+3, buttonRect->top+1);
				LineTo(buttonRect->right-4, buttonRect->top+1);
				RGBForeColor((buttonDepth==4) ? &gLtGray4 :
					isDown ? &gDkBlue8 : &gLtBlue8);
				MoveTo(buttonRect->right-4, buttonRect->top+2);
				LineTo(buttonRect->left+3, buttonRect->top+2);
				MoveTo(buttonRect->left+2, buttonRect->top+3);
				LineTo(buttonRect->left+2, buttonRect->bottom-4);
				RGBForeColor((buttonDepth==4) ? &gLtGray4 :
					isDown ? &gLtBlue8 : &gDkBlue8);
				MoveTo(buttonRect->right-3, buttonRect->top+2);
				LineTo(buttonRect->right-3, buttonRect->bottom-4);
				MoveTo(buttonRect->left+2, buttonRect->bottom-3);
				LineTo(buttonRect->right-4, buttonRect->bottom-3);
				RGBForeColor(isDown ? &gWhite : ((buttonDepth==4) ? &gMediumGray4 : &gDkGray8));
				MoveTo(buttonRect->right-2, buttonRect->top+3);
				LineTo(buttonRect->right-2, buttonRect->bottom-4);
				MoveTo(buttonRect->right-3, buttonRect->bottom-3);
				Line(0, 0);
				MoveTo(buttonRect->right-4, buttonRect->bottom-2);
				LineTo(buttonRect->left+3, buttonRect->bottom-2);
			}
		}
		else if (buttonType & kRadioButton)
		{
			RGBForeColor((buttonDepth==4) ? &gLtGray4 : &gLtGray8);
			PaintOval(&tempRect);
			InsetRect(&tempRect, -1, -1);
			RGBForeColor(&gBlack);
			FrameOval(&tempRect);
		}
		else if (buttonType & kSquareButton)
		{
			RGBForeColor((buttonDepth==4) ? &gLtGray4 : &gLtGray8);
			PaintRect(&tempRect);
			if (buttonType & kButtonIsDimmed)
			{
				RGBForeColor((buttonDepth==4) ? &gMediumGray4 : &gVeryLtGray8);
				MoveTo(buttonRect->left, buttonRect->top);
				LineTo(buttonRect->right-1, buttonRect->top);
				LineTo(buttonRect->right-1, buttonRect->bottom-1);
				LineTo(buttonRect->left, buttonRect->bottom-1);
				LineTo(buttonRect->left, buttonRect->top);
			}
			else
			{
				RGBForeColor(isDown ? ((buttonDepth==4) ? &gDkGray4 : &gMediumGray8) :
					((buttonDepth==4) ? &gWhite : &gVeryLtGray8));
				MoveTo(buttonRect->left, buttonRect->top);
				LineTo(buttonRect->right-2, buttonRect->top);
				MoveTo(buttonRect->left, buttonRect->top+1);
				LineTo(buttonRect->right-3, buttonRect->top+1);
				MoveTo(buttonRect->left, buttonRect->top+2);
				LineTo(buttonRect->left, buttonRect->bottom-2);
				MoveTo(buttonRect->left+1, buttonRect->top+2);
				LineTo(buttonRect->left+1, buttonRect->bottom-3);
				
				RGBForeColor(isDown ? ((buttonDepth==4) ? &gWhite : &gVeryLtGray8) :
					((buttonDepth==4) ? &gDkGray4 : &gMediumGray8));
				MoveTo(buttonRect->right-1, buttonRect->top);
				LineTo(buttonRect->right-1, buttonRect->bottom-1);
				MoveTo(buttonRect->right-2, buttonRect->top+1);
				LineTo(buttonRect->right-2, buttonRect->bottom-1);
				MoveTo(buttonRect->left+2, buttonRect->bottom-2);
				LineTo(buttonRect->right-1, buttonRect->bottom-2);
				MoveTo(buttonRect->left+1, buttonRect->bottom-1);
				LineTo(buttonRect->right-1, buttonRect->bottom-1);
			}
		}
	}
	else
	{
		if (buttonType & kRoundRectButton)
		{
			EraseRect(buttonRect);
			if (buttonType & kButtonIsDefault)
			{
				defaultRect=*buttonRect;
				PenSize(3, 3);
				InsetRect(&defaultRect, -4, -4);
				FrameRoundRect(&defaultRect, 16, 16);
				PenNormal();
			}
			FrameRoundRect(buttonRect, 10, 10);
			if (isDown)
			{
				MoveTo(buttonRect->left+1, buttonRect->top+3);
				LineTo(buttonRect->left+1, buttonRect->bottom-4);
				MoveTo(buttonRect->left+2, buttonRect->top+2);
				Line(0, 0);
				MoveTo(buttonRect->left+3, buttonRect->top+1);
				LineTo(buttonRect->right-4, buttonRect->top+1);
			}
			else
			{
				MoveTo(buttonRect->right-2, buttonRect->top+3);
				LineTo(buttonRect->right-2, buttonRect->bottom-4);
				MoveTo(buttonRect->right-3, buttonRect->bottom-3);
				Line(0, 0);
				MoveTo(buttonRect->right-4, buttonRect->bottom-2);
				LineTo(buttonRect->left+3, buttonRect->bottom-2);
			}
		}
		else if (buttonType & kRadioButton)
		{
			SetRect(&tempRect, buttonRect->left+1, buttonRect->top+1,
				buttonRect->right-1, buttonRect->bottom-1);
			EraseRect(buttonRect);
			FrameOval(&tempRect);
		}
		else if (buttonType & kSquareButton)
		{
			EraseRect(buttonRect);
			if (buttonType & kButtonIsDimmed)
				PenPat(&qd.gray);
			FrameRect(buttonRect);
			PenNormal();
			if ((buttonType & kButtonIsDimmed)==0)	/* not dimmed */
			{
				if (isDown==0)	/* not down */
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
		}
	}
	
	if (theTitle!=0L)
	{
		short			textX, textY;
		
		textX=buttonRect->left+(buttonRect->right-buttonRect->left)/2-
			(StringWidth(theTitle)/2)+(isDown ? 1 : 0)+((buttonType & kButtonUseDefaultFont) ? 0 : 1);
		textY=((iconHandle!=0L) ?
			buttonRect->bottom-6 : buttonRect->top+
			(buttonRect->bottom-buttonRect->top)/2+(theLineHeight/2)-2-
			((buttonType & kButtonUseDefaultFont) ? 2 : 0)+
			(isDown ? 1 : 0));
		if ((buttonDepth>2) && ((buttonType & kButtonIsDimmed)==0))
		{
			MoveTo(textX+1, textY+1);
			RGBForeColor((buttonDepth==4) ? &gWhite : &gVeryLtGray8);
			DrawString(theTitle);
		}
		if ((buttonDepth>2) && (buttonType & kButtonIsDimmed))
			ForeColor(whiteColor);
		else
			ForeColor(blackColor);
		MoveTo(textX, textY);
		DrawString(theTitle);
		ForeColor(blackColor);
	}
	
	if (iconHandle!=0L)
	{
		PlotIconSuite(&iconRect, atAbsoluteCenter, isDimmed ? ttDisabled : ttNone, iconHandle);
	}
	
	if (buttonType & kButtonIsPopup)
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
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);	
	}
	
	UseDefaultDrawingFont();
}

Boolean Track3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType)
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
				Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, buttonType+kButtonIsDown);
			}
		}
		else
		{
			if (buttonIsDown)
			{
				buttonIsDown=FALSE;
				Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, buttonType);
			}
		}
	}
	
	if (buttonIsDown)
		Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, buttonType);
	
	return buttonIsDown;
}

void Hit3DButton(Rect *buttonRect, unsigned char *theTitle,
	Handle iconHandle, short buttonDepth, short buttonType)
{
	long			dummy;
	
	Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, buttonType+kButtonIsDown);
	Delay(8, &dummy);
	Draw3DButton(buttonRect, theTitle, iconHandle, buttonDepth, buttonType);
}

