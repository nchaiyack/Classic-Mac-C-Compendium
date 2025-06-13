#include "styled text.h"

void DrawTheText(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect)
{
	short			i, numStyles;
	long			textPos;
	long			maxOffset;
	Str255			thisLine;
	Boolean			notDoneYet;
	unsigned char	thisChar;
	short			theRow, theCol;
	unsigned char	lastEnd, thisEnd;
	Boolean			overRun;
	
	numStyles=(**theStyleHandle).numStyles;
	textPos=0L;
	theRow=theRect.top+(**theStyleHandle).theStyle[0].fontDescent+1;
	theCol=theRect.left;
	thisLine[0]=0x00;
	lastEnd=0;
	for (i=0; i<numStyles; i++)
	{
		if (i==numStyles-1)
			maxOffset=GetHandleSize((Handle)theText);
		else
			maxOffset=(**theStyleHandle).theStyle[i+1].offset;
		
		TextFont((**theStyleHandle).theStyle[i].fontNum);
		TextFace((**theStyleHandle).theStyle[i].fontStyle);
		TextSize((**theStyleHandle).theStyle[i].fontSize);
		TextMode(theMode);
		
		while (textPos<maxOffset)
		{
			notDoneYet=TRUE;
			while ((textPos<maxOffset) && (notDoneYet))
			{
				thisChar=thisLine[++thisLine[0]]=(*theText)[textPos++];
				notDoneYet=((thisChar!=' ') && (thisChar!=0x0d));
			}
			
			thisEnd=thisLine[0];
			overRun=(theRect.right-theCol<=StringWidth(thisLine));
			
			if ((overRun) || (thisChar==0x0d) || (textPos==maxOffset))
			{
				if (overRun)
					thisLine[0]=lastEnd;
				if (theJust==kCenter)
					MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
							theCol, theRow);
				else
					MoveTo(theCol, theRow);
				theCol+=StringWidth(thisLine);
				DrawString(thisLine);
				if (overRun)
				{
					BlockMove(&thisLine[lastEnd+1], &thisLine[1], thisEnd-lastEnd+1);
					if (thisEnd>=lastEnd)
					{
						thisLine[0]=thisEnd-lastEnd-1;
						textPos--;
					}
					else
						thisEnd=thisLine[0]=0x00;
				}
				else thisLine[0]=0x00;
				if ((overRun) || (thisChar==0x0d))
				{
					theRow+=(**theStyleHandle).theStyle[i].lineHeight;
					theCol=theRect.left;
				}
			}
			
			lastEnd=thisEnd;
		}
		
		if (thisLine[0]!=0x00)
		{
			if (theJust==kCenter)
				MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
						theCol, theRow);
			else
				MoveTo(theCol, theRow);
			theCol+=StringWidth(thisLine);
			DrawString(thisLine);
			thisLine[0]=0x00;
		}
	}
	TextMode(srcOr);
}

