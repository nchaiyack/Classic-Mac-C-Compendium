#include "Get_volume_icon.h"
#include <exceptions.h>
#include "Get_resource_icons.h"
#include "Copy_each_icon.h"

/*	------------------------------------------------------------------
	Get_volume_icon				Create an icon suite for a volume,
								assuming that it does not have a custom
								icon.
	------------------------------------------------------------------
*/
OSErr	Get_volume_icon(
/* --> */	short	vRefNum,
/* --> */	IconSelectorValue	icon_selector,
/* <-- */	Handle	*the_suite
)
{
	OSErr	err;
	HParamBlockRec	pb;
	ParamBlockRec	cpb;
	Handle			iconHandle;
	short			save_resfile;
	Ptr				*drive_icon;
	
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVRefNum = vRefNum;
	pb.volumeParam.ioVolIndex = 0;
	
	err = PBHGetVInfoSync(&pb);
	forbid( err, PBHGetVInfoSync );
	
	// set up for Control call
	cpb.cntrlParam.ioCRefNum = pb.volumeParam.ioVDRefNum;
	cpb.cntrlParam.ioVRefNum = pb.volumeParam.ioVDrvInfo;
	drive_icon = (Ptr *) &cpb.cntrlParam.csParam;
	*drive_icon = NULL;
	
	// Warning: Under A/UX 3.0, the csCode = 22 call can fail
	// without returning an error code.
	// try media icon
	cpb.cntrlParam.csCode = 22;
	err = PBControlSync( &cpb );

	if ( (err != noErr) || ((long)*drive_icon <= 0) )
	{
		// try physical drive icon;
		cpb.cntrlParam.csCode = 21;
		err = PBControlSync( &cpb );
	}
	
	if ( (err == noErr) && (0 < (long)*drive_icon) )	// we got an icon
	{
		err = NewIconSuite( the_suite );	// create the new suite
		forbid( err, NewIconSuite );
		
		err = PtrToHand( *drive_icon, &iconHandle, kLargeIconSize );
		forbid( err, PtrToHand );

		err = AddIconToSuite(iconHandle, *the_suite, large1BitMask);
	}
	else	// maybe it's a Mac Plus
	{
		save_resfile = CurResFile();
		UseResFile(0);	// we want a System resource
		err = Get_resource_icons( the_suite, floppyIconResource, icon_selector );
		UseResFile( save_resfile );
	}

PtrToHand:
NewIconSuite:
PBHGetVInfoSync:
	return err;
}
