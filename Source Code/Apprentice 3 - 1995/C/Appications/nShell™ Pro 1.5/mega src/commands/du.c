/* ========== the commmand file: ==========

	du.c
	
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

#include "nshc.h"
#include "walk_utl.h"

#include "arg_utl.proto.h"
#include "buf_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"
#include "fss_utl.proto.h"
#include "walk_utl.proto.h"

/* ======================================== */

// define the walking states

typedef enum { ws_none, ws_init, ws_next } t_walks;

// data definition - this struct is the root of all data

typedef struct {

	short		arg;						// position in arg list
	t_walks		walk_state;					// interal walk state
	
	short		show_all;					// 1 when -a option present
	short		summary_only;				// 1 when -s option present
	
	Str32		names[MAX_WALK_LEVELS];		// the path being processed
	long		totals[MAX_WALK_LEVELS];	// the totals being calculated
	
	short		old_level;					// monitor when we pop up a level
	short		old_dir;					// 1 if last item was a dir
	Str32		old_name;					// name of last item
	
	t_walk_hndl	wData;						// storage for the walk structure

} t_du_data;

typedef	t_du_data	**t_du_hndl;

/* ======================================== */

// local prototypes

OSErr du_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr du_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr du_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void  du_shift_args( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short arg );
OSErr du_init( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_du_hndl duData );
OSErr du_next( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_du_hndl duData );
OSErr du_size_of( FSSpec *spec, long *size );
void  du_report_level( t_nshc_calls *nshc_calls, t_du_hndl duData, long size,  short level, Str32 name );

/* ======================================== */

void du_shift_args( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short arg )
{
	short		new_arg;

	new_arg = arg + 1;
	
	while ( new_arg < nshc_parms->argc ) {
		nshc_parms->argv[ arg ] = nshc_parms->argv[ new_arg ];
		arg++;
		new_arg++;
		}
		
	nshc_parms->argc--;
}

/* ======================================== */

