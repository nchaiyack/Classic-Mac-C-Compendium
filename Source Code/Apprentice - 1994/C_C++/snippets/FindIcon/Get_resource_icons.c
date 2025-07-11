#include "Get1IconSuite.h"
#include "Copy_each_icon.h"
#include "Get_resource_icons.h"

/*	---------------------------------------------------------------------
	Get_resource_icons				Get an icon suite from resources in
									the current resource file.
	
	Called by Get_normal_file_icon, Get_custom_file_icon,
	Get_normal_folder_icon, Get_volume_icon.
	---------------------------------------------------------------------
*/
pascal OSErr Get_resource_icons(
/* <-- */	Handle *theSuite,
/* --> */	short theID,
/* --> */	long theSelector
)
{
	OSErr	err;
	
	err = Get1IconSuite( theSuite, theID, theSelector );
	if (err == noErr)
	{
		err = Copy_each_icon( *theSuite );
	}
	
	return err;
}
