/*
 *	CapsKey.c
 *
 *	By Theron Trowbridge
 *	� 1995, all rights reserved
 *
 *	A XFCN for HyperCard to determine if the Caps Lock key is down or not.
 *	Takes no parameters.  Returns "down" if the Caps Lock key is down, "up" if it is not.
 *
 *	If there is one or more parameters, and the first one is a "?", syntax info is returned.
 *	If there is one or more parameters, and the first one is a "!", copyright info is returned.
 *
 */
 
#include "HyperXCmd.h"
#include "XCmdStuff.h"

Boolean	CapsLockIsDown( void );

 pascal void main( XCmdPtr paramPtr )
{
	char		*paramString;
	short		numParams, paramLength;
	char		paramChar;
	
	/********	Do the standard XCMD stuff	********/

	numParams = paramPtr->paramCount;

	/*	Lock the parameter block down so we can safely refer to it:	*/
	MoveLockParams( paramPtr, numParams );

	/*	If any parameters WERE passed, go check to see if the first one is a ! or ?	*/
	if ( numParams > 0 )
	{
		paramString = (char *) *paramPtr->params[0];
		paramLength = strlen( paramString );
		if ( paramLength == 1 )
		{
			paramChar = *paramString;
			switch ( paramChar )
			{
				case '!':
					ReturnMessage( paramPtr, "\pCapsKey XFCN �1995 Theron Trowbridge" );
					return;
					break;
				case '?':
				default:
					ReturnMessage( paramPtr, "\pSyntax is: \"CapsKey()\"" );
					return;
					break;
			}
		}
	}
	
	if ( CapsLockIsDown() )
		ReturnMessage( paramPtr, "\pdown" );
	else
		ReturnMessage( paramPtr, "\pup" );

	return;
}

Boolean	CapsLockIsDown( void )
{
	KeyMap		testKeyMap;
	
	GetKeys( testKeyMap );
	
	if ( BitTst( &testKeyMap, 62 ) )
		return( true );
	else
		return( false );
}