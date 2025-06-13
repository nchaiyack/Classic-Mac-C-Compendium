/*
 *	XCmdStuf.h
 *
 *	By Theron Trowbridge
 *	© 1994, all rights reserved
 *
 *	A library of utility routines for use in XCMDs and XFCNs.
 *	Some of these were adapted from the book "XCMDs for Hypercard" by Gary Bond.
 *	The strlen() function came from the K&R C book.
 *
 *	Permission for use in non-commercial projects granted.
 *
 */


/********	Standard XCMD Routines	********/
 
void			MoveLockParams(XCmdPtr paramPtr, short paramCount);
void			UnlockParams(XCmdPtr paramPtr, short paramCount);
void			ReturnMessage( XCmdPtr paramPtr, Str255 message );
void			ReturnError( XCmdPtr paramPtr, OSErr fileError );
short			GetParamShort( XCmdPtr paramPtr, short paramNum );
unsigned short	GetParamUShort( XCmdPtr paramPtr, short paramNum );
long			GetParamLong( XCmdPtr paramPtr, short paramNum );
unsigned long	GetParamULong( XCmdPtr paramPtr, short paramNum );
short			strlen(char *str);



/*
 *	Move into high memory the pointers in the parameter block
 *	passed by Hypercard and lock them there so we can refer
 *	to them safely.
 */
 
void	MoveLockParams( XCmdPtr paramPtr, short paramCount )
{
	short i;
	
	for (i=0; i <= paramCount - 1; i++)
	{
		MoveHHi(paramPtr->params[i]);
		HLock(paramPtr->params[i]);	
	}
}



/*
 *	Unlock the pointers in the parameter block so that we
 *	don't muck up memory.
 */
 
void	UnlockParams(XCmdPtr paramPtr, short paramCount)
{
	short i;
	
	for(i = 0; i <= paramCount-1; i++)	{
		HUnlock(paramPtr->params[i]);	
	}
}



/*
 *	Set the return value in the parameter block with a string.
 */
 
void	ReturnMessage( XCmdPtr paramPtr, Str255 message )
{
	paramPtr->returnValue = PasToZero( paramPtr, message );
	UnlockParams( paramPtr, paramPtr->paramCount );
}



/*
 *	Interpret file manager OSErr value and set the return value
 *	with a slightly more human-perceivable version of it.
 */
 
void	ReturnError( XCmdPtr paramPtr, OSErr fileError )
{
	switch ( fileError )
	{
		case eofErr:
			ReturnMessage( paramPtr, "\pError: End of file" );
			break;
		case extFSErr:
			ReturnMessage( paramPtr, "\pError: External file system" );
			break;
		case dskFulErr:
			ReturnMessage( paramPtr, "\pError: Disk full" );
			break;
		case dupFNErr:
			ReturnMessage( paramPtr, "\pError: File found instead of folder" );
			break;
		case fLckdErr:
			ReturnMessage( paramPtr, "\pError: File locked" );
			break;
		case fnfErr:
			ReturnMessage( paramPtr, "\pError: File/Folder not found" );
			break;
		case fnOpnErr:
			ReturnMessage( paramPtr, "\pError: File not open" );
			break;
		case ioErr:
			ReturnMessage( paramPtr, "\pError: I/O error" );
			break;
		case paramErr:
			ReturnMessage( paramPtr, "\pError: Negative count" );
			break;
		case posErr:
			ReturnMessage( paramPtr, "\pError: Attempt to position position before start of file" );
			break;
		case rfNumErr:
			ReturnMessage( paramPtr, "\pError: Bad reference number" );
			break;
		case vLckdErr:
			ReturnMessage( paramPtr, "\pError: Software volume lock" );
			break;
		case wPrErr:
			ReturnMessage( paramPtr, "\pError: Hardware volume lock" );
			break;
		case wrPermErr:
			ReturnMessage( paramPtr, "\pError: Read/write permission doesn't allow writing" );
			break;
		default:
			ReturnMessage( paramPtr, "\pError: Unknown error" );
			break;
	}
}



/*
 *	These routines retrieve a given parameter as a short, long, signed, or unsigned integer.
 *
 */
 
short	GetParamShort( XCmdPtr paramPtr, short paramNum )
{
	short	i, paramLength;
	Str255	paramString, pasParamString;
	
	paramLength = strlen( *paramPtr->params[paramNum] );
	for ( i = 0; i <= paramLength; i++ );
		paramString[i] = (*paramPtr->params[paramNum])[i];
	ZeroToPas( paramPtr, (char *) paramString, pasParamString );
	return( StrToNum( paramPtr, pasParamString ) );
}


unsigned short	GetParamUShort( XCmdPtr paramPtr, short paramNum )
{
	short	i, paramLength;
	Str255	paramString, pasParamString;
	
	paramLength = strlen( *paramPtr->params[paramNum] );
	for ( i = 0; i <= paramLength; i++ );
		paramString[i] = (*paramPtr->params[paramNum])[i];
	ZeroToPas( paramPtr, (char *) paramString, pasParamString );
	return( StrToLong( paramPtr, pasParamString ) );
}


long	GetParamLong( XCmdPtr paramPtr, short paramNum )
{
	short	i, paramLength;
	Str255	paramString, pasParamString;
	
	paramLength = strlen( *paramPtr->params[paramNum] );
	for ( i = 0; i <= paramLength; i++ );
		paramString[i] = (*paramPtr->params[paramNum])[i];
	ZeroToPas( paramPtr, (char *) paramString, pasParamString );
	return( StrToNum( paramPtr, pasParamString ) );
}


unsigned long	GetParamULong( XCmdPtr paramPtr, short paramNum )
{
	short	i, paramLength;
	Str255	paramString, pasParamString;
	
	paramLength = strlen( *paramPtr->params[paramNum] );
	for ( i = 0; i <= paramLength; i++ )
		paramString[i] = (*paramPtr->params[paramNum])[i];
	ZeroToPas( paramPtr, (char *) paramString, pasParamString );
	return( StrToLong( paramPtr, paramString ) );
}



/*
 *	My quickie version of the standard C lib strlen -
 *	so I don't have to include stdio.h.
 */

short	strlen(char *str)
{
	char *ptr = str;
	
	while (*ptr != '\0')
		ptr++;
	return ptr - str;
}