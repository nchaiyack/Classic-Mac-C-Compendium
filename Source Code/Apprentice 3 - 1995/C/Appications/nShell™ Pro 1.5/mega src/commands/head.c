/* ========== the commmand file: ==========

	head.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#define	HEAD_BUF_SIZE	2048

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	short	got_fss;		// 0 if FSSpec calls are not available
	short	arg;			// position in arg list
	short	n_arg;			// non-zero if an -n option was used
	short	by_file;		// 1 if files are listed, 0 if stdin
	short	fref;			// file ref. number, 0 if no file open
	long	lines;			// the line count for the current file
	long	lines_max;		// the line limit

} t_head_data;

typedef	t_head_data	**HeadHndl;

/* ======================================== */

// prototypes - utility

void head_bad( t_nshc_parms *nshc_parms, int code );
void head_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void head_good( t_nshc_parms *nshc_parms );

// prototypes - file copy routines

void head_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData );
void head_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData );

// prototypes - console copy routines

void head_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData );

// prototypes - state machine

void head_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void head_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void head_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void head_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void head_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\phead: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_FILE;
}

void head_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void head_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	// by definition, there is no open file, and it has no lines
	
	(**hData).fref = 0;
	(**hData).lines = 0;

	// skip the -n option
	
	if ( (**hData).arg == (**hData).n_arg )
		(**hData).arg += 2;
		
	if ( (**hData).arg >= nshc_parms->argc ) {
		head_good( nshc_parms );
		return;
		}
	
	// =====> convert path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fss );

	(**hData).arg++;
	
	if (result) {
		head_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir)
		return;
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\phead: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**hData).got_fss, &fss, fsRdPerm, &(**hData).fref);
	
	if ( result ) {
		head_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		return;
		}
}

/* ======================================== */

void head_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData )
{
	int		close;
	int		temp;
	int		result;
	long	bcount;
	char	buf[HEAD_BUF_SIZE+1];
	char	c;
	char	*p;
	
	result = close = 0;
	
	// =====> copy the data fork
	
	bcount = HEAD_BUF_SIZE;
	result = FSRead( (**hData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			close = 1;
			result = 0;
			}
		
		if (bcount) {
		
			buf[bcount] = '\0';					// make sure there is termination
			
			p = buf;
					
			while ( c = *p++ ) {				// adjust line count and limit string
				if ( c == '\r' )
					(**hData).lines++;
				if ( (**hData).lines >= (**hData).lines_max ) {
					*p = 0;
					close = 1;
					break;
					}
				}
				
			nshc_calls->NSH_puts( buf );	// display the line
				
			}
		
		}
	else
		head_bad_file( nshc_parms, nshc_calls, "\pread data" );
	
	// =====> close the input file
	
	if (close) {

		if ( (**hData).fref ) {
			if ( temp = FSClose( (**hData).fref ) )
				result = temp;
			(**hData).fref = 0;
			}
			
		if (result)
			head_bad_file( nshc_parms, nshc_calls, "\pclose file" );
				
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

#define	LINES_PER_PASS	10

void head_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_head_data **hData )
{
	long	bcount;
	char	buf[LINE_MAX];
	char	c;
	char	*p;
	int		pass;
	
	// if characters are waiting, pick up a few lines
		
	pass = 1;
	
	while ( pass++ < LINES_PER_PASS ) {
	
		bcount = nshc_calls->NSH_gets( buf, LINE_MAX );
		
		if (!bcount) return;				// go get more characters
				
		if ( (**hData).lines < (**hData).lines_max ) {

			p = buf;
				
			while ( c = *p++ )				// adjust line count
				if ( c == '\r' )
					(**hData).lines++;
			
			nshc_calls->NSH_puts( buf );	// display the line
			
			}
					
		if ( bcount == -1 )	 {				// end of input, set-up for exit
			head_good( nshc_parms );
			return;
			}
			
		}
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void head_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	HeadHndl	hData;	// handle to hold our data
	int			usage;  // if 1, usage error
	
	nshc_parms->action = nsh_continue;

	hData = (HeadHndl)NewHandleClear(sizeof(t_head_data));
	
	if (hData) {
	
		(**hData).arg = 1;							// start at the arg = 1 position
		(**hData).got_fss = fss_test();				// test if we can use FSSpec calls
		(**hData).lines_max = 10;					// do ten lines as default
		
		usage = 0;
		
		if ( (**hData).n_arg = nshc_got_option( nshc_parms, 'n' ) ) {
		
			usage = 1;
		
			if ( nshc_is_numeric_operand( nshc_parms, (**hData).n_arg + 1 ) ) {
				(**hData).lines_max = arg_to_num( nshc_parms, (**hData).n_arg + 1 );
				(**hData).by_file = nshc_parms->argc > 3;	// if we have other args, do file i/o
				usage = 0;
				}
				
			}
		else
			(**hData).by_file = nshc_parms->argc > 1;	// if we have other args, do file i/o
		
		if ( usage ) {
			nshc_calls->NSH_putStr_err( "\pUsage: head [-n lines] [files...]\r" );
			head_bad( nshc_parms, NSHC_ERR_MEMORY );
			}
		
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\phead: Could not allocate storage.\r" );
		head_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void head_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	HeadHndl	hData;

	if (hData = (HeadHndl)nshc_parms->data) {

		if ( (**hData).by_file ) {
		
			// if we are not in the middle of a file, open one

			if ( !(**hData).fref )
				head_open( nshc_parms, nshc_calls, hData );

			// read and write some of the open file

			if ( (**hData).fref )
				head_read_write( nshc_parms, nshc_calls, hData );

			// and see if we are done

			if ( !(**hData).fref && ( (**hData).arg >= nshc_parms->argc ) )
				head_good( nshc_parms );
				
			}
		else
			head_console( nshc_parms, nshc_calls, hData );

		}
}

/* ======================================== */

void head_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short		fRef;
	HeadHndl	hData;
	int			temp;
	int			result;
	
	result = 0;
	
	if (hData = (HeadHndl)nshc_parms->data) {
	
		if ( (**hData).fref ) {
			if ( temp = FSClose( (**hData).fref ) )
				result = temp;
			(**hData).fref = 0;
			}
			
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		head_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
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
				head_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				head_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				head_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
