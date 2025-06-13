/* ========== the commmand file: ==========

	find.c
	
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

#define	MAX_RULES 10

// define the types of search rules

typedef enum { rule_none,
			   rule_name,
			   rule_type,
			   rule_creator,
			   rule_and,
			   rule_or,
			   rule_files_only,
			   rule_dirs_only } t_rules;

// define the walking states

typedef enum { ws_none, ws_init, ws_next } t_walks;

// data definition - this struct is the root of all data

typedef struct {

	short		got_fss;					// 0 if FSSpec calls are not available
	short		arg;						// position in arg list
	t_walks		walk_state;					// interal walk state
	
	t_walk_hndl	wData;						// storage for the walk structure
	
	short		rule_count;					// number of rules in arg list
	short		rules[ MAX_RULES ];			// rule array
	Str255		patterns[ MAX_RULES ];		// pattern array
	OSType		ostypes[ MAX_RULES ];		// OS Type array

} t_find_data;

typedef	t_find_data	**t_find_hndl;

/* ======================================== */

// local prototypes

OSErr find_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr find_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr find_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
OSErr find_add_rule( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls , short rule, short arg );
OSErr find_get_OSType(t_nshc_parms *nshc_parms, int arg, OSType *os_type);
OSErr find_init( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_find_hndl fData );
OSErr find_next( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_find_hndl fData );

/* ======================================== */

