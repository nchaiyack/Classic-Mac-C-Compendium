#include "Get_icon_file_name.h"
#include <Folders.h>

/*	------------------------------------------------------------------
	Get_icon_file_name			Get the name of the invisible files
								used to store custom icons for folders
								and volumes.
	
	This does assume that the name of Finder is "\pFinder".  I am told
	that is true even in foreign systems.
	------------------------------------------------------------------
*/
void	Get_icon_file_name( Str255 icon_file_name )
{
	short	save_resfile, sys_vRefNum, Finder_resfile;
	long	sys_dirID;
	
	save_resfile = CurResFile();
	(void) FindFolder( kOnSystemDisk, kSystemFolderType,
		  kDontCreateFolder, &sys_vRefNum, &sys_dirID );
	SetResLoad( false );
	Finder_resfile = HOpenResFile( sys_vRefNum, sys_dirID,
		"\pFinder", fsRdPerm );
	SetResLoad( true );
	if (Finder_resfile != -1)	// successful open
	{
		GetIndString( icon_file_name, 11250, 5 );
		CloseResFile( Finder_resfile );
		UseResFile( save_resfile );
	}
	else
	{
		BlockMove( "\pIcon\r", icon_file_name, 6 );
	}
}