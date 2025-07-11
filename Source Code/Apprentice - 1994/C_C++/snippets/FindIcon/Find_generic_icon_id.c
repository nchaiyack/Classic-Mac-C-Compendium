#include <Finder.h>
#include "Find_generic_icon_id.h"

/*	------------------------------------------------------------------
	Find_generic_icon_id		Given a type, return
								the resource ID of appropriate generic
								icons, and a Boolean telling whether
	the icons live in the Finder, as opposed to the System.
	------------------------------------------------------------------
*/

short	Find_generic_icon_id(
/* --> */	OSType the_type,
/* <-- */	Boolean	*in_Finder )
{
	short	id;
	register	OSType *icon_types;
	register	short *icon_ids;
	Ptr			icon_ids_start;
	OSType		*begin_sys_icons;
	
	asm {
		BRA		@start
	@icon_type_storage
		dc.L	'ifil'
		dc.L	'sfil'
		dc.L	'ffil'
		dc.L	'tfil'
		dc.L	'kfil'
		dc.L	'FFIL'
		dc.L	'DFIL'
	@Finder_icons_end
		dc.L	kContainerFolderAliasType
		dc.L	kContainerTrashAliasType
		dc.L	kSystemFolderAliasType
		dc.L	'INIT'
		dc.L	'APPL'
		dc.L	'dfil'
		dc.L	'pref'
		dc.L	kAppleMenuFolderAliasType
		dc.L	kControlPanelFolderAliasType
		dc.L	kExtensionFolderAliasType
		dc.L	kPreferencesFolderAliasType
		dc.L	kStartupFolderAliasType
		dc.L	kApplicationAliasType
		dc.L	kExportedFolderAliasType
		dc.L	kDropFolderAliasType
		dc.L	kSharedFolderAliasType
		dc.L	kMountedFolderAliasType
	@icon_id_storage
		dc.W	12500
		dc.W	14000
		dc.W	14500
		dc.W	14501
		dc.W	14750
		dc.W	15500
		dc.W	15750

		dc.W	genericFolderIconResource
		dc.W	trashIconResource
		dc.W	systemFolderIconResource
		dc.W	genericExtensionIconResource
		dc.W	genericApplicationIconResource
		dc.W	genericDeskAccessoryIconResource
		dc.W	genericPreferencesIconResource
		dc.W	appleMenuFolderIconResource
		dc.W	controlPanelFolderIconResource
		dc.W	extensionsFolderIconResource
		dc.W	preferencesFolderIconResource
		dc.W	startupFolderIconResource
		dc.W	genericApplicationIconResource
		dc.W	ownedFolderIconResource
		dc.W	dropFolderIconResource
		dc.W	sharedFolderIconResource
		dc.W	mountedFolderIconResource
	@start
		LEA		@icon_type_storage, icon_types
		LEA		@icon_id_storage, icon_ids
		LEA		@Finder_icons_end, A0
		move.L	A0, begin_sys_icons
	}
	
	icon_ids_start = (Ptr) icon_ids;
	id = genericDocumentIconResource;	// default
	
	while (((Ptr) icon_types) < icon_ids_start)
	{
		if (the_type == *icon_types)
		{
			id = *icon_ids;
			break;
		}
		icon_types++;
		icon_ids++;
	}

	*in_Finder = (icon_types < begin_sys_icons);

	return id;
}
