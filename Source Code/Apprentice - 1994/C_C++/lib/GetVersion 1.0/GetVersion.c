/******************************************************************************
 GetVersion.c

	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-06-03

******************************************************************************/

#include <TCLHeaders>
#include "GetVersion.h"


#define HiNibble( _byte ) ( (_byte) >> 4 )
#define LoNibble( _byte ) ( (_byte) & 0x0F )



/******************************************************************************
 GetVersion

	Copy the values in the 'vers' resource id to the version structure.
******************************************************************************/

void GetVersion( short id, VERSION* version )
{
	Handle resource;

		/* Get the resource */

	resource = GetResource( 'vers', id );
	FailNILRes( resource );

		/*
			These three values are distributed between two bytes. The first
			byte has the major version number in BCD; it can have a max value
			of 99. The minor and minuscule values are encoded in the second
			byte in BCD; each can have a max value of 9.
		*/

	version->Major = HiNibble( (*resource)[0] ) * 10 + LoNibble( (*resource)[0] );
	version->Minor = HiNibble( (*resource)[1] );
	version->Minuscule = LoNibble( (*resource)[1] );
	
	version->Stage = (*resource)[2];	
	version->Release = HiNibble( (*resource)[3] ) * 10 + LoNibble( (*resource)[3] );
	
	version->Region = *(short*) ((*resource) + 4);

		/*
			The long and short description strings are concatinated in resource.
			Thus the long description's start is dependent upon the short
			description's length.
		*/

	CopyPString( (StringPtr) (*resource) + 6, version->ShortDescription );
	CopyPString( (StringPtr) (*resource) + (7 + version->ShortDescription[0] ), 
		version->LongDescription );

		/* Done with the resource */
	
	ReleaseResource( resource );

} /* GetVersion */