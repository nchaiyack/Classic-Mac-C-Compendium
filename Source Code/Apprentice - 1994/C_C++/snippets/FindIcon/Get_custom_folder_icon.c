#include "Get_custom_folder_icon.h"
#include "Get_custom_file_icon.h"

/*	------------------------------------------------------------------
	Get_custom_folder_icon		Given a directory that is known to have
								a custom icon, create an icon suite
								for it.
	
	The icons are stored in an invisible file whose name, on a U.S.
	system, is "\pIcon\r".  Unfortunately the name can vary in other
	countries, and is stored in the Finder.
	------------------------------------------------------------------
*/
OSErr	Get_custom_folder_icon(
/* --> */	short				vRefNum,
/* --> */	long				dirID,
/* --> */	IconSelectorValue	icon_selector,
/* --> */	ConstStr31Param		icon_file_name,
/* <-- */	Handle				*the_suite
)
{
	FSSpec	icon_file;
	
	icon_file.vRefNum = vRefNum;
	icon_file.parID = dirID;
	BlockMove( icon_file_name, icon_file.name, icon_file_name[0]+1 );
	
	return Get_custom_file_icon( &icon_file, icon_selector, the_suite );
}
