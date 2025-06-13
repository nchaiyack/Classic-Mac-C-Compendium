/*****
 *
 *	MemoryUtil.c
 *
 *****/

#include "MemoryUtil.h"


/* creates a new handle with all bytes set to nil. Returns nil if error occurs.
	If a non-nil value is passed in theErr, it is set to the memory error (usually noErr) */
Handle
MyNewHandleClear ( long theSize, OSErr *theErr )
{
	Handle	theHandle;
	OSErr	myErr;
	
	theHandle	= NewHandleClear ( theSize );
	myErr		= MemError ();
	
	if ( *theErr != nil )
	{
		*theErr = myErr;
	}
	
	if ( myErr != noErr )
	{
		return nil;
	}
	else
	{
		return theHandle;
	}
}


/* creates a new pointer. Returns nil if error occurs.
	If a non-nil value is passed in theErr, it is set to the memory error (usually noErr) */
Ptr
MyNewPtr ( long theSize, OSErr *theErr )
{
	Ptr		thePtr;
	OSErr	myErr;
	
	thePtr		= NewPtr ( theSize );
	myErr		= MemError ();
	
	if ( *theErr != nil )
	{
		*theErr = myErr;
	}
	
	if ( myErr != noErr )
	{
		return nil;
	}
	else
	{
		return thePtr;
	}
}


