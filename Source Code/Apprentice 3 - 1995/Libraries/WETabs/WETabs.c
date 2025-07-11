/************************************************************************\
	WETabs.c
	
	Hooks for adding tab support to WASTE
	
	Original code by Mark Alldritt
	Line breaking code by Dan Crevier
	Support for horizontal scrolling by Bert Seltzer
	
\************************************************************************/

#include "WASTEIntf.h"
#include "WETabs.h"

pascal void _WETabDrawText(Ptr pText, long textLength, Fixed slop, 
				JustStyleCode styleRunPosition, WEHandle hWE)
{
	long	ii, beginChar;
	Point	penPos;
	long	destLeft;
	
	destLeft = (*hWE)->destRect.left + 1L;
	
	beginChar = 0;
	for (ii = 0; ii < textLength; ii++)
	{
		if (pText[ii] == '\t')
		{
			DrawJustified(pText + beginChar, ii - beginChar, slop, styleRunPosition,
					  kOneToOneScaling, kOneToOneScaling);
			GetPen(&penPos);
     		MoveTo(destLeft + ((penPos.h - destLeft + WASTE_TAB_SIZE + 1 )/ 
                                WASTE_TAB_SIZE) * WASTE_TAB_SIZE, penPos.v);
			beginChar = ii + 1;
		}
	}
	DrawJustified(pText + beginChar, textLength - beginChar, slop, styleRunPosition,
			  kOneToOneScaling, kOneToOneScaling);

} // { _WETabDrawText }

pascal long _WETabPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE)
{
	long ii, beginChar, offset;
	Fixed lastWidth, tabWidth;
	long retVal;
	long destLeft;
	Boolean theEdge = *edge;

	destLeft = (*hWE)->destRect.left + 1L;
			
	beginChar = 0;
	offset = 0;
	for (ii = 0; ii < textLength && *width > 0; ii++)
	{
		if (pText[ii] == '\t')
		{
			/*	Measure this sub-segment	*/
	
			lastWidth = *width;
			offset += PixelToChar(pText + beginChar, ii - beginChar,
							slop, lastWidth, &theEdge, width,
							styleRunPosition, kOneToOneScaling, kOneToOneScaling);
			if (*width >= 0)
			{
				hPos += lastWidth - *width;
		
				tabWidth = destLeft + ((hPos - destLeft + ((WASTE_TAB_SIZE + 1)<<16))/
							(WASTE_TAB_SIZE<<16)) * (WASTE_TAB_SIZE<<16) - hPos;
				hPos += tabWidth;
				if (*width - tabWidth < 0)
				{
					if (*width - (tabWidth + (1 << 16))/2 > 0)
					{
						*edge = -1;
						offset++;
					}
					else
					{
						*edge = 0;
					}
					*width = -1 << 16;
				}
				else
				{
					offset++;
					*width -= tabWidth;
				}
		
				beginChar = ii + 1;
			}
		}
	}
	if (*width > 0)
	{
		lastWidth = *width;
		offset += PixelToChar(pText + beginChar, textLength - beginChar,
					slop, lastWidth, &theEdge, width, styleRunPosition,
					kOneToOneScaling, kOneToOneScaling);
	}
	
	retVal = offset;

	// { round width to nearest integer value }
	// { (this is supposed to fix an incompatibility with the WorldScript Power Adapter) }
	*width = BSL(FixRound(*width), 16);

	*edge = theEdge;
	
	return retVal;
} // { _WETabPixelToChar }

pascal short _WETabCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE)
{
	long	ii,
			beginChar;
	short	width;
	long	destLeft;

	destLeft = (*hWE)->destRect.left + 1L;
	beginChar = 0;
	width = 0;
	for (ii = 0; ii < textLength && offset > ii; ii++)
	{
		if (pText[ii] == '\t')
		{
			width += CharToPixel(pText + beginChar, ii - beginChar, slop,
							offset - beginChar,
							smHilite, styleRunPosition, kOneToOneScaling,
							kOneToOneScaling);
			beginChar = ii + 1;
     		width += destLeft + ((hPos + width - destLeft + WASTE_TAB_SIZE + 1)/ 
     					WASTE_TAB_SIZE) * WASTE_TAB_SIZE - hPos - width;
		}
	}
	width += CharToPixel(pText + beginChar, textLength - beginChar, slop,
					offset - beginChar, smHilite, styleRunPosition,
					kOneToOneScaling, kOneToOneScaling);
	return width;
} // { _WETabCharToPixel }

pascal StyledLineBreakCode _WETabLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE)
{
	long ii, beginChar;
	short h, tabWidth;
	Fixed tabWidthFixed;
	StyledLineBreakCode isBreak = 0;
	long destLeft;

	destLeft = (*hWE)->destRect.left + 1L;

	beginChar = textStart;

	for (ii = textStart; ii < textEnd; ii++)
	{
		if (pText[ii] == '\t')
		{
			// do previous "segment"
			isBreak = (StyledLineBreak(pText, textLength, beginChar, ii, 0,
						textWidth, textOffset) != smBreakOverflow);
			if (isBreak || (ii >= textLength)) break;

			beginChar = ii+1;
						
			// calculate tab width
			h = (*hWE)->destRect.right - (*hWE)->destRect.left - BSR(*textWidth, 16);
			tabWidth = WASTE_TAB_SIZE - (h % WASTE_TAB_SIZE);
			//tabWidth = (destLeft + ((h - destLeft + WASTE_TAB_SIZE+1)/WASTE_TAB_SIZE) * 
            //             WASTE_TAB_SIZE) - h;
			tabWidthFixed = BSL(tabWidth, 16);
	
			// if tabWidth > pixelWidth we break in tab
			// don't move tab to next line
			if (tabWidthFixed > *textWidth)
			{
				isBreak = true;
				*textOffset = ii + 1;
				break;
			}
			else
			{
				// subtract tab width from pixelWidth
				*textWidth -= tabWidthFixed;
			}
		}
	}
	// do last segment
	if (ii-beginChar >= 0 && !isBreak)
	{
	
		// do the styled break
		isBreak = StyledLineBreak(pText, textLength, beginChar, ii, 0,
			textWidth, textOffset);
	}
	return isBreak;
} // { _WETabLineBreak }

// static UPP's
static WEDrawTextUPP		_weTabDrawTextProc = NULL;
static WEPixelToCharUPP		_weTabPixelToCharProc = NULL;
static WECharToPixelUPP		_weTabCharToPixelProc = NULL;
static WELineBreakUPP		_weTabLineBreakProc = NULL;

pascal void WEInstallTabHooks(WEHandle hWE)
{
	// if first time, make UPP's
	if (_weTabDrawTextProc == NULL)
	{
		_weTabDrawTextProc = NewWEDrawTextProc(_WETabDrawText);
		_weTabPixelToCharProc = NewWEPixelToCharProc(_WETabPixelToChar);
		_weTabCharToPixelProc = NewWECharToPixelProc(_WETabCharToPixel);
		_weTabLineBreakProc = NewWELineBreakProc(_WETabLineBreak);
	}
	
	(*hWE)->drawTextHook = (UniversalProcPtr)_weTabDrawTextProc;
	(*hWE)->pixelToCharHook = (UniversalProcPtr)_weTabPixelToCharProc;
	(*hWE)->charToPixelHook = (UniversalProcPtr)_weTabCharToPixelProc;
	(*hWE)->lineBreakHook = (UniversalProcPtr)_weTabLineBreakProc;
}


