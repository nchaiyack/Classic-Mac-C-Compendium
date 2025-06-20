#include "Find_icon.h"
#include <exceptions.h>
#include <Finder.h>
#include "Get_custom_folder_icon.h"
#include "Get_normal_folder_icon.h"
#include "Get_volume_icon.h"
#include "Get_custom_file_icon.h"
#include "Get_normal_file_icon.h"
#include "Is_vol_ejected.h"

/*	------------------------------------------------------------------
	Find_icon		Given a file specification for a file, folder, or
					volume, create an appropriate icon suite
					and find its label color.
	------------------------------------------------------------------
*/
pascal OSErr Find_icon(
/* --> */	FSSpec	*thing,
/* --> */	IconSelectorValue	icon_selector,
/* --> */	ConstStr31Param		icon_file_name,
/* <-- */	Handle	*the_suite,
/* <-- */	short	*label_color
)
{
	CInfoPBRec		cpb;
	OSErr			err;
	
	*the_suite = NULL;
	*label_color = 0;
	
	forbid_action( Is_vol_ejected( thing->vRefNum ), ejected,
		err = volOffLinErr );
	
	cpb.hFileInfo.ioVRefNum = thing->vRefNum;
	cpb.hFileInfo.ioDirID = thing->parID;
	cpb.hFileInfo.ioNamePtr = thing->name;
	cpb.hFileInfo.ioFDirIndex = 0;
	err = PBGetCatInfoSync( &cpb );
	forbid( err, PBGetCatInfoSync );
	
	/*
		Shifted this way, the color is ready to be used as an
		icon transformation.
	*/
	*label_color = (cpb.hFileInfo.ioFlFndrInfo.fdFlags & kColor) << 7;
	
	if ( (cpb.hFileInfo.ioFlAttrib & ioDirMask) == 0 )	// file
	{
		if (cpb.hFileInfo.ioFlFndrInfo.fdFlags & kHasCustomIcon)
		{
			err = Get_custom_file_icon( thing,
				icon_selector, the_suite );
		}
		else	// no custom icon
		{
			err = Get_normal_file_icon( &cpb,
				icon_selector, the_suite );
		}
	}
	else	// directory
	{
		if (cpb.hFileInfo.ioFlFndrInfo.fdFlags & kHasCustomIcon)
		{
			err = Get_custom_folder_icon( thing->vRefNum,
				cpb.hFileInfo.ioDirID, icon_selector, icon_file_name,
				the_suite );
		}
		else	// no custom icon
		{
			if (cpb.hFileInfo.ioDirID == fsRtDirID)	// a volume
			{
				err = Get_volume_icon( thing->vRefNum,
					icon_selector, the_suite );
			}
			else	// a folder
			{
				err = Get_normal_folder_icon( &cpb,
					icon_selector, the_suite );
			}
		}
	}
	goto getout;
	// ----------- end of normal case --------------

ejected:
PBGetCatInfoSync:	// ------- error handler ---------
	if (thing->parID == fsRtParID)	// a volume
	{
		if (err == volOffLinErr)
		{
			*label_color = ttOffline;
		}
		err = Get_volume_icon( thing->vRefNum,
					icon_selector, the_suite );
	}


getout:
	return err;
}
