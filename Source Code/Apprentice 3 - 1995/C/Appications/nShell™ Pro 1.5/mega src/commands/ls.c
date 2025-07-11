/* ========== the commmand file: ==========

	ls.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// Get the appropriate A4 stuff
#include <A4Stuff.h>
#else
#include <SetUpA4.h>
#endif

#include <Packages.h>

#include "nshc.h"
#include "walk_utl.h"

#include "arg_utl.proto.h"
#include "buf_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"
#include "fss_utl.proto.h"
#include "walk_utl.proto.h"

/* ======================================== */

#define IS_A_DIRECTORY	fpb->ioFlAttrib & 16

// define the walking states

typedef enum { ls_dont_walk, ls_walk } t_walks;

// data definition - this struct is the root of all data

typedef struct {

	short		arg;				// position in arg list
	
	short		first_time;			// 1 on first pass
	short		was_dir;			// 1 when a last item was a dir
	
	int			option_c;			// 1 when -c option is used
	int			option_l;			// 1 when -l option is used
	int			option_R;			// 1 when -R option is used
	
	char		time_flag;			// contians time flag (m=modified,g=creation,b=backup)
	long		cols;				// number of columns for -c option
	
	t_walk_hndl	wData;				// storage for the walk structure
	t_walks		walk_state;			// ls walk state

} t_ls_data;

typedef	t_ls_data	**t_ls_hndl;

/* ======================================== */

// local prototypes

OSErr ls_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr ls_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr ls_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void  ls_shift_args( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short arg, short distance );
OSErr ls_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData );
OSErr ls_many( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData );
OSErr ls_get_path(  short vRefNum, long DirID, Str255 s );
OSErr ls_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData, FSSpec *fss );

/* ======================================== */

static void lsHeader( t_nshc_calls *nshc_calls, t_ls_hndl lsData )
{
	switch( (**lsData).time_flag ) {
		case 'g' :
			buf_putStr( "\p Crea Type RSize DSize  Creation Date       Name" );
			break;
		case 'b' :
			buf_putStr( "\p Crea Type RSize DSize  Backup Date         Name" );
			break;
		default :
			buf_putStr( "\p Crea Type RSize DSize  Modification Date   Name" );
			break;
		}
	buf_putStr( "\p\r ---- ---- ----- ----- -------------------- ----\r" );
}

/* ======================================== */

static void lsSize( t_nshc_calls *nshc_calls, t_ls_hndl lsData, long size)
{
	char	units;
	int		blanks;
	Str255	buf;

	units = ' ';

	if (size > 9999) {
		size /= 1024;
		units = 'k';
		}

	if (size > 9999) {
		size /= 1024;
		units = 'M';
		}

	NumToString(size, buf);

	for (blanks = 4 - buf[0]; blanks > 0; blanks--)
		buf_putchar( ' ' );
	buf_putStr( buf );

	buf_putchar( units );
}

/* ======================================== */

