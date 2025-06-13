#include "turbo peano.h"

#define kNumX			44
#define kNumY			17
#define kNumDirection	8

#define kLen			2
#define kStartLevel		2

#define kStartX			(1+6*kLen)
#define kStartY			(theWindow->portRect.bottom-6*kLen)

char	gXPattern[kNumX]=
	{
		'd', 'd', 'r', 'y', 'l', 'l', 'd', 'l', 'l', 'y',
		'r', 'd', 'd', 'r', 'y', 'l', 'l', 'd', 'l', 'l',
		'y', 'r', 'd', 'd', 'r', 'y', 'l', 'l', 'd', 'l',
		'l', 'y', 'r', 'd', 'd', 'r', 'y', 'l', 'l', 'd',
		'l', 'l', 'y', 'r'
	};
char	gYPattern[kNumY]=
	{
		'y', 'r', 'd', 'd', 'r', 'y', 'l', 'l', 'd', 'l', 'l', 'y', 'r', 'd', 'd', 'r', 'y'
	};

enum	/* direction */
{
	kRight=0,
	kDownRight,
	kDown,
	kDownLeft,
	kLeft,
	kUpLeft,
	kUp,
	kUpRight
};

static	void Recurse(short level, char whichpattern);

static	short			direction;

void DrawPeano(short theDepth)
{
	RGBColor		oldForeColor, oldBackColor;
	RGBColor		color8={16383, 16383, 16383};
	RGBColor		color4={16383, 16383, 16383};
	RGBColor		myForeColor={65535, 65535, 65535};
	Rect			sourceRect, destRect, nextSourceRect, centerRect;
	WindowRef		theWindow;
	
	GetPort(&theWindow);
	if (theDepth<=2)
	{
		EraseRect(&(theWindow->portRect));
		return;
	}
	
	FillRect(&(theWindow->portRect), &qd.black);
	if (theDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
		myForeColor=(theDepth==4) ? color4 : color8;
	}
	
	direction=kRight;
	MoveTo(kStartX, kStartY);
	if (theDepth>2)
		RGBForeColor(&myForeColor);
	Recurse(kStartLevel, 'x');
	if (theDepth>2)
		RGBForeColor(&oldForeColor);
	SetRect(&sourceRect, kStartX-6*kLen, kStartY-9*kLen, kStartX+8*kLen+1, kStartY+5*kLen+1);
	nextSourceRect=sourceRect;
	while (sourceRect.right-sourceRect.left<theWindow->portRect.right-theWindow->portRect.left)
	{
		destRect=sourceRect;
		OffsetRect(&destRect, 0, -(sourceRect.bottom-sourceRect.top-1+2*kLen));
		nextSourceRect.top=destRect.top;
		CopyBits(&(theWindow->portBits), &(theWindow->portBits), &sourceRect, &destRect, 0, 0L);
		OffsetRect(&destRect, sourceRect.right-sourceRect.left-1+2*kLen, 0);
		destRect.bottom=sourceRect.bottom;
		sourceRect.top=destRect.top;
		nextSourceRect.right=destRect.right;
		CopyBits(&(theWindow->portBits), &(theWindow->portBits), &sourceRect, &destRect, 0, 0L);
		centerRect=nextSourceRect;
		InsetRect(&centerRect, (centerRect.right-centerRect.left)/2-2*kLen, (centerRect.bottom-centerRect.top)/2-2*kLen);
		FillRect(&centerRect, &qd.black);
		if (theDepth>2)
			RGBForeColor(&myForeColor);
		MoveTo(centerRect.left+kLen, centerRect.bottom-1);
		LineTo(centerRect.right-1-kLen, centerRect.bottom-1);
		MoveTo(centerRect.right-1, centerRect.bottom-1-kLen);
		LineTo(centerRect.right-1, centerRect.top+kLen);
		MoveTo(centerRect.right-1-kLen, centerRect.top);
		LineTo(centerRect.left+kLen, centerRect.top);
		MoveTo(centerRect.left, centerRect.top+kLen);
		LineTo(centerRect.left, centerRect.bottom-1-kLen);
		if (theDepth>2)
			RGBForeColor(&oldForeColor);
		sourceRect=nextSourceRect;
	}
	
	if (theDepth>2)
	{
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
	}
}

static	void Recurse(short level, char whichpattern)
{
	short		i;
	char		theChar;
	
	for (i=0; i<((whichpattern=='x') ? kNumX : kNumY); i++)
	{
		theChar=(whichpattern=='x') ? gXPattern[i] : gYPattern[i];
		switch (theChar)
		{
			case 'x':
			case 'y':
				if (level>1) Recurse(level-1, theChar);
				else theChar='d';
				break;
		}
		
		switch (theChar)
		{
			case 'l':
				direction--;
				if (direction<0) direction=kNumDirection-1;
				break;
			case 'r':
				direction++;
				if (direction==kNumDirection) direction=0;
				break;
			case 'd':
				switch (direction)
				{
					case kRight:
						Line(kLen, 0);
						break;
					case kDownRight:
						Line(kLen, kLen);
						break;
					case kDown:
						Line(0, kLen);
						break;
					case kDownLeft:
						Line(-kLen, kLen);
						break;
					case kLeft:
						Line(-kLen, 0);
						break;
					case kUpLeft:
						Line(-kLen, -kLen);
						break;
					case kUp:
						Line(0, -kLen);
						break;
					case kUpRight:
						Line(kLen, -kLen);
						break;
				}
				break;
			case 'x':
			case 'y':
				break;
		}
	}
}
