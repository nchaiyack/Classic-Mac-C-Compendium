/* ==========================================

	drop.c - drag and drop stuff
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include <GestaltEqu.h>

#include "nsh.h"
#include "multi.h"
#include "interp.h"

#include "drop.proto.h"
#include "interp.proto.h"
#include "multi.proto.h"
#include "nsh.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "vlist.proto.h"
#include "utility.proto.h"
#include "str_utl.proto.h"

/* ============================================= */

#define PARMS_MAX 10240

char	gParms[PARMS_MAX];
int		gParmIndex;
int		gParmCount;

int		gTooLate;

short	g_drop_vol;
long	g_drop_dir;
Str255	g_drop_path;
Str255	g_drop_name;

extern	short	g_appl_vol;
extern	long	g_appl_dir;
extern	Str255	g_appl_path;
extern	Str255	g_appl_name;

extern	Boolean	gHasFindFolder;

/* ============================================= */

void drop_init( void )
{
	gParmIndex = 0;
	gParmCount = 0;
	gTooLate = 0;
}

/* ========================================== */

// install passed parameters;

int drop_vars( ShellH shell, InterpH interp )
{
	int		i;
	int		len;
	int		error;
	int		arg;
	char	*args;
	Str32	name;
	Str255	value;
	
	error = vlist_push_bead( shell );

	if (!error)
		error  = vlist_set_parm( shell, "\p0", g_drop_name );

	i = 0;
	arg = 1;

	HLock( shell );

	while (!error && (arg <= gParmCount)) {
	
		NumToString( arg, name );
	
		args = &gParms[i];
		
		len = cStrLen( args );
			
		if ( len > 255 ) {
			pipe_putStr_err( shell, "\pscript: Parameter is too long\r");
			(**interp).action = nsh_stop;
			}
		else
			pStrFromC( value, args );
			
		i = i + len + 1;
			
		error = vlist_set_parm( shell, name, value );
		
		arg++;
	}
	
	HUnlock( shell );
	
	if (!error) {
		NumToString(gParmCount+1,value);
		error  = vlist_set_parm( shell, "\p#", value );
		}

	if (error)
		pipe_putStr_err( shell, "\pscript: Could not allocate shell parameters.\r");
	
	return( error );
}
		
/* ============================================= */

void drop_open( void )
{
	ShellH	shell;
	InterpH	interp;
	
	shell = multi_allocate();
			
	if (!shell) {
		error_note( "\p could not allocate shell data." );
		return;
		}
	
	pStrCopy((**shell).shell_fss.name, g_drop_name);
	SetWTitle((**shell).WindPtr, g_drop_name);
	ShowWindow((**shell).WindPtr);

	interp = interp_new( shell );
	
	if (interp) {
	
		pStrCopy( (**interp).script_fss.name, g_drop_name );
		(**interp).script_fss.parID = g_drop_dir;
		(**interp).script_fss.vRefNum = g_drop_vol;
		(**interp).source = 1;
		(**interp).action = nsh_start;
		
		gTooLate = 1;		// too late to accept any more vars

		drop_vars( shell, interp );
		
		}
	else 
		pipe_putStr_err( shell, "\pscript: Could not allocate interpreter data.\r" );
}

/* ========================================== */

void drop_filename( Str255 filename )
{
	int	i,j;

	j = filename[0];
	
	if ( gParmIndex + j + 1 > PARMS_MAX )
		return;
	
	for ( i = 0; i < j; i++ )
		gParms[ gParmIndex + i ] = filename[ i + 1 ];
		
	gParms[ gParmIndex + j ] = 0;
	
	gParmIndex = gParmIndex + j + 1;
	gParmCount++;
}

/* ========================================== */

void drop_fss( FSSpec *fss )
{
	int		error;
	Str255	full_path;
	
	error = 0;
	
	if ( fss->parID == fsRtParID ) {
	
		// if this is a volume, deal with it
	
		pStrCopy( full_path, fss->name );
		full_path[ ++full_path[0] ] = ':';
		
		}
	else {
	
		// if this is a file or folder, deal with it
	
		error = path_from_dir( fss->vRefNum, fss->parID, full_path );
		
		if (!error) pStrAppend( full_path, fss->name );
			
		}
		
	if (!error)
		drop_filename( full_path );
}

