#include <stdio.h>
#include <PrintTraps.h>
#include <Folders.h>
#include "mac-specific.h"
#include "dvihead.h"
#include "gendefs.h"
#include "gblprocs.h"

Boolean TrapAvailable( short trapnum );

#ifndef GESTALT
#define GESTALT			0xA1AD
#endif

#ifndef NIL
#define NIL		0L
#endif

#define		PREF_TYPES	4

short	g_pref_resfile;	/* reference number of preferences file */
short	g_pref_vRefNum;	/* vRefNum of preferences file */

void	Find_prefs( void );

void	Find_prefs( void )
{
	long	response;
	OSErr	err;
	long	dirID, procID;
	SysEnvRec	world;
	HParamBlockRec	hpb;
	Str255	name;
	Boolean	newfile;
	ResType	pref_type[PREF_TYPES]
		= {'Cstr', 'int ', 'WPos', 'Bool'};
	short	pt_num;
	ResType	what_type;
	THPrint	print_rec_h;
	Handle	a_res;
	short	index, res_id;
	Str255	standard_name = "\pDVIM72-Mac Prefs";
	short	res_attr;
	
	g_pref_resfile = -1;
	newfile = false;
	
	/*
		First on the agenda is to find the right folder.  If the
		Folder Manager is available, we use if to find the Preferences
		folder, otherwise we use SysEnvirons to find the System Folder.
	*/
	if (!TrapAvailable(GESTALT) ||
		(noErr != Gestalt( gestaltFindFolderAttr, &response )) ||
		!(response & 1L) ||
		(noErr != (err = FindFolder( kOnSystemDisk,'pref', kCreateFolder,
			&g_pref_vRefNum, &dirID))) )
	{
		err = SysEnvirons( 1, &world );
		procID = NIL;
		err = GetWDInfo( world.sysVRefNum, &g_pref_vRefNum, &dirID, &procID );
	}
	/*
		At this point, unless something awful has happened, we have the
		dirID and vRefNum of the appropriate folder.  Next we try to
		find the file, which should have creator 'dviM' and type 'dviP'.
	*/
	hpb.ioParam.ioCompletion = NIL;
	hpb.ioParam.ioNamePtr = name;
	hpb.ioParam.ioVRefNum = g_pref_vRefNum;
	hpb.fileParam.ioDirID = dirID;
	hpb.fileParam.ioFDirIndex = 1;
	err = noErr;
	do {
		err = PBHGetFInfo( &hpb, false );
		hpb.fileParam.ioDirID = dirID;
		hpb.fileParam.ioFDirIndex ++;
	} while ( (err == noErr) &&
		((hpb.fileParam.ioFlFndrInfo.fdType != 'dviP') ||
			(hpb.fileParam.ioFlFndrInfo.fdCreator != 'dviM') ) );
	/*
		If we didn't find the file, we need to create it.
	*/
	if (err != noErr)
	{
		BlockMove( standard_name, name, 256 );
		err = HCreate( g_pref_vRefNum, dirID, name, 'dviM', 'dviP' );
		if (err == noErr)
		{
			HCreateResFile( g_pref_vRefNum, dirID, name );
			err = ResError();
		}
		if (err == noErr)
			Show_error( "Preferences file not found; a new one "
				"will be created." );
		else
			Show_error( "Preferences file not found; error creating "
				"new one." );
		newfile = true;
	}
	if (err != noErr)
		return;	/* give up */
	/*
		Now, unless we have an error code, we have a file.
		Time to open it.
	*/
	g_pref_resfile = HOpenResFile( g_pref_vRefNum, dirID, name, fsRdWrPerm );
	if (g_pref_resfile == -1)
	{
		fatal( "Error opening preferences file." );
	}
	/*
		If we just created this file, we need to copy some resources to it
		from our application.
	*/
	if (newfile)
	{
		print_rec_h = (THPrint) NewHandle( sizeof(TPrint) );
		if ( (print_rec_h == nil) || (MemError() != noErr) )
		{
			fatal("Find_prefs: Can't get memory for print record.");
		}
		PrOpen();
		if (PrError() != noErr)
		{
			fatal("Find_prefs: Can't open printer driver.");
		}
		PrintDefault( print_rec_h );
		PrClose();
		AddResource( (Handle) print_rec_h, 'Prec', 128, "\p" );
		if (ResError() != noErr)
		{
			fatal("Find_prefs: can't add 'Prec' resource.");
		}
		for (pt_num = 0; pt_num < PREF_TYPES; pt_num++)
		{
			what_type = pref_type[pt_num];
			for (index = 1; ; index++)
			{
				UseResFile( g_app_resfile );
				a_res = Get1IndResource( what_type, index );
				if (a_res == NIL)
					break;
				GetResInfo( a_res, &res_id, &what_type, name );
				res_attr = GetResAttrs( a_res );
				DetachResource( a_res );
				UseResFile( g_pref_resfile );
				AddResource( a_res, what_type, res_id, name );
				SetResAttrs( a_res, res_attr );
				ChangedResource( a_res );
			}
		}
		UseResFile( g_pref_resfile );
		UpdateResFile( g_pref_resfile );
		err = FlushVol( NIL, g_pref_vRefNum );
	}
}