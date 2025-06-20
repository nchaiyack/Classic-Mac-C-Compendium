#include "kant parser.h"
#include "text layer.h"
#include "memory layer.h"
#include "program globals.h"
#include "cancel.h"
#include "environment.h"
#include "window layer.h"
#include "cursor layer.h"

static	void SafeDisposeHandle(Handle theHandle);
static	unsigned char GetTheCharacter(short *currentPosition);
static	void RememberCharacter(unsigned char theChar);
static	ParserError GetReferenceName(Str255 theReferenceName, short *endPosition);
static	ParserError GetReferenceID(Str255 theReferenceName, short *refNum, short *numOptions);
static	ParserError InstantiateReference(Str255 theText, short refNum, short numOptions);
static	ParserError ReplaceTheText(short start, short end, Str255 theStr);
static	ParserError GetChoiceText(Str255 theText, short start, short *end, Boolean chooseFirst);
static	ParserError GetOptionalText(Str255 theText, short start, short *end, Boolean chooseFirst);
static	ParserError GetProbableText(Str255 theText, short start, short *end);
static	double Str255ToReal(Str255 theStr);

static	TEHandle		gTheTEHandle;
static	WindowRef		gTheWindow;
static	short			gTheWindowIndex;
static	short			gTheTEPosition;
static	short			gTheEndPosition;
static	short			gTheTELength;
static	unsigned char	gRememberedChar;
static	Boolean			gRememberedCharValid;

void InitTheParser(WindowRef theWindow, short rawStart, short rawEnd)
{
	gTheTEHandle=GetWindowTE(theWindow);
	gTheWindow=theWindow;
	gTheWindowIndex=GetWindowIndex(theWindow);
	gTheTEPosition=rawStart;
	gTheEndPosition=rawEnd;
	gTheTELength=(**gTheTEHandle).teLength;
	gRememberedChar=0x00;
	gRememberedCharValid=FALSE;
}

ParserError ParseLoop(void)
{
	unsigned char	theChar;
	short			currentPosition, endPosition;
	Str255			theReferenceName;
	ParserError	theError;
	Str255			theText;
	short			numOptions;
	short			refNum;
	
	theChar=' ';
	while (theChar!=0xFF)	/* end of text */
	{
		theChar=GetTheCharacter(&currentPosition);
		switch (theChar)
		{
			case '&':	/* reference */
				theError=GetReferenceName(theReferenceName, &endPosition);
				if (theError!=kNoError)
					return theError;
				else
				{
					theError=GetReferenceID(theReferenceName, &refNum, &numOptions);
					if (theError!=kNoError)
						return theError;
					if (numOptions==0)
						return kNoInstantiations;
					theError=InstantiateReference(theText, refNum, numOptions);
					if (theError!=kNoError)
						return theError;
					theError=ReplaceTheText(currentPosition, endPosition, theText);
					if (theError!=kNoError)
						return theError;
				}
				break;
			case '[':	/* choice */
				theError=GetChoiceText(theText, currentPosition, &endPosition,
					(TickCount()%2) ? TRUE : FALSE);
					if (theError!=kNoError)
					return theError;
				theError=ReplaceTheText(currentPosition, endPosition, theText);
				if (theError!=kNoError)
					return theError;
				break;
			case '<':	/* optional text */
				theError=GetOptionalText(theText, currentPosition, &endPosition,
					(TickCount()%2) ? TRUE : FALSE);
				if (theError!=kNoError)
					return theError;
				theError=ReplaceTheText(currentPosition, endPosition, theText);
				if (theError!=kNoError)
					return theError;
				break;
			case '{':	/* probable text */
				theError=GetProbableText(theText, currentPosition, &endPosition);
				if (theError!=kNoError)
					return theError;
				theError=ReplaceTheText(currentPosition, endPosition, theText);
				if (theError!=kNoError)
					return theError;
				break;
			case '#':	/* next letter capitalized */
				theChar=GetTheCharacter(&endPosition);
				if (((theChar>='a') && (theChar<='z')) ||
					((theChar>='A') && (theChar<='Z')))
				{
					theText[0]=0x01;
					theText[1]=theChar&0xdf;
					theError=ReplaceTheText(currentPosition, endPosition+1, theText);
					if (theError!=kNoError)
						return theError;
				}
				else RememberCharacter(theChar);
				break;
			case '\\':	/* backslash character */
				theChar=GetTheCharacter(&endPosition);
				switch (theChar)
				{
					case 'r':
					case 'R':
						theText[0]=0x01;
						theText[1]='\r';
						break;
					case 't':
					case 'T':
						theText[0]=0x01;
						theText[1]='\t';
						break;
					case '0':
						theText[0]=0x00;
						break;
					default:
						return kBadBackslash;
						break;
				}
				theError=ReplaceTheText(currentPosition, endPosition+1, theText);
				if (theError!=kNoError)
					return theError;
				break;
		}
	}
	
	return kNoError;
}