OSErr find_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short		arg;
	short		usage;
	OSErr		error;
	FSSpec		fss;
	Str255		temp_str;
	t_find_hndl fData;

	// get data space

	fData = (t_find_hndl)NewHandleClear( sizeof( t_find_data ) );

	if (!fData) {
		nshc_calls->NSH_putStr_err( "\pfind: Could not allocate data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
	
	(**fData).got_fss = fss_test();				// test if we can use FSSpec calls
	(**fData).arg = 1;							// start at the arg = 1 position
	(**fData).walk_state = ws_init;				// start by initing a searchrule_

	nshc_parms->data = (Handle)fData;
	
	// set up parameters
	
	usage = 0;
		
	arg = 1;
	
	while ( arg < nshc_parms->argc ) {
	
		usage = arg_to_str( nshc_parms, nshc_calls, arg, temp_str );
		
		if (usage) break;
		
		if ( pStrEqual( temp_str, "\p-name" ) )
			usage = find_add_rule( nshc_parms, nshc_calls, rule_name, arg );
		else
		if ( pStrEqual( temp_str, "\p-type" ) ) 
			usage = find_add_rule( nshc_parms, nshc_calls, rule_type, arg );
		else
		if ( pStrEqual( temp_str, "\p-creator" ) )
			usage = find_add_rule( nshc_parms, nshc_calls, rule_creator, arg );
		else
		if ( pStrEqual( temp_str, "\p-a" ) ) 
			usage = find_add_rule( nshc_parms, nshc_calls, rule_and, arg );
		else
		if ( pStrEqual( temp_str, "\p-o" ) )
			usage = find_add_rule( nshc_parms, nshc_calls, rule_or, arg );
		else
		if ( pStrEqual( temp_str, "\p-f" ) )
			usage = find_add_rule( nshc_parms, nshc_calls, rule_files_only, arg );
		else
		if ( pStrEqual( temp_str, "\p-d" ) )
			usage = find_add_rule( nshc_parms, nshc_calls, rule_dirs_only, arg );
		else
			arg++;
			
		if (usage) break;
		
		}
	
	if ( nshc_parms->argc == 1 )
		usage = 1;

	if (usage) {
		nshc_calls->NSH_putStr_err( "\pUsage: find pathname [options]\r" );
		nshc_parms->action = nsh_stop;
		return( error );
		}
	
	// set up walk library
	
	nshc_parms->action = nsh_continue;
}

/* ======================================== */

OSErr find_add_rule( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls , short rule, short arg )
{
	short		usage;
	short		shift;
	short		new_arg;
	short		rule_no;
	t_find_hndl fData;
	
	fData = (t_find_hndl)nshc_parms->data;
	
	rule_no = (**fData).rule_count;
	
	(**fData).rules[ rule_no ] = rule;
	
	switch ( rule ) {
		case rule_name:
			usage = arg_to_str( nshc_parms, nshc_calls, arg+1, (**fData).patterns[rule_no] );
			shift = 2;
			break;
		case rule_type:
		case rule_creator:
			usage = find_get_OSType( nshc_parms, arg+1, &(**fData).ostypes[rule_no] );
			shift = 2;
			break;
		default:
			shift = 1;
		}
		
	new_arg = arg + shift;
	
	while ( new_arg < nshc_parms->argc ) {
		nshc_parms->argv[ arg ] = nshc_parms->argv[ new_arg ];
		arg++;
		new_arg++;
		}
		
	nshc_parms->argc -= shift;

	(**fData).rule_count = rule_no + 1;
	
	return( usage );
}

/* ======================================== */

OSErr find_get_OSType(t_nshc_parms *nshc_parms, int arg, OSType *os_type)
{
	char	c;
	char	*p;
	int		i;
	int		error;
	
	if (arg >= nshc_parms->argc)
		return(1);
	
	error = 0;
	*os_type = 0;
	
	p = &nshc_parms->arg_buf[ nshc_parms->argv[ arg ] ];
	
	for (i = 0 ; i < 4 ; i++ )
		if ( c = *p++ ) {
			*os_type = *os_type << 8;
			*os_type = *os_type + c;
			}
		else
			error = 1;
			
	if (*p) error = 1;
	
	return(error);
}
	
/* ======================================== */

OSErr find_init( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_find_hndl fData )
{
	OSErr		result;
	FSSpec		fss;
	t_walk_hndl wData;
	
	if ( (**fData).arg >= nshc_parms->argc ) {
		nshc_parms->action = nsh_stop;
		return( 0 );
		}

	result = arg_to_real_fss( nshc_parms, nshc_calls, (**fData).arg, &fss );

	(**fData).arg++;
	
	if (result) {
		nshc_parms->action = nsh_stop;
		return( -1 );
		}
	
	wData = walk_init( &fss );
	
	if ( !wData ) {
		nshc_calls->NSH_putStr_err( "\pfind: Could not init Walk Library.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	(**fData).wData = wData;
	(**fData).walk_state = ws_next;
}

/* ======================================== */

OSErr find_next( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_find_hndl fData )
{
	short		i;
	short		match;
	short		rule;
	short		level;
	short		isDir;
	FSSpec		fss;
	OSErr		error;
	t_walk_hndl wData;
	FInfo		fndrInfo;
	
	wData = (**fData).wData;
	
	if ( !wData ) {
		nshc_calls->NSH_putStr_err( "\pfind: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	error = walk_next( wData, &fss, &level, &isDir );
	
	if (error) {
		(**fData).walk_state = ws_init;
		DisposeHandle( wData );
		(**fData).wData = 0L;
		return( 0 );
		}

	match = 1;

	for ( rule = 0 ; rule < (**fData).rule_count; rule++ )
		switch ( (**fData).rules[ rule ] ) {
			case rule_name:
				switch ( nshc_calls->NSH_match( (**fData).patterns[rule], fss.name ) ) {
					case 0:
						break;
					case 1:
						match = 0;
						break;
					default:
						nshc_calls->NSH_putStr_err( "\pfind: Bad Pattern.\r" );
						nshc_parms->action = nsh_stop;
						return( -1 );
					}
				break;
			case rule_type:
				fss_GetFInfo((**fData).got_fss, &fss, &fndrInfo);
				if (fndrInfo.fdType != (**fData).ostypes[rule] )
					match = 0;
				break;
			case rule_creator:
				fss_GetFInfo((**fData).got_fss, &fss, &fndrInfo);
				if (fndrInfo.fdCreator != (**fData).ostypes[rule] )
					match = 0;
				break;
			case rule_and:
				if ( !match )
					rule = 1000;
				break;
			case rule_or:
				if ( match )
					rule = 1000;
				else
					match = 1;
				break;
			case rule_files_only:
				if ( isDir )
					match = 0;
				break;
			case rule_dirs_only:
				if ( !isDir )
					match = 0;
				break;
			}

	if (match) {
		buf_init( nshc_calls );
		for ( i = 1 ; i <= level; i++ ) {
			buf_putStr( (**wData).levels[i].fss.name );
			buf_putchar( ':' );
			}
		buf_putStr( fss.name );
		buf_putchar( '\r' );
		buf_flush();
		}

	return(0);
}

	
/* ======================================== */

OSErr find_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	OSErr		result;
	t_find_hndl fData;

	fData = (t_find_hndl)nshc_parms->data;

	if ( !fData ) {
		nshc_calls->NSH_putStr_err( "\pfind: Missing data.\r" );
		nshc_parms->action = nsh_stop;
		return( -1 );
		}

	if ( (**fData).walk_state == ws_init )
		result = find_init( nshc_parms, nshc_calls, fData );
	else
		result = find_next( nshc_parms, nshc_calls, fData );
		
	return( result );
}

/* ======================================== */

OSErr find_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_find_hndl fData;
	t_walk_hndl wData;

	fData = (t_find_hndl)nshc_parms->data;
	wData = (**fData).wData;

	if (fData)
		DisposeHandle( fData );

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
	
	if (!nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION )) {
	
 		// otherwise, handle requests from the application
	
	  	switch (nshc_parms->action) {
			case nsh_start:
				nshc_parms->result = find_start( nshc_parms, nshc_calls );
				break;
			case nsh_continue:
				nshc_parms->result = find_continue( nshc_parms, nshc_calls );
				break;
			case nsh_stop:
				nshc_parms->result = find_stop( nshc_parms, nshc_calls );
				break;
			default:
				nshc_parms->result = NSHC_NO_ERR;
				nshc_parms->action = nsh_idle;
				break;
			}
		
		}
		
#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}
