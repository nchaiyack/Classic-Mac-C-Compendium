/* ========== the commmand file: ==========

	tail.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#define	TAIL_BUF_SIZE	2048

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "fss_utl.proto.h"
#include "nshc_utl.proto.h"
#include "str_utl.proto.h"

/* ======================================== */

// typedefs

// The main data structure, the one that becomes nshc_parms->data

typedef enum { file_none, file_open, file_read, file_write, file_close } f_state;

typedef struct {

	short	arg;			// position in arg list
	short	n_arg;			// non-zero if an -n option was used
	short	by_file;		// 1 if files are listed, 0 if stdin
	
	long	lines;			// the line count for the current file
	long	lines_max;		// the line limit
	
	short	fref;			// file ref. number, 0 if no file open (when by file)
	long	fpos;			// position in the current file (when by file)
	f_state	fstate;			// a state machine control used when searching files
	
	Handle	root;			// start of linked list of strings (used with console)
	Handle	last;			// end of linked list (used with console)

	short	got_fss;		// 0 if FSSpec calls are not available

} t_tail_data;

typedef	t_tail_data	**TailHndl;

// This structure holds strings typed on standard input until they are echoed

typedef struct {

		Handle	next;
		char	string[];
		
} t_string_rec;

typedef t_string_rec **t_string_hndl;

// This structure holds string positions from files until they are echoed

typedef struct {

		Handle	next;
		long	position;
		
} t_position_rec;

typedef t_position_rec **t_position_hndl;

/* ======================================== */

// prototypes - utility

void tail_bad( t_nshc_parms *nshc_parms, int code );
void tail_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void tail_good( t_nshc_parms *nshc_parms );

// prototypes - file copy routines

void  tail_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );
void  tail_read( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );
void  tail_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );
void  tail_close( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );
void  tail_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );

// prototypes - console copy routines

void  tail_copy( char *q, char *p );
OSErr tail_string_add( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData, char *buf );
void  tail_string_print( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );
void  tail_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData );

// prototypes - state machine