static void lsLong( t_nshc_calls *nshc_calls, t_ls_hndl lsData, HFileInfo *fpb )
{
	int			i;
	OSType		type;
	char		*p;
	long		value;
	Str255		string;
	
	if ( IS_A_DIRECTORY ) {
	
		buf_putStr( "\p                      " );

		switch( (**lsData).time_flag ) {
			case 'g' :
				value = ((DirInfo *)fpb)->ioDrCrDat;
				break;
			case 'b' :
				value = ((DirInfo *)fpb)->ioDrBkDat;
				break;
			default :
				value = ((DirInfo *)fpb)->ioDrMdDat;
				break;
			}
				
		}
	else {
	
		buf_putchar( ' ' );

		type = fpb->ioFlFndrInfo.fdCreator;
		p = (char *) &type;
		for (i = 0; i < 4; i++)
			buf_putchar( *p++ );
		buf_putchar( ' ' );

		type = fpb->ioFlFndrInfo.fdType;
		p = (char *) &type;
		for (i = 0; i < 4; i++)
			buf_putchar( *p++ );

		buf_putchar( ' ' );
		lsSize( nshc_calls, lsData, fpb->ioFlRLgLen );
		
		buf_putchar( ' ' );
		lsSize( nshc_calls, lsData, fpb->ioFlLgLen );

		switch( (**lsData).time_flag ) {
			case 'g' :
				value = fpb->ioFlCrDat;
				break;
			case 'b' :
				value = fpb->ioFlBkDat;
				break;
			default :
				value = fpb->ioFlMdDat;
				break;
			}
	}
	
	buf_putchar( ' ' );

	IUDateString( value, shortDate, string );
	for (i = (8 - string[0]); i > 0; i--)
		buf_putchar( ' ' );
	buf_putStr( string );
	buf_putchar( ' ' );

	IUTimeString( value, 1, string );
	for (i = (11 - string[0]); i > 0; i--)
		buf_putchar( ' ' );
	buf_putStr( string );
	buf_putchar( ' ' );
		
}

/* ======================================== */

void ls_shift_args( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short arg, short distance )
{
	short		new_arg;

	new_arg = arg + distance;
	
	while ( new_arg < nshc_parms->argc ) {
		nshc_parms->argv[ arg ] = nshc_parms->argv[ new_arg ];
		arg++;
		new_arg++;
		}
		
	nshc_parms->argc -= distance;
}

/* ======================================== */

OSErr ls_get_path(  short vRefNum, long DirID, Str255 s )
{
	CInfoPBRec	block;
	Str255		directoryName;
	int			error;

	*s = 0;
	
	if ( DirID == 1 )
		return( 0 );
	
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = DirID;
	
	do {
	
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;

		error = PBGetCatInfo(&block,false);
		
		if ( ( s[0] + directoryName[0] + 1 ) > 255 )
			error = 1;
			
		if (!error) {
			pStrAppend(directoryName,"\p:");
			pStrAppend(directoryName,s);
			pStrCopy(s,directoryName);
			}

	} while (!error && (block.dirInfo.ioDrDirID != fsRtDirID));

	return(error);
}

/* ======================================== */

