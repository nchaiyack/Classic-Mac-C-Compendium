/* ========== the commmand file: ==========

	wc.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#define	FILE_BUF_SIZE		2048

#define	CONSOLE_BUF_SIZE	200

#include "nshc.h"
#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// our data record, NewHandled and attached to nshc_parms->data

typedef struct {

	int		by_file;		// 1 if files are listed, 0 if stdin
	
	int		got_fss;		// 0 if FSSpec calls are not available
	int		arg;			// position in arg list (if by file)
	short	fref;			// file ref. number, 0 if no file open
	int		fcount;			// number of files processed
	int		inword;			// 1 if we are in the middle of a word
	
	int		got_options;	// 1 if any option was given
	int		line_option;	// show-line option was given
	int		word_option;	// show-word option was given
	int		bytes_option;	// show-bytes option was given

	long	lines;			// number of carridge returns (seen a carridge lately?)
	long	lines_total;	// total number of carridge returns for all files

	long	words;			// number of "words" (actually things between whitespace)
	long	words_total;	// total number of words for all files

	long	bytes;			// number of bytes
	long	bytes_total;	// total number of bytes for all files
	
} t_wc_data;

typedef t_wc_data **t_wc_handle;

/* ======================================== */

// prototypes - utility

void wc_bad( t_nshc_parms *nshc_parms, int code );
void wc_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
int  wc_by_file( t_nshc_parms *nshc_parms );
void wc_good( t_nshc_parms *nshc_parms );
int	 wc_whitespace( char c );

// prototypes - print routines

void wc_print_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData, int arg );
void wc_print_total( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData );

// prototypes - file access routines

void wc_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData );
void wc_process_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData );

// console access routines

void wc_process_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData );

// prototypes - state machine

void wc_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void wc_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void wc_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility

int	wc_by_file( t_nshc_parms *nshc_parms )
{

	// if we have any operands, then we count by file
	
	int	arg;
	
	arg = 1;
	
	while ( arg < nshc_parms->argc ) {
		if ( nshc_is_operand( nshc_parms, arg ) )
			return( 1 );
		arg++;
		}
		
	return( 0 );
}

void wc_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void wc_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pwc: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void wc_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

int	wc_whitespace( char c )
{
	switch (c) {
		case ' ':
		case '\t':
		case '\r':
			return(1);
		default:
			return(0);
		}
}

/* ======================================== */

// print routines

/* ======================================== */

void wc_print_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData, int arg )
{
	int	all;
	
	all = !(**ourData).got_options;
	
	if (all || (**ourData).line_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).lines );
	
	if (all || (**ourData).word_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).words );
	
	if (all || (**ourData).bytes_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).bytes );

	if (arg)
		nshc_calls->NSH_puts( &nshc_parms->arg_buf[nshc_parms->argv[arg]] );
		
	nshc_calls->NSH_putchar('\r');
}

void wc_print_total( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData )
{
	int	all;
	
	all = !(**ourData).got_options;

	if (all || (**ourData).line_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).lines_total );
	
	if (all || (**ourData).word_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).words_total );
	
	if (all || (**ourData).bytes_option )
		nshc_calls->NSH_printf( "%7ld ", (**ourData).bytes_total );
		
	nshc_calls->NSH_putStr("\ptotal\r");
}

/* ======================================== */

// file access routines

/* ======================================== */

