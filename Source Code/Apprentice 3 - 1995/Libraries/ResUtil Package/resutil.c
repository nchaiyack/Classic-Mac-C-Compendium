/* resutils.c  22 Aug 94  Eric Kidd */

# include "resutil.h"

/* resutils.c
** resource manipulation library
**
** Functions for making resource manipulation moderately civilized. Originally designed for
** File Typer and inspired by the new Inside Macintosh volumes.
*/

/* OSErr ClearResID( short rf, OSType rType, short rID )
** 
** Remove any resources with the given type and ID from the file "rf". If somebody has
** written in extra copies of a resource, messing up the structure, this routine should
** remove them all.
*/

OSErr ClearResID( short rf, ResType rType, short rID )
{
	short savedRF;
	OSErr err;
	Handle theRes;
	OSErr returnVal;
	
	savedRF = CurResFile( );
	UseResFile( rf );
	
	/* Read somewhere that I should make sure there is no res before installing one */
	/* If the resource is duplicated, keep on deleting until the ID is clear		*/
	/* Sometimes multiple resources will have been written with the same ID. Yech.	*/
	
	theRes = Get1Resource( rType, rID );
	while ( ResError( ) == noErr && theRes != NULL )
	{
		RmveResource( theRes );
		
		if ( ( returnVal = ResError( ) ) != noErr )
		{
			UseResFile( savedRF );
			return returnVal;
		}
			
		DisposeHandle( theRes );
		UpdateResFile( rf );
		theRes = Get1Resource( rType, rID );
	}
	
	if ( ResError( ) == resNotFound )
		returnVal = noErr;
	else
		returnVal = ResError( );
	
	UseResFile( savedRF );
	return returnVal;
}

/* OSErr InstallResource( short rf, Handle theRes, ResType rType,
**							short rID, StringPtr name, short attr )
** 
** Installs the handle's data as a res in "rf" with the specified information. Releases the resource
** when done, if successful.
*/

OSErr InstallResource( short rf, Handle theRes, ResType rType, short rID, StringPtr name, short attr )
{
	OSErr err;
	short savedRF;
	
	savedRF = CurResFile( );
	UseResFile( rf );
	
	err = ClearResID( rf, rType, rID );
	if ( err == noErr )
	{
		AddResource( theRes, rType, rID, name );
		
		if ( ResError( ) == noErr )
			SetResAttrs( theRes, attr );
		if ( ResError( ) == noErr )
			ChangedResource( theRes );
		if ( ResError( ) == noErr )
			WriteResource( theRes );
		if ( ResError( ) == noErr )
			ReleaseResource( theRes );
			
		err = ResError( );
	}
	
	UseResFile( savedRF );
	return err;
}

/* OSErr CopyResource( ResType rType, short rID, short src, short dest )
**
** Apple Computer's standard CopyResource procedure in C with a few modifications for greater
** reliability and capabilities. Uses InstallResource.
*/

OSErr CopyResource( ResType rType, short rID, short src, short dest )
{
	short savedRF;
	Handle theRes;
	short attr;
	Str255 name;
	
	short scratchID;
	ResType scratchType;
	OSErr returnVal;
		
	savedRF = CurResFile( );
	UseResFile( src );
	
	theRes = Get1Resource( rType, rID );
	if ( theRes == NULL )
	{
		UseResFile( savedRF );
		return ResError( );
	}
	
	GetResInfo( theRes, &scratchID, &scratchType, name );
	attr = GetResAttrs( theRes );
	DetachResource( theRes );
	UseResFile( savedRF );

	return InstallResource( dest, theRes, rType, rID, name, attr );
}