OSErr ls_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData )
{
	OSErr		result;
	FSSpec		fss;
	t_walk_hndl wData;
	Str255		path;
	
	if ( ( (**lsData).arg > 1 ) && ( (**lsData).arg >= nshc_parms->argc ) ) {
		nshc_parms->action = nsh_stop;
		return( 0 );
		}
	
	if ( nshc_parms->argc > 1 )
		result = arg_to_real_fss( nshc_parms, nshc_calls, (**lsData).arg, &fss );
	else {
		pStrCopy( path, "\p:" );
		result = nshc_calls->NSH_path_expand( path );
		if ( !result )
			result = nshc_calls->NSH_path_to_FSSpec( path, &fss );
		}

	(**lsData).arg++;
	
	if (result) {
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
		
	if ( (**lsData).option_R ) {
		wData = walk_init( &fss );
		
		if ( !wData ) {
			nshc_calls->NSH_putStr_err( "\pls: Could not init Walk Library.\r" );
			nshc_parms->action = nsh_stop;
			return( -1 );
			}
	
		(**lsData).wData = wData;
		(**lsData).walk_state = ls_walk;
		
		return( 0 );
		}

	return( ls_display( nshc_parms, nshc_calls, lsData, &fss ) );
}

/* ======================================== */

OSErr ls_many( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData )
{
	short		i;
	long		size;
	short		level;
	short		isDir;
	FSSpec		fss;
	OSErr		error;
	t_walk_hndl wData;
	FInfo		fndrInfo;
	
	wData = (**lsData).wData;
	
	if ( !wData ) {
		nshc_calls->NSH_putStr_err( "\pls: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
		
	do
		
		error = walk_next( wData, &fss, &level, &isDir );
		
	while ( !error && !isDir );
	
	if (error) {
		(**lsData).walk_state = ls_dont_walk;
		DisposeHandle( wData );
		(**lsData).wData = 0L;
		return(0);
		}

	return( ls_display( nshc_parms, nshc_calls, lsData, &fss ) );
}

/* ======================================== */

OSErr ls_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_ls_hndl lsData, FSSpec *fss )
{
	CInfoPBRec		cipbr;
	HFileInfo		*fpb;
	short			idx;
	Str255			localPath;
	long			foundDir;
	int				ci, cmax;
	int				count, blanks;
	
	// set up data block
	
	fpb = (HFileInfo *)&cipbr;
	fpb->ioFDirIndex = 0;
	fpb->ioVRefNum = fss->vRefNum;
	fpb->ioDirID = fss->parID;
	fpb->ioNamePtr = localPath;
	pStrCopy( localPath, fss->name );

	if (PBGetCatInfo( &cipbr, FALSE )) {
		nshc_calls->NSH_putStr_err("\pls: File or directory not found.\r");
		return(1);
		}
		
	// save id of found dir
		
	foundDir = fpb->ioDirID;
	
	// display the data

	if ( (**lsData).first_time && (**lsData).option_l )
		lsHeader( nshc_calls, lsData );
		
	if ( IS_A_DIRECTORY ) {

		if ( ( nshc_parms->argc > 2 ) || ((**lsData).walk_state == ls_walk ) ) {
		
			buf_putStr( "\p\rDir = " );
			ls_get_path( fss->vRefNum, foundDir, localPath );
			buf_putStr( localPath );
			buf_putchar( '\r' );
			
			}

		cmax = 0;
		
		if ( (**lsData).option_c )
			for (idx = 1; TRUE; idx++) {
				fpb->ioVRefNum = fss->vRefNum;
				fpb->ioDirID = foundDir;
				fpb->ioFDirIndex = idx;
				if (PBGetCatInfo( &cipbr, FALSE ))
					break;
				ci = localPath[0] + 2;
				if (ci > cmax)
					cmax = ci;
				}
				
		blanks = 0;
		count = 0;

		for( idx=1; TRUE; idx++) {
		
			fpb->ioVRefNum = fss->vRefNum;
			fpb->ioDirID = foundDir;
			fpb->ioFDirIndex = idx;
		
			if (PBGetCatInfo( &cipbr, FALSE )) break;
			
			if ( (**lsData).option_l )
				lsLong( nshc_calls, lsData, fpb );

			if ( (**lsData).option_c ) {
				while (blanks--)
					buf_putchar( ' ' );
				blanks = cmax - localPath[0];
				}

			buf_putStr( localPath );

			if ( IS_A_DIRECTORY ) {
				buf_putchar( ':' );
				blanks--;
				}
				
			if ( (**lsData).option_c ) {
				count++;
				if ( count >= (**lsData).cols ) {
					buf_putchar( '\r' );
					count = 0;
					blanks = 0;
					}
				}
			else
				buf_putchar( '\r' );
			
			}
			
		if ( count )
			buf_putchar( '\r' );
			
		(**lsData).was_dir = 1;

		}
	else {
		if ( (**lsData).was_dir )
			buf_putchar( '\r' );
		if ( (**lsData).option_l )
			lsLong( nshc_calls, lsData, fpb );
		buf_putStr( localPath );
		buf_putchar( '\r' );
		(**lsData).was_dir = 0;
		}
		
	(**lsData).first_time = 0;
	
	return(0);
}

/* ======================================== */

OSErr ls_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short		arg;
	short		usage;
	t_ls_hndl 	lsData;

	// get data space

	lsData = (t_ls_hndl)NewHandleClear( sizeof( t_ls_data ) );

	if (!lsData) {
		nshc_calls->NSH_putStr_err( "\pls: Could not allocate data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
		
	nshc_parms->data = (Handle)lsData;
	
	// set defaults
	
	usage = 0;	// good usage until determined otherwise

	(**lsData).arg = 1;						// start at the arg = 1 position
	(**lsData).walk_state = ls_dont_walk;	// start by initing a search
	(**lsData).first_time = 1;				// put up headers on first pass

	// read main options from command line
	
	(**lsData).option_R = nshc_got_option( nshc_parms, 'R' );
	(**lsData).option_l = nshc_got_option( nshc_parms, 'l' );

	(**lsData).time_flag = 'm';
	if (nshc_got_option( nshc_parms, 'b' )) (**lsData).time_flag = 'b';
	if (nshc_got_option( nshc_parms, 'g' )) (**lsData).time_flag = 'g';

	(**lsData).option_c = nshc_got_option( nshc_parms, 'c' );
	
	if ( arg = (**lsData).option_c )
			if ( nshc_is_numeric_operand( nshc_parms, ++arg ) )
			(**lsData).cols = arg_to_num( nshc_parms, arg );
		else
			usage = 1;
			
	if ( (**lsData).option_l )
		(**lsData).option_c = 0;
		
	// if usage error, report problem and exit
	
	if (usage) {
		nshc_calls->NSH_putStr_err("\pUsage: ls [<path1> <path2>...] [-c columns] [-l [-m] [-b] [-g]].\r");
		nshc_parms->action = nsh_stop;
		return( NSHC_ERR_PARMS );
		}
		
	// remove options (already read) from arg list
	
	if ( arg = nshc_got_option( nshc_parms, 'c' ) )
		ls_shift_args( nshc_parms, nshc_calls, arg, 2 );
		
	if ( arg = nshc_got_option( nshc_parms, 'R' ) )
		ls_shift_args( nshc_parms, nshc_calls, arg, 1 );
		
	if ( arg = nshc_got_option( nshc_parms, 'l' ) )
		ls_shift_args( nshc_parms, nshc_calls, arg, 1 );
	
	if ( arg = nshc_got_option( nshc_parms, 'g' ) )
		ls_shift_args( nshc_parms, nshc_calls, arg, 1 );
	
	if ( arg = nshc_got_option( nshc_parms, 'b' ) )
		ls_shift_args( nshc_parms, nshc_calls, arg, 1 );
	
	// set up walk library
	
	nshc_parms->action = nsh_continue;
}

/* ======================================== */

OSErr ls_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	OSErr		result;
	t_ls_hndl lsData;

	lsData = (t_ls_hndl)nshc_parms->data;

	if ( !lsData ) {
		nshc_calls->NSH_putStr_err( "\pls: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	if ( (**lsData).walk_state == ls_dont_walk )
		result = ls_one( nshc_parms, nshc_calls, lsData );
	else
		result = ls_many( nshc_parms, nshc_calls, lsData );
		
	return( result );
}

/* ======================================== */

OSErr ls_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_ls_hndl lsData;
	t_walk_hndl wData;

	lsData = (t_ls_hndl)nshc_parms->data;
	wData = (**lsData).wData;

	if (lsData)
		DisposeHandle( lsData );

	if (wData)
		DisposeHandle( wData );

	nshc_parms->action = nsh_idle;
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#else
	RememberA0();
	SetUpA4();
#endif

	buf_init( nshc_calls );
	
	if (!nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) {
	
 		// otherwise, handle requests from the application
	
	  	switch (nshc_parms->action) {
			case nsh_start:
				nshc_parms->result = ls_start( nshc_parms, nshc_calls );
				break;
			case nsh_continue:
				nshc_parms->result = ls_continue( nshc_parms, nshc_calls );
				break;
			case nsh_stop:
				nshc_parms->result = ls_stop( nshc_parms, nshc_calls );
				break;
			default:
				nshc_parms->result = NSHC_NO_ERR;
				nshc_parms->action = nsh_idle;
				break;
			}
		
		}
		
	buf_flush();
	
#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}