/* --------------------------------------------------------- */
/* the rest of these functions are internal to kant parser.c */
/* --------------------------------------------------------- */

static	void SafeDisposeHandle(Handle theHandle)
{
	if (theHandle!=0L)
		DisposeHandle(theHandle);
}

static	unsigned char GetTheCharacter(short *currentPosition)
{
	if (gRememberedCharValid)
	{
		gRememberedCharValid=FALSE;
		*currentPosition=gTheTEPosition-1;
		
		return gRememberedChar;
	}
	else
	{
		*currentPosition=gTheTEPosition;
		
		if (gTheTEPosition<gTheTELength)
		{
			return (*(**gTheTEHandle).hText)[gTheTEPosition++];
		}
		else
		{
			return 0xFF;
		}
	}
}

static	void RememberCharacter(unsigned char theChar)
{
	gRememberedChar=theChar;
	gRememberedCharValid=TRUE;
}

static	ParserError GetReferenceName(Str255 theReferenceName, short *endPosition)
{
	unsigned char	theChar;
	short			dummyPosition;
	
	theReferenceName[0]=0x00;
	theChar=GetTheCharacter(&dummyPosition);
	while ((theChar!=0xFF) &&
			(((theChar>='a') && (theChar<='z')) ||
			((theChar>='A') && (theChar<='Z')) ||
			((theChar>='0') && (theChar<='9')) ||
			(theChar=='-') || (theChar=='_')))
	{
		theReferenceName[++theReferenceName[0]]=theChar;
		theChar=GetTheCharacter(&dummyPosition);
	}
	
	RememberCharacter(theChar);
	
	if (theReferenceName[0]==0x00)
		return kNoReferenceName;
	
	*endPosition=dummyPosition;
	return kNoError;
}

static	ParserError GetReferenceID(Str255 theReferenceName, short *refNum, short *numOptions)
{
	Handle			theResHandle;
	Str255			name;
	ResType			theType;
	
	SetResLoad(TRUE);
	theResHandle=Get1NamedResource('STR#', theReferenceName);
	if (theResHandle==0L)
		return kBadReference;
	
	*numOptions=**((short**)theResHandle);
	GetResInfo(theResHandle, refNum, &theType, name);
	ReleaseResource(theResHandle);
	
	return kNoError;
}

static	ParserError InstantiateReference(Str255 theText, short refNum, short numOptions)
{
	GetIndString(theText, refNum, 1+((Random()&0x7fff)%numOptions));
	return kNoError;
}

static	ParserError ReplaceTheText(short start, short end, Str255 theStr)
{
	long			textLength;
	unsigned char	dummy;
	short			dummyPos;
	long			dummyLong;
	
	TESetSelect(start, end, gTheTEHandle);
	TEDelete(gTheTEHandle);
	textLength=theStr[0];
	if (textLength>0)
		TEInsert(&theStr[1], textLength, gTheTEHandle);
	SetWindowIsModified(gTheWindow, TRUE);
	if (!gFastResolve)
	{
		TESelView(gTheTEHandle);
		AdjustVScrollBar(GetWindowVScrollBar(gTheWindow), gTheTEHandle);
	}
	gRememberedCharValid=FALSE;
	gTheTELength=(**gTheTEHandle).teLength;
	gTheTEPosition=start-1;
	if (gTheTEPosition<0)
		gTheTEPosition=0;
	else
	{
		dummy=GetTheCharacter(&dummyPos);
		if (dummy=='#')
			RememberCharacter(dummy);
	}
	
	if (!DealWithOtherPeople())
		return kUserCancelled;
	if (!gInProgress)
		return kUserCancelled;
	if (!IndWindowExistsQQ(gTheWindowIndex))
		return kUserCancelled;
	
	if (!gFastResolve)
		Delay(gSpeedDelay, &dummyLong);
	else
		AnimateTheCursor();
	
	return kNoError;
}