void wc_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	(**ourData).fref = 0;

	// a good time to initialize counts, etc.
		
	(**ourData).lines = 0;
	(**ourData).words = 0;
	(**ourData).bytes = 0;
	(**ourData).inword = 0;

	// =====> convert path to fsspec
	
	if ( nshc_is_operand( nshc_parms, (**ourData).arg ) ) {
		result = arg_to_fss( nshc_parms, nshc_calls, (**ourData).arg, &fss );
		(**ourData).arg++;
		}
	else {
		(**ourData).arg++;
		return;
		}
	
	if (result) {
		wc_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir) {
		nshc_calls->NSH_putStr_err("\pwc: Skiping directory = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pwc: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**ourData).got_fss, &fss, fsRdPerm, &(**ourData).fref);
	
	if ( result ) {
		wc_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		return;
		}
}

/* ======================================== */

void wc_process_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData )
{
	int		close;
	int		temp;
	int		result;
	long	bcount;
	char	buf[FILE_BUF_SIZE];
	char	c;
	int		i;
	
	result = close = 0;
	
	// =====> copy the data fork
		
	bcount = FILE_BUF_SIZE;
	result = FSRead( (**ourData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			close = 1;
			result = 0;
			}
		
		if (bcount && !result) {
		
			(**ourData).bytes += bcount;

			for (i=0 ; i<bcount; i++ ) {
				c = buf[i];
				if ( c == '\r' ) (**ourData).lines++;
				temp = wc_whitespace( c );
				if ( temp && (**ourData).inword )
					(**ourData).words++;
				(**ourData).inword = !temp;		
				}
				
			}
		
		}
	else
		wc_bad_file( nshc_parms, nshc_calls, "\pread resource" );
	
	// =====> close the input file
	
	if (close) {
	
		// close

		if ( (**ourData).fref ) {
			if ( temp = FSClose( (**ourData).fref ) )
				result = temp;
			(**ourData).fref = 0;
			}
			
		// if an error occured, bail.  otherwise compute & print

		if (result)
			wc_bad_file( nshc_parms, nshc_calls, "\pclose file" );
		else {
		
			if ( (**ourData).inword )	// if we finish in a word, it's still a word
				(**ourData).words++;
				
			(**ourData).fcount++;
			
			(**ourData).lines_total += (**ourData).lines;
			(**ourData).words_total += (**ourData).words;
			(**ourData).bytes_total += (**ourData).bytes;
			
			wc_print_one( nshc_parms, nshc_calls, ourData, (**ourData).arg - 1 );
		
			}
		
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

#define	LINES_PER_PASS	10

void wc_process_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_wc_data **ourData )
{
	long	bcount;
	char	buf[CONSOLE_BUF_SIZE];
	char	c;
	int		i;
	int		temp;
	int		pass;
	
	// if characters are waiting, pick up a few lines
		
	pass = 1;
	
	while ( pass++ < LINES_PER_PASS ) {
	
		bcount = nshc_calls->NSH_gets( buf, CONSOLE_BUF_SIZE );
		
		// if no characters are waiting, return to app
		
		if (!bcount) return;
		
		// otherwise, proccess the line
					
		i = 0;
		
		while ( c = buf[i++] ) {
		
			(**ourData).bytes++;
	
			if ( c == '\r' ) (**ourData).lines++;
			
			temp = wc_whitespace( c );
			if ( temp && (**ourData).inword )
				(**ourData).words++;
			(**ourData).inword = !temp;		
				
			}

		// end of input, display and exit

		if ( bcount == -1 ) {
		
			if ( (**ourData).inword )	// if we finish in a word, it's still a word
				(**ourData).words++;
			
			wc_print_one( nshc_parms, nshc_calls, ourData, 0 );
			wc_good( nshc_parms );
			
			return;
	
			}
	}
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void wc_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_wc_handle	ourData;

	nshc_parms->action = nsh_idle;
	nshc_parms->result = NSHC_NO_ERR;
	
	ourData = (t_wc_handle)NewHandleClear( sizeof(t_wc_data) );

	if (ourData) {

		// setup for by_file (or by stdin)
	
		(**ourData).by_file = wc_by_file( nshc_parms );
		
		(**ourData).arg = 1;

		// test if we can use FSSpec calls
				
		(**ourData).got_fss = fss_test();
		
		// record user options
		
		(**ourData).line_option = nshc_got_option( nshc_parms, 'l' );
		(**ourData).word_option = nshc_got_option( nshc_parms, 'w' );
		(**ourData).bytes_option = nshc_got_option( nshc_parms, 'c' );
		
		(**ourData).got_options = (**ourData).line_option ||
								  (**ourData).word_option ||
								  (**ourData).bytes_option;
								 
		nshc_parms->data = (Handle)ourData;
		nshc_parms->action = nsh_continue;
		}
	else {
		nshc_calls->NSH_puts("wc: Could not allocate storage.\r");
		nshc_parms->result = NSHC_ERR_MEMORY;
		}
}

/* ======================================== */

void wc_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_wc_handle	ourData;
	
	if (ourData = (t_wc_handle)nshc_parms->data) {

		if ( (**ourData).by_file ) {
			
			// if we are not in the middle of a file, open one

			if ( !(**ourData).fref )
				wc_open( nshc_parms, nshc_calls, ourData );
			
			// read and proccess some of the open file

			if ( (**ourData).fref )
				wc_process_file( nshc_parms, nshc_calls, ourData );
			
			// and see if we are done

			if ( !(**ourData).fref && ( (**ourData).arg >= nshc_parms->argc ) ) {
				wc_good( nshc_parms );
				return;
				}

			}
		else
			wc_process_console( nshc_parms, nshc_calls, ourData );

		}
}

/* ======================================== */

void wc_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{		
	short		fRef;
	t_wc_handle	ourData;
	int			temp;
	int			result;
	
	result = 0;
	
	if (ourData = (t_wc_handle)nshc_parms->data) {

		// these are unsuccessfull closes only - so no file totals are printed
	
		if ( (**ourData).fref ) {
			if ( temp = FSClose( (**ourData).fref ) )
				result = temp;
			(**ourData).fref = 0;
			}
			
		if (result)
			wc_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
			
		// if there have been no errors, print the grand total
		
		if ( !nshc_parms->result && ( (**ourData).fcount > 1 ) )
			wc_print_total( nshc_parms, nshc_calls, ourData );
			
		DisposeHandle(nshc_parms->data);
		}
		
		
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
				wc_start(nshc_parms,nshc_calls);
				break;
			case nsh_continue:
				wc_continue(nshc_parms,nshc_calls);
				break;
			default:
				wc_stop(nshc_parms,nshc_calls);
				break;
			}
			
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