/* ======================================== */

int drop_search_one( short vol )
{
	DTPBRec		desktopParams;
	OSErr		myErr;
	int			found;
	
	found = 0;

	desktopParams.ioVRefNum = vol;
	desktopParams.ioNamePtr = nil;
	
	myErr = PBDTGetPath(&desktopParams);
	
	if (!myErr && (desktopParams.ioDTRefNum != 0)) {
	
		desktopParams.ioIndex = 0;
		desktopParams.ioFileCreator = APPLICATION_SIGNATURE;
		desktopParams.ioNamePtr = g_appl_name;
		
		myErr = PBDTGetAPPLSync(&desktopParams);
		
		if (!myErr) {
		
			// okay, found it; fill in the application file spec
			// and set the flag indicating we're done
			
			g_appl_dir = desktopParams.ioAPPLParID;
			g_appl_vol = desktopParams.ioVRefNum;
			found = 1;
			
		}
	}
	
	return( found );	
}

int drop_search_others( Boolean remote_search )
{
	HParamBlockRec			hfsParams;
	FInfo					documentFInfo;
	short					volumeIndex;
	int						found;
	GetVolParmsInfoBuffer	volumeInfoBuffer;
	OSErr					myErr;
	
	found = 0;
	volumeIndex = 0;
	
	while (!found) {
		
		volumeIndex++;
			
		// convert the volumeIndex into a vRefNum
		
		hfsParams.volumeParam.ioNamePtr = nil;
		hfsParams.volumeParam.ioVRefNum = 0;
		hfsParams.volumeParam.ioVolIndex = volumeIndex;
		myErr = PBHGetVInfoSync(&hfsParams);
		
		// a nsvErr indicates that the current pass is over
		if (myErr) return( 0 );
		
		// since we handled the document volume during the documentPass,
		// skip it if we have hit that volume again
		
		if (hfsParams.volumeParam.ioVRefNum != g_drop_vol) {
		
			// call GetVolParms to determine if this volume is a server
			// (a remote volume)
			
			hfsParams.ioParam.ioBuffer = (Ptr) &volumeInfoBuffer;
			hfsParams.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
			myErr = PBHGetVolParmsSync(&hfsParams);
			
			if (myErr) return( 0 );
			
			// if the vMServerAdr field of the volume information buffer
			// is zero, this is a local volume; skip this volume
			// if it's local on a remote pass or remote on a local pass
			
			if ( ( volumeInfoBuffer.vMServerAdr == 0 ) != remote_search )
				found = drop_search_one( hfsParams.volumeParam.ioVRefNum );
				
			}
				
	}
	
	return( found );
}

/* ======================================== */

// Previously, we have set up g_appl... to point to our application
// This routine moves that info to g_drop... and tries to find the nShell
// application and put that info in g_appl...
	
void drop_get_home( void )
{
	int			found;

	// copy g_appl... to g_drop...
	
	g_drop_vol = g_appl_vol;
	g_drop_dir = g_appl_dir;
	pStrCopy( g_drop_path, g_appl_path );
	pStrCopy( g_drop_name, g_appl_name );
	
	// bail if the desktop database stuff is absent
	
	if ( !gHasFindFolder )
		return;

	// search this volume first

	found = drop_search_one( g_drop_vol );
	
	// then search disks on this node
	
	if (!found)
		found = drop_search_others( 0 );

	// then search the network

	if (!found)
		found = drop_search_others( 1 );
		
	if (!found) {
		Notify( "\pThis script requires that an nShellª or nShell-Proª application be installed on your system.\r\rRun canceled.\r\rNewport Software Development\rP.O. Box 1485\rNewport Beach, CA 92659", 1 );
		nsh_quit();
		}
}

