/* ========== the commmand file: ==========

	grep.c
	
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

#include <script.h>
#include <string.h>

#include "nshc.h"
#include "regexp.h"
#include "regexp.proto.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		by_file;			// 1 if files are listed, 0 if stdin
	int		got_fss;			// 1 if FSSpec calls are available
	int		arg;				// next file to process in arg list
	int		found;				// next file to process in arg list
	short	e_arg;				// position of '-p' in arg list
	short	fref;				// file ref. number, 0 if no file open
	char	pattern[LINE_MAX];	// the pattern for which we search
	char	line[LINE_MAX];		// the line we are currently looking at
	int		line_pos;			// the place to put the next char in 'line'

} t_grep_data;

typedef	t_grep_data	**t_grep_handle;

/* ======================================== */

// globals

t_nshc_calls	*g_nshc_calls;		// global copy of nshc_calls to reduce parm passing

/* ======================================== */

// prototypes - utility

int  grep( char *pattern, char *target );
void grep_bad( t_nshc_parms *nshc_parms, int code );
void grep_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void grep_good(  t_nshc_parms *nshc_parms, t_grep_handle gData );

// prototypes - file copy routines

void grep_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData );
void grep_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData );

// prototypes - console copy routines

void grep_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData );

// prototypes - state machine

void grep_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void grep_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void grep_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void regerror(char *s)
{
	g_nshc_calls->NSH_puts_err( s );
}

void grep_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void grep_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pgrep: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void grep_good(  t_nshc_parms *nshc_parms, t_grep_handle gData )
{
	nshc_parms->action = nsh_stop;
	
	if ( (**gData).found )
		nshc_parms->result = 0;
	else
		nshc_parms->result = 1;
}

/* ======================================== */

// file access routines

/* ======================================== */

void grep_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	(**gData).fref = 0;
	(**gData).line_pos = 0;

	// =====> skip the -p option
	
	if ( (**gData).arg == (**gData).e_arg )
		(**gData).arg += 2;
		
	if ( (**gData).arg >= nshc_parms->argc ) {
		grep_good( nshc_parms, gData );
		return;
		}
	
	// =====> convert path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**gData).arg, &fss );

	(**gData).arg++;
	
	if (result) {
		grep_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir)
		return;
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pgrep: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**gData).got_fss, &fss, fsRdPerm, &(**gData).fref);
	
	if ( result ) {
		grep_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		return;
		}
}

/* ======================================== */

void grep_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData )
{
	char	c;
	int		close;
	int		i,j;
	int		temp;
	int		result;
	long	bcount;
	char	buf[LINE_MAX];
	char	*filename;
	
	result = close = 0;
	
	// =====> copy the data fork
	
	bcount = LINE_MAX - 1;
	result = FSRead( (**gData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			close = 1;
			result = 0;
			}
		
		if (bcount) {
			j = (**gData).line_pos;
			for ( i = 0 ; i < bcount ; i++ ) {
				c = buf[i];
				if ( c && ( c != '\r' ) && ( j < 255 ) )
					(**gData).line[j++] = c;
				else {
					(**gData).line[j] = 0;
					HLock( gData );
					if ( grep( (**gData).pattern, (**gData).line ) ) {
						filename = &nshc_parms->arg_buf[nshc_parms->argv[(**gData).arg-1]];
						nshc_calls->NSH_puts( filename );
						nshc_calls->NSH_putStr( "\p: " );
						nshc_calls->NSH_puts( (**gData).line );
						nshc_calls->NSH_putchar( '\r' );
						(**gData).found = 1;
						}
					HUnlock( gData );
					j = 0;
					}
				}
			(**gData).line_pos = j;
			}
	
		}
	else
		grep_bad_file( nshc_parms, nshc_calls, "\pread data" );
	
	// =====> close the input file
	
	if (close) {

		if ( (**gData).fref ) {
			if ( temp = FSClose( (**gData).fref ) )
				result = temp;
			(**gData).fref = 0;
			}
			
		if (result)
			grep_bad_file( nshc_parms, nshc_calls, "\pclose file" );
				
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

void grep_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_grep_data **gData )
{
	long	bcount;
	char	buf[LINE_MAX];
	
	bcount = nshc_calls->NSH_gets( buf, LINE_MAX );
	
	if (!bcount) return;			// wait for characters
	
	if ( grep( (**gData).pattern, buf ) ) {
		nshc_calls->NSH_puts( buf );
		(**gData).found = 1;
		}

	if ( bcount == -1 )				// end of input, set-up for exit
		grep_good( nshc_parms, gData );
}

/* ======================================== */

int grep( char *pattern, char *target )
{
	regexp	*r;
	int		result;
	
	result = -1;
	
	r = regcomp( pattern );
	
	if ( r ) {
		result = regexec( r, target );
		DisposPtr( r );
		}
		
	return( result );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void grep_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	int				arg;
	char 			*p;
	t_grep_handle	gData;	// handle to hold our data
	
	nshc_parms->action = nsh_continue;

	gData = (t_grep_handle)NewHandleClear(sizeof(t_grep_data));
	
	if (gData) {
	
		nshc_parms->data = (Handle)gData;			// save the handle to our data
		(**gData).got_fss = fss_test();				// test if we can use FSSpec calls

		arg = nshc_got_option( nshc_parms, 'e' );	// get the position of '-e' arg
		
		// figure out if we are by_file, based on presence of -e option

		if (arg) {
			(**gData).by_file = nshc_parms->argc > 3;
			(**gData).arg = 1;
			}
		else {
			(**gData).by_file = nshc_parms->argc > 2;
			(**gData).arg = 2;
			}
			
		(**gData).e_arg = arg;						// save position of pattern
		
		if ( ++arg < nshc_parms->argc )
			strcpy( (**gData).pattern, &nshc_parms->arg_buf[nshc_parms->argv[arg]] );
		else {
			nshc_calls->NSH_putStr_err( "\pUsage: grep [-e] pattern [files...]\r" );
			grep_bad( nshc_parms, NSHC_ERR_PARMS );
			}
		
		}
	else {
		nshc_calls->NSH_putStr_err( "\pgrep: Could not allocate storage.\r" );
		grep_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void grep_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	t_grep_handle	gData;

	if (gData = (t_grep_handle)nshc_parms->data) {

		if ( (**gData).by_file ) {
		
			// if we are not in the middle of a file, open one

			if ( !(**gData).fref )
				grep_open( nshc_parms, nshc_calls, gData );

			// read and write some of the open file

			if ( (**gData).fref )
				grep_read_write( nshc_parms, nshc_calls, gData );

			// and see if we are done

			if ( !(**gData).fref && ( (**gData).arg >= nshc_parms->argc ) )
				grep_good( nshc_parms, gData );
				
			}
		else
			grep_console( nshc_parms, nshc_calls, gData );

		}
}

/* ======================================== */

void grep_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short			fRef;
	t_grep_handle	gData;
	int				temp;
	int				result;
	
	result = 0;
	
	if (gData = (t_grep_handle)nshc_parms->data) {
	
		if ( (**gData).fref ) {
			if ( temp = FSClose( (**gData).fref ) )
				result = temp;
			(**gData).fref = 0;
			}
			
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		grep_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
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

	g_nshc_calls = nshc_calls;
	
	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
	
		switch (nshc_parms->action) {
			case nsh_start:
				grep_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				grep_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				grep_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}

/* ======================================== */
