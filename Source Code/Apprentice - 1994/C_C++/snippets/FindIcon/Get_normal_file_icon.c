#include <Finder.h>
#include <Folders.h>
#include "Get_normal_file_icon.h"
#include <exceptions.h>
#include "Find_generic_icon_id.h"
#include "Copy_each_icon.h"
#include "Is_suite_empty.h"
#include "Get_resource_icons.h"

/*	------------------------------------------------------------------
	Get_normal_file_icon		Get an icon suite for a file that does
								not have a custom icon.
	------------------------------------------------------------------
*/

static	short	Find_desktop_database(
/* --> */	short first_vRefNum,
/* --> */	OSType file_creator );	// returns a DT refnum or 0

static	Boolean	In_one_desktop(
/* --> */	short	vRefNum,
/* --> */	OSType	file_creator,
/* <-- */	short	*dt_refnum
);

static	pascal OSErr Get_icon( ResType theType, Handle *theIcon,
			void *yourDataPtr );

typedef struct 
{
	OSType		file_creator;
	OSType		file_type;
	short		DTRefNum;
} Get_icon_data;

OSErr	Get_normal_file_icon(
/* --> */	CInfoPBRec			*cpb,
/* --> */	IconSelectorValue	icon_selector,
/* <-- */	Handle	*the_suite
)
{
	OSErr	err;
	long	data_size;
	Handle	icon_data;
	Byte	icon_type;
	Get_icon_data	get_data;
	short	icon_id;
	Boolean	in_Finder;
	short	save_resfile, Finder_resfile, sys_vRefNum;
	long	sys_dirID;
	
	icon_id = Find_generic_icon_id( cpb->hFileInfo.ioFlFndrInfo.fdType,
		&in_Finder );
	save_resfile = CurResFile();
	
	if (in_Finder)
	{
		(void) FindFolder( kOnSystemDisk, kSystemFolderType,
			  kDontCreateFolder, &sys_vRefNum, &sys_dirID );
		SetResLoad( false );
		Finder_resfile = HOpenResFile( sys_vRefNum, sys_dirID,
			"\pFinder", fsRdPerm );
		SetResLoad( true );
		forbid_action( Finder_resfile == -1, HOpenResFile,
			err = ResError() );
		err = Get_resource_icons( the_suite, icon_id, icon_selector );
		CloseResFile( Finder_resfile );
HOpenResFile:	;
	}
	else	// icons in desktop DB or in System
	{
		get_data.DTRefNum = Find_desktop_database( cpb->dirInfo.ioVRefNum,
			cpb->hFileInfo.ioFlFndrInfo.fdCreator );
		if ( get_data.DTRefNum != 0 )	// the right icons are in some desktop
		{
			err = NewIconSuite( the_suite );
			forbid( err, NewIconSuite );
			
			get_data.file_creator = cpb->hFileInfo.ioFlFndrInfo.fdCreator;
			get_data.file_type = cpb->hFileInfo.ioFlFndrInfo.fdType;
			if (get_data.file_type == kApplicationAliasType)
				get_data.file_type = 'APPL';
			err = ForEachIconDo( *the_suite, icon_selector,
				Get_icon, &get_data );
		NewIconSuite:
			;
		}
		
		if ( (get_data.DTRefNum == 0) || Is_suite_empty( *the_suite ) )
		{
			UseResFile( 0 );
			err = Get_resource_icons( the_suite, icon_id, icon_selector );
		}
	}

	UseResFile( save_resfile );

	return err;
}

