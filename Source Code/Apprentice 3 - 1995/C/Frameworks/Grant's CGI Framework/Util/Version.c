/*****
 *
 *	Version.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "Version.h"


/***  FUNCTIONS  ***/

/* set the versionString to the short version string from the 'vers' resource with ID resID */
void
VersionGetShort ( short resId, Str255 versionString )
{
	Handle	resource;

	/* Get the resource */
	resource = GetResource ( 'vers', resId );

	if ( resource == nil )
	{
		versionString[0] = nil;
		return;
	}
	
	/*	The short description (pascal) string starts at the 7th byte */
	BlockMove ( (StringPtr) (*resource) + 6, versionString, (*resource)[6] + 1 );

	ReleaseResource ( resource );
} /* VersionGetShort */


/*****  EOF  *****/