OSErr du_init( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_du_hndl duData )
{
	OSErr		result;
	FSSpec		fss;
	t_walk_hndl wData;
	Str255		path;
	
	if ( ( (**duData).arg > 1 ) && ( (**duData).arg >= nshc_parms->argc ) ) {
		nshc_parms->action = nsh_stop;
		return( 0 );
		}
	
	if ( nshc_parms->argc > 1 )
		result = arg_to_real_fss( nshc_parms, nshc_calls, (**duData).arg, &fss );
	else {
		pStrCopy( path, "\p:" );
		result = nshc_calls->NSH_path_expand( path );
		if ( !result )
			result = nshc_calls->NSH_path_to_FSSpec( path, &fss );
		}

	(**duData).arg++;
	
	if (result) {
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
	
	wData = walk_init( &fss );
	
	if ( !wData ) {
		nshc_calls->NSH_putStr_err( "\pdu: Could not init Walk Library.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	if ( (**duData).arg > 2 )
		buf_putchar( '\r' );		// put a blank line between sets of data

	(**duData).wData = wData;
	(**duData).walk_state = ws_next;
	(**duData).old_level = 0;
	(**duData).old_dir = 0;
	(**duData).old_name[0] = 0;
}

/* ======================================== */

OSErr du_next( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_du_hndl duData )
{
	short		i;
	long		size;
	short		level;
	short		isDir;
	FSSpec		fss;
	OSErr		error;
	t_walk_hndl wData;
	FInfo		fndrInfo;
	
	wData = (**duData).wData;
	
	if ( !wData ) {
		nshc_calls->NSH_putStr_err( "\pdu: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	error = walk_next( wData, &fss, &level, &isDir );
	
	if ( ( level <= (**duData).old_level ) && (**duData).old_dir )
		du_report_level( nshc_calls, duData, 0L, (**duData).old_level, (**duData).old_name );

	if ( level < (**duData).old_level )
		for ( i = (**duData).old_level ; i > level ; i-- ) {
			size = (**duData).totals[i];
			(**duData).totals[i] = 0L;
			du_report_level( nshc_calls, duData, size, i, 0L );
			(**duData).totals[i - 1] += size;
			}
			
	if (error) {
		(**duData).walk_state = ws_init;
		DisposeHandle( wData );
		(**duData).wData = 0L;
		(**duData).old_level = 0;
		return(0);
		}
		
	if ( level > (**duData).old_level )
		for ( i = 1 ; i <= level ; i++ )
			pStrCopy( (**duData).names[i], (**wData).levels[i].fss.name );

	(**duData).old_level = level;
	(**duData).old_dir = isDir;
	pStrCopy( (**duData).old_name, fss.name );
	
	if (isDir)
		return( 0 );

	error = du_size_of( &fss, &size );

	if ( error ) {
		nshc_calls->NSH_putStr_err( "\pdu: Could not read file size.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
	
	(**duData).totals[level] += size;
	
	if ( (**duData).show_all )
		du_report_level( nshc_calls, duData, size, level, fss.name );

	return(0);
}

/* ========================================== */

void du_report_level( t_nshc_calls *nshc_calls,
					  t_du_hndl duData,
					  long size,
					  short level,
					  Str32 name )
{
	short	i;
	Str32	temp_str;
	
	if ( (**duData).summary_only && ( ( level > 1 ) || name ) )
		return;
	
	NumToString( ( size + 512 ) / 1024, temp_str );
	i = 7 - temp_str[0];
	while (i--) buf_putchar( ' ' );
	buf_putStr( temp_str );
	buf_putchar( ' ' );

	for ( i = 1 ; i <= level; i++ ) {
		buf_putStr( (**duData).names[i] );
		buf_putchar( ':' );
		}
	
	if ( name )
		buf_putStr( name );
		
	buf_putchar( '\r' );
}

/* ========================================== */

OSErr du_size_of( FSSpec *spec, long *size )
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;
	error = PBGetCatInfoSync(&pb);
	
	if ( error || (pb.hFileInfo.ioFlAttrib & 16) )	// if is a dir, return 0
		*size = 0;
	else
		*size = pb.hFileInfo.ioFlPyLen + pb.hFileInfo.ioFlRPyLen;
	
	return (error);
}

/* ======================================== */

OSErr du_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short		arg;
	t_du_hndl	duData;

	// get data space

	duData = (t_du_hndl)NewHandleClear( sizeof( t_du_data ) );

	if (!duData) {
		nshc_calls->NSH_putStr_err( "\pdu: Could not allocate data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
	
	nshc_parms->data = (Handle)duData;
	
	// set defaults
	
	(**duData).arg = 1;							// start at the arg = 1 position
	(**duData).walk_state = ws_init;			// start by initing a searchrule_

	// set up options
			
	(**duData).show_all = nshc_got_option( nshc_parms, 'a' );
	
	(**duData).summary_only = nshc_got_option( nshc_parms, 's' );
	
	// and remove options from parameter list

	if ( arg = nshc_got_option( nshc_parms, 'a' ) )
		du_shift_args( nshc_parms, nshc_calls, arg );
		
	if ( arg = nshc_got_option( nshc_parms, 's' ) )
		du_shift_args( nshc_parms, nshc_calls, arg );
		
	// and continue
	
	nshc_parms->action = nsh_continue;
}

/* ======================================== */

OSErr du_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	OSErr		result;
	t_du_hndl duData;

	duData = (t_du_hndl)nshc_parms->data;

	if ( !duData ) {
		nshc_calls->NSH_putStr_err( "\pdu: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	if ( (**duData).walk_state == ws_init )
		result = du_init( nshc_parms, nshc_calls, duData );
	else
		result = du_next( nshc_parms, nshc_calls, duData );
		
	return( result );
}

/* ======================================== */

OSErr du_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_du_hndl duData;
	t_walk_hndl wData;

	duData = (t_du_hndl)nshc_parms->data;
	wData = (**duData).wData;

	if (duData)
		DisposeHandle( duData );

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
				nshc_parms->result = du_start( nshc_parms, nshc_calls );
				break;
			case nsh_continue:
				nshc_parms->result = du_continue( nshc_parms, nshc_calls );
				break;
			case nsh_stop:
				nshc_parms->result = du_stop( nshc_parms, nshc_calls );
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
