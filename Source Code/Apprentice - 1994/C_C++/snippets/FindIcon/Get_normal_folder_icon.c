#include <Finder.h>
#include <Folders.h>
#include "Get_normal_folder_icon.h"
#include <exceptions.h>
#include "Get_resource_icons.h"
#include "Copy_each_icon.h"

static Boolean Match_special_dirID( OSType folder_type, long dirID );

/*	------------------------------------------------------------------
	Get_normal_folder_icon			Create an icon suite for a folder
									that does not have a custom icon.
	------------------------------------------------------------------
*/
OSErr	Get_normal_folder_icon(
/* --> */	CInfoPBRec	*cpb,
/* --> */	IconSelectorValue	icon_selector,
/* <-- */	Handle	*the_suite
)
{
	OSErr	err;
	short	found_vRefNum, icon_id;
	long	found_dirID, dirID;
	short	save_resfile;
	
	dirID = cpb->dirInfo.ioDrDirID;
	
	if ( (FindFolder( kOnSystemDisk, kSystemFolderType, kDontCreateFolder,
		&found_vRefNum, &found_dirID ) == noErr)
		&& (found_vRefNum == cpb->dirInfo.ioVRefNum)
		&& (found_dirID == dirID) )
	{
		icon_id = systemFolderIconResource;
	}
	else if ( (found_vRefNum == cpb->dirInfo.ioVRefNum) &&
		(found_dirID == cpb->dirInfo.ioDrParID) )
	{
		/*
			The folder is a subfolder of the System Folder, so it
			might be one with a special icon.
		*/
		if ( Match_special_dirID( kAppleMenuFolderType, dirID ) )
			icon_id = appleMenuFolderIconResource;
		else if ( Match_special_dirID( kControlPanelFolderType, dirID ) )
			icon_id = controlPanelFolderIconResource;
		else if ( Match_special_dirID( kExtensionFolderType, dirID ) )
			icon_id = extensionsFolderIconResource;
		else if ( Match_special_dirID( kPreferencesFolderType, dirID ) )
			icon_id = preferencesFolderIconResource;
		else if ( Match_special_dirID( kPrintMonitorDocsFolderType, dirID ) )
			icon_id = printMonitorFolderIconResource;
		else if ( Match_special_dirID( kStartupFolderType, dirID ) )
			icon_id = startupFolderIconResource;
		else if ( Match_special_dirID( kFontsFolderType, dirID ) )
			icon_id = fontsFolderIconResource;
	}
	else
	{
		/* These attribute bits are described in TN 301. */
		if ((cpb->dirInfo.ioFlAttrib & 0x08) != 0)
			icon_id = mountedFolderIconResource;
		else if ((cpb->dirInfo.ioFlAttrib & 0x20) != 0)
			icon_id = sharedFolderIconResource;
		else if ((cpb->dirInfo.ioFlAttrib & 0x04) != 0)
			icon_id = ownedFolderIconResource;
		else
			icon_id = genericFolderIconResource;
	}
	
	save_resfile = CurResFile();
	UseResFile(0);
	err = Get_resource_icons( the_suite, icon_id, icon_selector );
	
	UseResFile( save_resfile );
	
	return err;
}

static Boolean Match_special_dirID( OSType folder_type, long dirID )
{
	OSErr	err;
	short	found_vRefNum;
	long	found_dirID;
	
	err = FindFolder( kOnSystemDisk, folder_type, kDontCreateFolder,
			&found_vRefNum, &found_dirID );
	
	return (err == noErr) && (found_dirID == dirID);
}