/*	------------------------------------------------------------------
	Get_icon		This is an IconAction procedure to fill in one
					slot of an icon suite, given a file type, creator,
					and desktop database.
	------------------------------------------------------------------
*/
static	pascal OSErr Get_icon( ResType theType, Handle *theIcon,
			void *yourDataPtr )
{
	OSErr	err;
	Get_icon_data	*data;
	DTPBRec			desk_rec;
	
	err = noErr;
	data = (Get_icon_data *) yourDataPtr;

	*theIcon = NewHandle( kLarge8BitIconSize );
	require_action( *theIcon, NewHandle, err = memFullErr );
	HLock( *theIcon );

	desk_rec.ioDTRefNum = data->DTRefNum;
	desk_rec.ioDTBuffer = **theIcon;
	desk_rec.ioDTReqCount = kLarge8BitIconSize;
	desk_rec.ioFileCreator = data->file_creator;
	desk_rec.ioFileType = data->file_type;
	switch (theType)
	{
		case large1BitMask:
			desk_rec.ioIconType = kLargeIcon;
			break;
		case large4BitData:
			desk_rec.ioIconType = kLarge4BitIcon;
			break;
		case large8BitData:
			desk_rec.ioIconType = kLarge8BitIcon;
			break;
		case small1BitMask:
			desk_rec.ioIconType = kSmallIcon;
			break;
		case small4BitData:
			desk_rec.ioIconType = kSmall4BitIcon;
			break;
		case small8BitData:
			desk_rec.ioIconType = kSmall8BitIcon;
			break;
		
		default:
			// The desktop database does not have "mini" icons
			desk_rec.ioIconType = 1000;
			break;
	}
	err = PBDTGetIconSync( &desk_rec );
	if (err == noErr)
	{
		HUnlock( *theIcon );
		SetHandleSize( *theIcon, desk_rec.ioDTActCount );
	}
	else
	{
		DisposeHandle( *theIcon );
		*theIcon = NULL;
		err = noErr;
	}
	
NewHandle:
	return err;
}

/*	------------------------------------------------------------------
	Find_desktop_database			Find the reference number of a
									desktop database containing icons
									for a specified creator code.
	The search begins on a specified volume, but covers all volumes.
	------------------------------------------------------------------
*/
static	short	Find_desktop_database(
/* --> */	short first_vRefNum,
/* --> */	OSType file_creator )	// returns a DT refnum or 0
{
	OSErr	err;
	VolumeParam		vpb;
	short			DTRefNum;

	DTRefNum = 0;
	
	if (!In_one_desktop(first_vRefNum, file_creator, &DTRefNum))
	{
		vpb.ioNamePtr = NULL;
		for (vpb.ioVolIndex = 1;
			PBGetVInfoSync( (ParmBlkPtr) &vpb ) == noErr;
			++vpb.ioVolIndex )
		{
			if ( vpb.ioVRefNum == first_vRefNum )
				continue;
			if ( In_one_desktop( vpb.ioVRefNum, file_creator,
				&DTRefNum ) )
				break;
		}
	}
	
	return DTRefNum;
}

/*	------------------------------------------------------------------
	In_one_desktop			Determine whether the desktop database for
							one particular volume contains icons for
							a given creator code, and if so, return its
							reference number.
	------------------------------------------------------------------
*/
static	Boolean	In_one_desktop(
/* --> */	short	vRefNum,
/* --> */	OSType	file_creator,
/* <-- */	short	*dt_refnum
)
{
	OSErr		err;
	DTPBRec		desk_rec;
	Boolean		retval;
	
	retval = false;	// default to failure
	desk_rec.ioNamePtr = NULL;
	desk_rec.ioVRefNum = vRefNum;
	err = PBDTGetPath( &desk_rec );
	forbid( err, PBDTGetPath );
	
	/*
		We want to ignore any non-icon data, such as the 'paul'
		item that is used for drag-and-drop.
	*/
	desk_rec.ioFileCreator = file_creator;
	desk_rec.ioIndex = 1;
	do {
		desk_rec.ioTagInfo = 0;
		err = PBDTGetIconInfoSync( &desk_rec );
		desk_rec.ioIndex += 1;
	} while ( (err == noErr) && (desk_rec.ioIconType <= 0) );
	if (err == noErr)
	{
		retval = true;
		*dt_refnum = desk_rec.ioDTRefNum;
	}
	
PBDTGetPath:
	return retval;
}
