#include <Finder.h>
#include "Get_custom_file_icon.h"
#include <exceptions.h>
#include "Copy_each_icon.h"
#include "Get_resource_icons.h"
#include "Is_suite_empty.h"

/*	------------------------------------------------------------------
	Get_custom_file_icon		Fill a suite with the custom icon of a
								specified file.
	------------------------------------------------------------------
*/
/*
	Custom icons numbered -16496 appear in aliases to volumes and
	servers.  I don't know where this is documented.
*/
#define	kVolumeAliasIconResource	-16496

OSErr	Get_custom_file_icon(
/* --> */	FSSpec	*filespec,
/* --> */	IconSelectorValue	icon_selector,
/* <-- */	Handle	*the_suite
)
{
	short	save_resfile, custom_resfile;
	OSErr	err;
	
	save_resfile = CurResFile();
	SetResLoad( false );
	custom_resfile = FSpOpenResFile( filespec, fsRdPerm );
	SetResLoad( true );
	forbid_action( custom_resfile == -1, FSpOpenResFile,
		err = ResError() );
	
	err = Get_resource_icons( the_suite, kCustomIconResource, icon_selector );
	forbid( err, GetIconSuite );
	
	if (Is_suite_empty( *the_suite ))
	{
		err = Get_resource_icons( the_suite, kVolumeAliasIconResource,
			icon_selector );
	}
	
GetIconSuite:
	CloseResFile( custom_resfile );
	UseResFile( save_resfile );
FSpOpenResFile:
	return err;
}
