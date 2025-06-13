/* ========== the commmand file: ==========

	match.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#define	MATCH_BUF_SIZE	2048

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <script.h>
#include <string.h>

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		by_file;		// 1 if files are listed, 0 if stdin
	int		got_fss;		// 1 if FSSpec calls are available
	int		arg;			// next file to process in arg list
	int		found;			// 1 if the pattern was found at least once
	short	p_arg;			// position of '-p' in arg list
	short	fref;			// file ref. number, 0 if no file open
	Str255	pattern;		// the pattern for which we search
	Str255	line;			// the line we are currently looking at
	int		line_pos;		// the place to put the next char in 'line'

} t_match_data;

typedef	t_match_data	**t_match_hndl;

/* ======================================== */

// prototypes - utility

void match_bad( t_nshc_parms *nshc_parms, int code );
void match_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void match_good( t_nshc_parms *nshc_parms, t_match_hndl mData );

// prototypes - file copy routines

void match_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData );
void match_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData );

// prototypes - console copy routines

void match_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData );

// prototypes - state machine

void match_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void match_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void match_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void match_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void match_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pmatch: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void match_good( t_nshc_parms *nshc_parms, t_match_hndl mData )
{
	nshc_parms->action = nsh_stop;
	
	if ( (**mData).found )
		nshc_parms->result = 0;
	else
		nshc_parms->result = 1;
}

/* ======================================== */

// file access routines

/* ======================================== */

void match_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	(**mData).fref = 0;
	(**mData).line_pos = 0;

	// =====> skip the -p option
	
	if ( (**mData).arg == (**mData).p_arg )
		(**mData).arg += 2;
		
	if ( (**mData).arg >= nshc_parms->argc ) {
		match_good( nshc_parms, mData );
		return;
		}
	
	// =====> convert path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**mData).arg, &fss );

	(**mData).arg++;
	
	if (result) {
		match_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir)
		return;
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pmatch: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**mData).got_fss, &fss, fsRdPerm, &(**mData).fref);
	
	if ( result ) {
		match_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		return;
		}
}

/* ======================================== */

void match_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData )
{
	char	c;
	int		close;
	int		i,j;
	int		temp;
	int		result;
	long	bcount;
	char	buf[MATCH_BUF_SIZE+1];
	char	*filename;
	
	result = close = 0;
	
	// =====> copy the data fork
	
	bcount = MATCH_BUF_SIZE;
	result = FSRead( (**mData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			close = 1;
			result = 0;
			}
		
		if (bcount) {
			j = (**mData).line_pos;
			for ( i = 0 ; i < bcount ; i++ ) {
				c = buf[i];
				if ( c && ( c != '\r' ) && ( j < 255 ) )
					(**mData).line[++j] = c;
				else {
					(**mData).line[0] = j;
					HLock( mData );
					if ( !nshc_calls->NSH_match( (**mData).pattern, (**mData).line ) ) {
						filename = &nshc_parms->arg_buf[nshc_parms->argv[(**mData).arg-1]];
						nshc_calls->NSH_puts( filename );
						nshc_calls->NSH_putStr( "\p: " );
						nshc_calls->NSH_putStr( (**mData).line );
						nshc_calls->NSH_putchar( '\r' );
						(**mData).found = 1;
						}
					HUnlock( mData );
					j = 0;
					}
				}
			(**mData).line_pos = j;
			}
	
		}
	else
		match_bad_file( nshc_parms, nshc_calls, "\pread data" );
	
	// =====> close the input file
	
	if (close) {

		if ( (**mData).fref ) {
			if ( temp = FSClose( (**mData).fref ) )
				result = temp;
			(**mData).fref = 0;
			}
			
		if (result)
			match_bad_file( nshc_parms, nshc_calls, "\pclose file" );
				
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

void match_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_match_hndl mData )
{
	long	bcount;
	Str255	buf;
	char	c;
	int		i;
	int		temp;
	
	bcount = nshc_calls->NSH_getStr( buf );
	
	if (!bcount) return;			// wait for characters
	
	if ( !nshc_calls->NSH_match( (**mData).pattern, buf ) ) {
		nshc_calls->NSH_putStr( buf );	// display the line
		(**mData).found = 1;
		}
	
	if ( bcount == -1 )				// end of input, set-up for exit
		match_good( nshc_parms, mData );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void match_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	int				arg;
	char 			*p;
	t_match_hndl	mData;	// handle to hold our data
	
	nshc_parms->action = nsh_continue;

	mData = (t_match_hndl )NewHandleClear(sizeof(t_match_data));
	
	if (mData) {
		nshc_parms->data = (Handle)mData;			// save the handle to our data
		(**mData).arg = 1;							// start at the arg = 1 position
		(**mData).by_file = nshc_parms->argc > 3;	// if we have other args, do file i/o
		(**mData).got_fss = fss_test();				// test if we can use FSSpec calls

		arg = nshc_got_option( nshc_parms, 'p' );	// get the position of '-p' arg
		(**mData).p_arg = arg;					// and save it
		
		if ( arg && ( ++arg < nshc_parms->argc ) ) {
			p = &nshc_parms->arg_buf[nshc_parms->argv[arg]];
			if ( cStrLen( p ) < 253 ) {
				(**mData).pattern[0] = 1;
				(**mData).pattern[1] = '*';
				pStrAppendC( (**mData).pattern, p );
				pStrAppendC( (**mData).pattern, "*" );
				}
			else {
				nshc_calls->NSH_putStr_err( "\pmatch: Pattern is too long.\r" );
				match_bad( nshc_parms, NSHC_ERR_PARMS );
				}
			}
		else {
			nshc_calls->NSH_putStr_err( "\pUsage: match -p pattern [files...]\r" );
			match_bad( nshc_parms, NSHC_ERR_PARMS );
			}
		
		}
	else {
		nshc_calls->NSH_putStr_err( "\pmatch: Could not allocate storage.\r" );
		match_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void match_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_match_hndl 	mData;

	if (mData = (t_match_hndl )nshc_parms->data) {

		if ( (**mData).by_file ) {
		
			// if we are not in the middle of a file, open one

			if ( !(**mData).fref )
				match_open( nshc_parms, nshc_calls, mData );

			// read and write some of the open file

			if ( (**mData).fref )
				match_read_write( nshc_parms, nshc_calls, mData );

			// and see if we are done

			if ( !(**mData).fref && ( (**mData).arg >= nshc_parms->argc ) )
				match_good( nshc_parms, mData );
				
			}
		else
			match_console( nshc_parms, nshc_calls, mData );

		}
}

/* ======================================== */

void match_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short			fRef;
	t_match_hndl 	mData;
	int				temp;
	int				result;
	
	result = 0;
	
	if (mData = (t_match_hndl )nshc_parms->data) {
	
		if ( (**mData).fref ) {
			if ( temp = FSClose( (**mData).fref ) )
				result = temp;
			(**mData).fref = 0;
			}
			
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		match_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
	nshc_parms->action = nsh_idle;
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
	
		switch (nshc_parms->action) {
			case nsh_start:
				match_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				match_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				match_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