void tail_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void tail_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void tail_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void tail_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void tail_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{
	nshc_calls->NSH_putStr_err("\ptail: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_FILE;
}

void tail_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void tail_open( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	int		result;
	long	dirID;
	Boolean	isDir;
	FSSpec	fss;
	
	// by definition, there is no open file, and it has no lines
	
	(**hData).fref = 0;
	(**hData).lines = 0;
	(**hData).fpos = 0;

	// skip the -n option
	
	if ( (**hData).arg == (**hData).n_arg )
		(**hData).arg += 2;
		
	if ( (**hData).arg >= nshc_parms->argc ) {
		tail_good( nshc_parms );
		return;
		}
	
	// =====> convert path to fsspec
	
	result = arg_to_fss( nshc_parms, nshc_calls, (**hData).arg, &fss );

	(**hData).arg++;
	
	if (result) {
		tail_bad( nshc_parms, result );
		return;
		}
	
	result = fss_to_DirID( &fss, &dirID, &isDir );
		
	if (( result == noErr) && isDir)
		return;
			
	if ( result == fnfErr ) {
		nshc_calls->NSH_putStr_err("\ptail: File not found = ");
		nshc_calls->NSH_putStr_err((StringPtr)fss.name);
		nshc_calls->NSH_putchar('\r');
		return;
		}
			
	if (!result)
		result = fss_OpenDF((**hData).got_fss, &fss, fsRdPerm, &(**hData).fref);
	
	if (!result) {
		HLock( hData );
		result = GetEOF((**hData).fref, &(**hData).fpos );
		HUnlock( hData );
		}
		
	if ( result )
		tail_bad_file( nshc_parms, nshc_calls, (StringPtr)fss.name );
		
	(**hData).fstate = file_read;
}

/* ======================================== */

void tail_read( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	int		i;
	int		result;
	long	bcount;
	char	c;
	char	buf[TAIL_BUF_SIZE+1];
	
	result = 0;
	
	// =====> determine and set file position
	
	bcount = TAIL_BUF_SIZE;
	(**hData).fpos -= TAIL_BUF_SIZE;
	
	if ( (**hData).fpos < 0 ) {		// adjust buf size if we hit the
		bcount += (**hData).fpos;	// beginning of the file
		(**hData).fpos = 0;
		}
	
	result = SetFPos( (**hData).fref, fsFromStart, (**hData).fpos );
	
	if (result) {
		tail_bad_file( nshc_parms, nshc_calls, "\pset position" );
		return;
		}

	// =====> read the data fork
	
	result = FSRead( (**hData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (bcount) {
		
			buf[bcount] = '\0';					// make sure there is termination
			
			i = bcount - 1;
			
			while ( i >= 0 ) {					// search backwards for n lines
				c = buf[i];
				if ( c == '\r' )
					(**hData).lines++;
				if ( (**hData).lines > (**hData).lines_max ) {	// print if found
					nshc_calls->NSH_puts( &buf[i+1] );
					(**hData).fstate = file_write;
					break;
					}
				if ( !(**hData).fpos && !i) {					// print if beginning
					nshc_calls->NSH_puts( buf );				// of file is hit
					(**hData).fstate = file_write;				// before n lines
					break;										// are found
					}
				i--;
				}

			}
			
		if (result == eofErr) {
			(**hData).fstate = file_write;
			result = 0;
			}
				
		}
	else
		tail_bad_file( nshc_parms, nshc_calls, "\pread data" );
	
}


/* ======================================== */

void tail_write( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	int		result;
	long	bcount;
	char	buf[TAIL_BUF_SIZE+1];
	
	result = 0;
	
	bcount = TAIL_BUF_SIZE;
	result = FSRead( (**hData).fref, &bcount, &buf );
	
	if (( result == noErr ) || ( result == eofErr )) {
	
		if (result == eofErr) {
			(**hData).fstate = file_close;
			result = 0;
			}
		
		if (bcount) {
			buf[bcount] = '\0';
			nshc_calls->NSH_puts( buf );
			}
		
		}
	else
		tail_bad_file( nshc_parms, nshc_calls, "\pwrite data" );
	
}

/* ======================================== */

void tail_close( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	OSErr	result;
	
	result = 0;
	
	if ( (**hData).fref ) {
		result = FSClose( (**hData).fref );
		(**hData).fref = 0;
		}
		
	if (result)
		tail_bad_file( nshc_parms, nshc_calls, "\pclose file" );	
		
	(**hData).fstate = file_open;		// start the next file, if any	
}

/* ======================================== */

void tail_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	switch ((**hData).fstate) {
		case file_open:
			tail_open( nshc_parms, nshc_calls, hData );
			break;
		case file_read:
			tail_read( nshc_parms, nshc_calls, hData );
			break;
		case file_write:
			tail_write( nshc_parms, nshc_calls, hData );
			break;
		case file_close:
			tail_close( nshc_parms, nshc_calls, hData );
			break;
		}
}

/* ======================================== */

// console access routines

/* ======================================== */

// list management routines

void tail_copy( char *q, char *p )
{
	char c;
	
	while ( c = *p++ )
		*q++ = c;
		
	*q = 0;
}

OSErr tail_string_add( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData, char *buf )
{
	short	length;
	Handle	new_element;
	Handle	old_element;
	
	length = 8 + cStrLen( buf );	// room for handle plus some padding
	
	new_element = NewHandleClear( length );
	
	if ( !new_element )
		return( NSHC_ERR_MEMORY );
		
	HLock( new_element );
	tail_copy( (**(t_string_hndl)new_element).string, buf );
	HUnlock( new_element );
	
	(**(t_string_hndl)(**hData).last).next = new_element;
	(**hData).last = new_element;

	if ( !(**hData).root )
		(**hData).root = new_element;
		
	if ( (**hData).lines > (**hData).lines_max ) {
		old_element = (**(t_string_hndl)(**hData).root).next;
		DisposeHandle( (**hData).root );
		(**hData).root = old_element;
		}

	(**hData).lines++;
	
	return( NSHC_NO_ERR );
}

/* ======================================== */

void tail_string_print( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
{
	Handle	this;
	Handle	next;
	
	this = (**hData).root;		// detach list from data block
	(**hData).root = nil;
		
	while ( this ) {			// print and delete each element
		HLock( this );
		nshc_calls->NSH_puts( (**(t_string_hndl)this).string );
		HUnlock( this );
		next = (**(t_string_hndl)this).next;
		DisposeHandle( this );
		this = next;
		}
}

/* ======================================== */

#define	LINES_PER_PASS	10

void tail_console( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_tail_data **hData )
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
		
		if ( tail_string_add( nshc_parms, nshc_calls, hData, buf ) ) {
			nshc_calls->NSH_putStr_err( "\ptail: Could not allocate enough string storage.\r" );
			tail_bad( nshc_parms, NSHC_ERR_MEMORY );
			return;
			}
			
		if ( bcount == -1 ) {				// end of input, set-up for exit
			tail_string_print( nshc_parms, nshc_calls, hData );
			tail_good( nshc_parms );
			return;
			}
			
		}
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void tail_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	TailHndl	hData;	// handle to hold our data
	int			usage;  // if 1, usage error
	
	nshc_parms->action = nsh_continue;

	hData = (TailHndl)NewHandleClear(sizeof(t_tail_data));
	
	if (hData) {
	
		(**hData).arg = 1;							// start at the arg = 1 position
		(**hData).got_fss = fss_test();				// test if we can use FSSpec calls
		(**hData).lines_max = 10;					// do ten lines as default
		(**hData).fstate = file_open;				// if we do files, start with open
			
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
			nshc_calls->NSH_putStr_err( "\pUsage: tail [-n lines] [files...]\r" );
			tail_bad( nshc_parms, NSHC_ERR_MEMORY );
			}
		
		if ( (**hData).lines_max < 1 ) {
			nshc_calls->NSH_putStr_err( "\ptail: Line count must be greater than one.\r" );
			tail_bad( nshc_parms, NSHC_ERR_MEMORY );
			}
		
		nshc_parms->data = (Handle)hData;
		}
	else {
		nshc_calls->NSH_putStr_err( "\ptail: Could not allocate storage.\r" );
		tail_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void tail_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	TailHndl	hData;

	if (hData = (TailHndl)nshc_parms->data) {
	
		if ( (**hData).by_file )
			tail_file( nshc_parms, nshc_calls, hData );
		else
			tail_console( nshc_parms, nshc_calls, hData );
		
		}
}

/* ======================================== */

void tail_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	short		fRef;
	TailHndl	hData;
	int			temp;
	int			result;
	Handle		this;
	Handle		next;
	
	result = 0;
	
	if (hData = (TailHndl)nshc_parms->data) {
	
		this = (**hData).root;						// detach list from data block
		(**hData).root = nil;
		
		while ( this ) {							// delete each element
			next = (**(t_string_hndl)this).next;
			DisposeHandle( this );
			this = next;
			}
	
		if ( (**hData).fref ) {
			if ( temp = FSClose( (**hData).fref ) )
				result = temp;
			(**hData).fref = 0;
			}
			
		DisposeHandle(nshc_parms->data);
		}
		
	if (result)
		tail_bad_file( nshc_parms, nshc_calls, "\pclosing files" );
		
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
				tail_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				tail_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				tail_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
