// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright © 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include "ConnToolCommon.h"

// ===========================================================================
// Function prototypes
// ===========================================================================
extern pascal long	main(ConnHandle hConn, short msg, Ptr pConfig, long unused, long unused2);
extern Ptr		GetConfig(ConnHandle hConn);
extern short		SetConfig(ConnHandle hConn, Ptr theSource);
extern void		CStrCopy(char *sourceStr, char *targetStr);
extern void		CStrAppend(char *targetStr, char *sourceStr);
extern void		P2CStrConvert(char *sourceStr, char *targetStr);
extern void		C2PStrConvert(char *sourceStr, char *targetStr);
extern short		CStrLen(char *sourceStr);
extern void		GetToken(char **sourceStr, char *targetStr, char delim);
extern Boolean	CStrEqual(char *sourceStr, char *targetStr);

// ===========================================================================
// main()
// 	This function is the entry point for the 'cscr' resource.  It passes control to the appropriate
// 	subroutines, depending on the incoming message.  This can probably remain unchanged.
// ===========================================================================
pascal long main(ConnHandle hConn, short msg, Ptr pConfig, long unused, long unused2)
{
long			rtnValue;

	switch (msg)
		{
			case cmMgetMsg:										
				rtnValue = (long)GetConfig(hConn);
				break;
			
			case cmMsetMsg:										
				rtnValue = SetConfig(hConn, pConfig);
				break;

			default:														// Say WHAT?
				rtnValue = cmNotSupported;
				break;
		}
}

// ===========================================================================
// GetConfig()
// 	This function is called in response to a cmMgetMsg.  It should return a string that describes the
// 	connection record.
// ===========================================================================
Ptr	GetConfig(ConnHandle hConn)
{
Ptr			thePtr;
ConfigPtr	pConfig;
Str255		theString, string2;
short		len;

	pConfig = (ConfigPtr)((*hConn)->config);					// Get the config record
	CStrCopy("FOOBAR", (char *)theString);					// The attribute's name is FOOBAR
	NumToString(pConfig->foobar, string2);					// Get the attribute value
	P2CStrConvert((char *)string2, (char *)string2);	// Convert to C string
	CStrAppend((char *)theString, (char *)string2);		// Concatenate string2 to theString
	len = CStrLen((char *)theString) + 1;						// Get its length (incl. trailing null)
	thePtr = NewPtr(len);												// Allocate space for it
	
	if (thePtr)
		{
			BlockMove(theString, thePtr, len);
		}
	
	return (thePtr);
}


// ===========================================================================
// SetConfig()
// 	This function is called in response to a cmMsetMsg.  It should set the specific configuration
//	fields requested.  It should return 0 for no error, -1 for a generic error, a number < -1 for an
//	OSErr, and a positive number for the character position where the parsing stopped.
// ===========================================================================
short	SetConfig(ConnHandle hConn, Ptr theSource)
{
ConfigPtr	pConfig;
Str255		paramStr, valueStr;
Boolean		outOfTokens;
short		rtnValue;
long			localLong;
char			*tokenStart, *sourceStart;

	pConfig = (ConfigPtr)((*hConn)->config);
	rtnValue = noErr;
	sourceStart = theSource;
	
	if (*theSource == 0)
		{
			outOfTokens = TRUE;
		}
	else
		{
			outOfTokens = FALSE;
		}

//	This currently only parses strings using space delimiters.  It is not WorldScript-savvyª.  If
//	you want to add that support (using IntlTokenize), you'll sacrifice System 6.0.x support.  I
//	really don't see any point in doing that until OTI supercedes CTB.  (OpenTransport Interface, and
//	Communications ToolBox, respectively)

	while (!outOfTokens)
		{
			tokenStart = theSource;
			GetToken(&theSource, (char *)paramStr, ' ');
			if (CStrEqual((char *)paramStr, "FOOBAR"))
				{
					GetToken(&theSource, (char *)paramStr, ' ');
					C2PStrConvert((char *)paramStr, (char *)valueStr);
					StringToNum(valueStr, &localLong);
					pConfig->foobar = localLong;
				}
			else
				{
					rtnValue = (tokenStart - sourceStart) + 1;
					outOfTokens = TRUE;
				}
		}

	return (rtnValue);
}

// ===========================================================================
// These may not be pretty, but they were designed to compile as efficiently as possible under Think C
// 6.  (Try disassembling them...I don't think you'll get them much tighter)
//
// Excerpted from
//
// "StrUtils.c" hand-coded 680x0 assembly library
// Translated to C to compile to code identical to the hand-coded asm
// Copyright © 1992-1994 Peter J. Creath
// All Rights Reserved Worldwide
// ===========================================================================

void	CStrCopy(char *sourceStr, char *targetStr)
{
register char	*pSource;
register char	*pTarget;

	pSource	= sourceStr;
	pTarget		= targetStr;

	while (*(pTarget++) = *(pSource++));
}

void CStrAppend(char *targetStr, char *sourceStr)
{
register char	*pSource;
register char	*pTarget;

	pSource	= sourceStr;
	pTarget		= targetStr;

	while (*(pTarget++));								// Move to end of C string
	pTarget--;												// Move back to erase trailing null
	while (*(pTarget++) = *(pSource++));		// Copy to this new point
}

void	P2CStrConvert(char *sourceStr, char *targetStr)
{
register char	*pSource;
register char	*pTarget;
register char	len;

	pSource	= sourceStr;
	pTarget		= targetStr;

	if (len = *(pSource)++)								// Get length of Pascal string
		{
			do
				{
					*(pTarget++) = *(pSource++);	// Copy string
				} while (--len);
		}
	*pTarget = 0;											// Add trailing null
}

void	C2PStrConvert(char *sourceStr, char *targetStr)
{
register char	*pSource;
register char	*pTarget;

	pSource	= sourceStr;
	pTarget		= targetStr + 1;

	while (*(pTarget++) = *(pSource++));

	pTarget		= targetStr;
	*pTarget	= (pSource - sourceStr - 1);
}

short	CStrLen(char *sourceStr)
{
register char	*pSource;
register char	len = 0;

	pSource = sourceStr;
	while (*(pSource++)) len++;
	
	return (len);
}

void		GetToken(char **sourceStr, char *targetStr, char delim)
{
register char	*pSource;
register char	*pTarget;
register char	locDelim;

	pSource = *sourceStr;
	pTarget = targetStr;
	locDelim = delim;
	while (*pSource && *pSource != locDelim) *(pTarget++) = *(pSource++);
	*pTarget = 0;
	*sourceStr = pSource + 1;
}

Boolean		CStrEqual(char *sourceStr, char *targetStr)
{
register char		*pSource;
register char		*pTarget;
register Boolean	areEqual;

	pSource = sourceStr;
	pTarget = targetStr;
	areEqual = TRUE;

	do
		{
			if (*pSource != *(pTarget++))
				{
					areEqual = FALSE;
					break;
				}
		} while (*(pSource++));
	
	return (areEqual);
}