void DrawThePartialText(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect, short startLine, short endLinePlusOne)
{
	short			i, numStyles;
	long			textPos;
	long			maxOffset;
	Str255			thisLine;
	Boolean			notDoneYet;
	unsigned char	thisChar;
	short			theRow, theCol;
	unsigned char	lastEnd, thisEnd;
	Boolean			overRun;
	short			thisLineIndex;
	
	thisLineIndex=0;
	numStyles=(**theStyleHandle).numStyles;
	textPos=0L;
	theRow=theRect.top+(**theStyleHandle).theStyle[0].fontDescent+1;
	theCol=theRect.left;
	thisLine[0]=0x00;
	lastEnd=0;
	for (i=0; i<numStyles; i++)
	{
		if (i==numStyles-1)
			maxOffset=GetHandleSize((Handle)theText);
		else
			maxOffset=(**theStyleHandle).theStyle[i+1].offset;
		
		TextFont((**theStyleHandle).theStyle[i].fontNum);
		TextFace((**theStyleHandle).theStyle[i].fontStyle);
		TextSize((**theStyleHandle).theStyle[i].fontSize);
		TextMode(theMode);
		
		while (textPos<maxOffset)
		{
			notDoneYet=TRUE;
			while ((textPos<maxOffset) && (notDoneYet))
			{
				thisChar=thisLine[++thisLine[0]]=(*theText)[textPos++];
				notDoneYet=((thisChar!=' ') && (thisChar!=0x0d));
			}
			
			thisEnd=thisLine[0];
			overRun=(theRect.right-theCol<=StringWidth(thisLine));
			
			if ((overRun) || (thisChar==0x0d) || (textPos==maxOffset))
			{
				if (overRun)
					thisLine[0]=lastEnd;
				if (theJust==kCenter)
					MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
							theCol, theRow);
				else
					MoveTo(theCol, theRow);
				theCol+=StringWidth(thisLine);
				if ((thisLineIndex>=startLine) && (thisLineIndex<endLinePlusOne))
					DrawString(thisLine);
				if (overRun)
				{
					BlockMove(&thisLine[lastEnd+1], &thisLine[1], thisEnd-lastEnd+1);
					if (thisEnd>=lastEnd)
					{
						thisLine[0]=thisEnd-lastEnd-1;
						textPos--;
					}
					else
						thisEnd=thisLine[0]=0x00;
				}
				else thisLine[0]=0x00;
				if ((overRun) || (thisChar==0x0d))
				{
					if ((thisLineIndex>=startLine) && (thisLineIndex<endLinePlusOne))
						theRow+=(**theStyleHandle).theStyle[i].lineHeight;
					theCol=theRect.left;
					thisLineIndex++;
				}
			}
			
			lastEnd=thisEnd;
		}
		
		if (thisLine[0]!=0x00)
		{
			if (theJust==kCenter)
				MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
						theCol, theRow);
			else
				MoveTo(theCol, theRow);
			theCol+=StringWidth(thisLine);
			if ((thisLineIndex>=startLine) && (thisLineIndex<endLinePlusOne))
				DrawString(thisLine);
			thisLine[0]=0x00;
		}
	}
	TextMode(srcOr);
}

short CountLines(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect)
{
	short			i, numStyles;
	long			textPos;
	long			maxOffset;
	Str255			thisLine;
	Boolean			notDoneYet;
	unsigned char	thisChar;
	short			theRow, theCol;
	unsigned char	lastEnd, thisEnd;
	Boolean			overRun;
	short			numLines;
	
	numLines=0;
	numStyles=(**theStyleHandle).numStyles;
	textPos=0L;
	theRow=theRect.top+(**theStyleHandle).theStyle[0].fontDescent+1;
	theCol=theRect.left;
	thisLine[0]=0x00;
	lastEnd=0;
	for (i=0; i<numStyles; i++)
	{
		if (i==numStyles-1)
			maxOffset=GetHandleSize((Handle)theText);
		else
			maxOffset=(**theStyleHandle).theStyle[i+1].offset;
		
		TextFont((**theStyleHandle).theStyle[i].fontNum);
		TextFace((**theStyleHandle).theStyle[i].fontStyle);
		TextSize((**theStyleHandle).theStyle[i].fontSize);
		TextMode(theMode);
		
		while (textPos<maxOffset)
		{
			notDoneYet=TRUE;
			while ((textPos<maxOffset) && (notDoneYet))
			{
				thisChar=thisLine[++thisLine[0]]=(*theText)[textPos++];
				notDoneYet=((thisChar!=' ') && (thisChar!=0x0d));
			}
			
			thisEnd=thisLine[0];
			overRun=(theRect.right-theCol<=StringWidth(thisLine));
			
			if ((overRun) || (thisChar==0x0d) || (textPos==maxOffset))
			{
				if (overRun)
					thisLine[0]=lastEnd;
//				if (theJust==kCenter)
//					MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
//							theCol, theRow);
//				else
//					MoveTo(theCol, theRow);
				theCol+=StringWidth(thisLine);
//				DrawString(thisLine);
				if (overRun)
				{
					BlockMove(&thisLine[lastEnd+1], &thisLine[1], thisEnd-lastEnd+1);
					if (thisEnd>=lastEnd)
					{
						thisLine[0]=thisEnd-lastEnd-1;
						textPos--;
					}
					else
						thisEnd=thisLine[0]=0x00;
				}
				else thisLine[0]=0x00;
				if ((overRun) || (thisChar==0x0d))
				{
					theRow+=(**theStyleHandle).theStyle[i].lineHeight;
					theCol=theRect.left;
					numLines++;
				}
			}
			
			lastEnd=thisEnd;
		}
		
		if (thisLine[0]!=0x00)
		{
//			if (theJust==kCenter)
//				MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
//						theCol, theRow);
//			else
//				MoveTo(theCol, theRow);
			theCol+=StringWidth(thisLine);
//			DrawString(thisLine);
			thisLine[0]=0x00;
		}
	}
	TextMode(srcOr);
	
	return numLines;
}