static	ParserError GetChoiceText(Str255 theText, short start, short *end, Boolean chooseFirst)
{
	#pragma unused(start)

	short			level;
	unsigned char	theChar;
	short			endOfFirstChoice;
	short			thePosition;
	
	level=1;
	theText[0]=0x00;
	theChar=GetTheCharacter(&thePosition);
	endOfFirstChoice=-1;
	while (theChar!=0xFF)
	{
		if ((chooseFirst && (endOfFirstChoice==-1)) ||
			(!chooseFirst && (endOfFirstChoice!=-1)))
		{
			theText[++theText[0]]=theChar;
		}
		
		switch (theChar)
		{
			case '[':
				level++;
				break;
			case ']':
				level--;
				if (level==0)
				{
					if (endOfFirstChoice==-1)
						return kBadChoiceFormat;
					*end=thePosition+1;
					theText[0]--;
					return kNoError;
				}
				break;
			case '|':
				if (level==1)
				{
					if (endOfFirstChoice==-1)
						endOfFirstChoice=thePosition;
					else
						return kBadChoiceFormat;
				}
				break;
		}
		
		theChar=GetTheCharacter(&thePosition);
	}
	
	return kBadChoiceFormat;
}

static	ParserError GetOptionalText(Str255 theText, short start, short *end, Boolean useIt)
{
	#pragma unused(start)
	
	short			level;
	unsigned char	theChar;
	short			thePosition;
	
	level=1;
	theText[0]=0x00;
	theChar=GetTheCharacter(&thePosition);
	while (theChar!=0xFF)
	{
		if (useIt)
		{
			theText[++theText[0]]=theChar;
		}
		
		switch (theChar)
		{
			case '<':
				level++;
				break;
			case '>':
				level--;
				if (level==0)
				{
					*end=thePosition+1;
					if (useIt)
						theText[0]--;
					return kNoError;
				}
				break;
		}
		
		theChar=GetTheCharacter(&thePosition);
	}
	
	return kBadOptionalFormat;
}

static	ParserError GetProbableText(Str255 theText, short start, short *end)
{
	#pragma unused(start)
	
	short			level;
	unsigned char	theChar;
	short			thePosition;
	Str255			probabilityStr;
	double			probability;
	Boolean			done, useIt;
	
	theChar=GetTheCharacter(&thePosition);
	done=FALSE;
	probabilityStr[0]=0x00;
	while ((theChar!=0xFF) && (!done))
	{
		if (theChar=='|')
			done=TRUE;
		else if (((theChar>='0') && (theChar<='9')) || (theChar=='.'))
		{
			probabilityStr[++probabilityStr[0]]=theChar;
		}
		else return kBadProbabilityFormat;
		theChar=GetTheCharacter(&thePosition);
	}
	
	probability=Str255ToReal(probabilityStr);
	probability*=32768;
	useIt=((Random()&0x7fff)<probability);
	level=1;
	theText[0]=0x00;
	while (theChar!=0xFF)
	{
		if (useIt)
		{
			theText[++theText[0]]=theChar;
		}
		
		switch (theChar)
		{
			case '{':
				level++;
				break;
			case '}':
				level--;
				if (level==0)
				{
					*end=thePosition+1;
					if (useIt)
						theText[0]--;
					return kNoError;
				}
				break;
		}
		
		theChar=GetTheCharacter(&thePosition);
	}
	
	return kBadProbabilityFormat;
}

static	double Str255ToReal(Str255 theStr)
{
	double			result;
	unsigned short	pos;
	unsigned char	*charPtr;
	Boolean			isNegative;
	unsigned short	i;
	double			powerOfTen;
	double			thisDigit;
	unsigned short	theLength;
	
	theLength=theStr[0];
	charPtr=&theStr[1];
	pos=0;
	result=0.0;
	isNegative=FALSE;
	if (charPtr[0]=='+')
		pos++;
	else if (charPtr[0]=='-')
	{
		isNegative=TRUE;
		pos++;
	}
	
	powerOfTen=0.1;
	for (i=pos; (i<theLength); i++)
	{
		if (charPtr[i]=='.')
			i=theLength;
		else
			powerOfTen*=10;
	}
	
	for (; pos<theLength; pos++)
	{
		if ((charPtr[pos]>='0') && (charPtr[pos]<='9'))
		{
			thisDigit=charPtr[pos]-'0';
			result+=thisDigit*powerOfTen;
			powerOfTen/=10;
		}
	}
	
	return (isNegative ? -result : result);
}
