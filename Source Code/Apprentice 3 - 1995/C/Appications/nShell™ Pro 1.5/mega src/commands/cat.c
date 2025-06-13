/* ========== the commmand file: ==========

	cat.c
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#define	CAT_BUF_SIZE	2048

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

typedef enum { copy_none, copy_data, copy_rsrc } c_state;

typedef struct {

	int		by_file;		// 1 if files are listed, 0 if stdin

	int		got_fss;		// 0 if FSSpec calls are not available

	int		arg;			// position in arg list
	
	short	fref;			// file ref. number, 0 if no file open

} t_cat_data;

typedef	t_cat_data	**CDataH;

/* ======================================== */

// prototypes - utility

void cat_bad( t_nshc_parms *nshc_parms, int code );
void cat_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void cat_good( t_nshc_parms *nshc_parms );

// prototypes - file copy routines

void cat_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cat_data **hData );
void cat_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cat_data **hData );

// prototypes - console copy routines

void cat_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

// prototypes - state machine

void cat_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void cat_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void cat_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void cat_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void cat_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\pcat: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}

void cat_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void cat_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cat_data **hData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	(**hData).fref = 0;

	// =====> convert path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fss );

	(**hData).arg++;
	
	if (result) {
		cat_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir) {
		nshc_calls->NSH_putStr_err("\pcat: Skiping directory = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\pcat: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**hData).got_fss, &fss, fsRdPerm, &(**hData).fref);
	
	if ( result ) {
		cat_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		return;
		}
}

/* ======================================== */

void cat_read_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_cat_data **hData )
{
	int		close;
	int		temp;
	int		result;
	long	bcount;
	char	buf[CAT_BUF_SIZE+1];
	
	result = close = 0;
	
	// =====> copy the data fork
	
	bcount = CAT_BUF_SIZE;
	result = FSRead( (**hData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			close = 1;
			result = 0;
			}
		
		if (bcount) {
			buf[bcount] = '\0';
			nshc_calls->NSH_puts( buf );
			}
		
		}
	else
		cat_bad_file( nshc_parms, nshc_calls, "\pread data" );
	
	// =====> close the input file
	
	if (close) {

		if ( (**hData).fref ) {
			if ( temp = FSClose( (**hData).fref ) )
				result = temp;
			(**hData).fref = 0;
			}
			
		if (result)
			cat_bad_file( nshc_parms, nshc_calls, "\pclose file" );
				
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

#define	LINES_PER_PASS	10

void cat_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	long	bcount;
	char	buf[LINE_MAX];
	char	c;
	int		i;
	int		temp;
	int		pass;
	
	// if characters are waiting, pick up a few lines
		
	pass = 1;
	
	while ( pass++ < LINES_PER_PASS ) {
	
		bcount = nshc_calls->NSH_gets( buf, LINE_MAX );
		
		if (!bcount) return;			// go get more characters
		
		nshc_calls->NSH_puts( buf );	// display the line
		
		if ( bcount == -1 )	 {			// end of input, set-up for exit
			cat_good( nshc_parms );
			return;
			}
			
		}
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void cat_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	CDataH	hData;	// handle to hold our data
	
	nshc_parms->action = nsh_continue;

	hData = (CDataH)NewHandleClear(sizeof(t_cat_data));
	
	if (hData) {
		(**hData).by_file = nshc_parms->argc > 1;	// if we have args, do file i/o
		(**hData).arg = 1;							// start at the arg = 1 position
		(**hData).got_fss = fss_test();				// test if we can use FSSpec calls
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\pcat: Could not allocate storage.\r" );
		cat_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void cat_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	CDataH	hData;

	if (hData = (CDataH)nshc_parms->data) {

		if ( (**hData).by_file ) {
		
			// if we are not in the middle of a file, open one

			if ( !(**hData).fref )
				cat_open( nshc_parms, nshc_calls, hData );

			// read and write some of the open file

			if ( (**hData).fref )
				cat_read_write( nshc_parms, nshc_calls, hData );

			// and see if we are done

			if ( !(**hData).fref && ( (**hData).arg >= nshc_parms->argc ) )
				cat_good( nshc_parms );
				
			}
		else
			cat_console( nshc_parms, nshc_calls );

		}
}

/* ======================================== */

void cat_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short	fRef;
	CDataH	hData;
	int		temp;
	int		result;
	
	result = 0;
	
	if (hData = (CDataH)nshc_parms->data) {
	
		if ( (**hData).fref ) {
			if ( temp = FSClose( (**hData).fref ) )
				result = temp;
			(**hData).fref = 0;
			}
			
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		cat_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
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
				cat_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				cat_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				cat_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
