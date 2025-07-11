/* ==========================================

	script.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "pipe.h"
#include "script.h"

#include "arg_utl.proto.h"
#include "fss.proto.h"
#include "fss_utl.proto.h"
#include "interp.proto.h"
#include "pipe.proto.h"
#include "script.proto.h"
#include "str_utl.proto.h"
#include "vlist.proto.h"

extern		InterpH	i_interp;
extern		ShellH	i_shell;

extern		Boolean	gHasFSSpec;	// true if the calls are available

extern		Str255	gInFile;	// holds path for script stdin
extern		int		gInState;	// holds state for script in-pipe

extern		Str255	gOutFile;	// holds path for script stdout
extern		int		gOutState;	// holds state for script out-pipe
extern		int		gOutAppend;	// holds append flag for script stdout

/* ========================================== */

// install passed parameters;

int script_vars( void )
{
	int		error;
	int		arg;
	int		arg_max;
	char	*args;
	Str32	name;
	Str255	value;
	
	error = vlist_push_bead( i_shell );

	arg  = 0;
	arg_max = (**i_shell).nshc.argc;

	HLock( i_shell );

	while (!error && (arg < arg_max)) {
	
		NumToString( arg, name );
	
		args = &(**i_shell).nshc.arg_buf[(**i_shell).nshc.argv[arg]];
			
		if ( cStrLen( args ) > 255 ) {
			pipe_putStr_err( i_shell, "\pscript: Parameter is too long\r");
			(**i_interp).action = nsh_stop;
			}
		else
			pStrFromC( value, args );
			
		error = vlist_set_parm( i_shell, name, value );
		
		arg++;
	}
	
	if (!error) {
		NumToString(arg_max,value);
		error  = vlist_set_parm( i_shell, "\p#", value );
		}

	HUnlock( i_shell );
	
	if (error)
		pipe_putStr_err( i_shell, "\pscript: Could not allocate shell parameters.\r");
	
	return( error );
}
		
/* ========================================== */

// to set up a new script to run

void script_start( void )
{
	OSErr	error;
	InterpH	interp;

	error = noErr;
	
	interp = interp_new( i_shell );
	
	if (interp) {
	
		HLock( interp );
		error = fss_make( (**i_shell).cmd_path, &(**interp).script_fss );
		HUnlock( interp );
		
		if ( error ) {
			pipe_putStr_err( i_shell, "\pscript: Could not start script.\r" );
			(**i_interp).action = nsh_stop;
			(**i_interp).result = NSHC_ERR_GENERAL;
			return;
			}

		if (gInState)
			error = pipe_open_input( i_shell, gInFile, gInState, redir_by_script );

		if (gOutState && !error)
			error = pipe_open_output( i_shell, gOutFile, gOutState, redir_by_script, gOutAppend );

		(**interp).source = 1;

		if (error || script_vars())
			(**interp).action = nsh_stop;
		else
			(**interp).action = nsh_start;
		
		}
	else {
	
		pipe_putStr_err( i_shell, "\pscript: Could not allocate interpreter data.\r" );
		(**i_interp).action = nsh_stop;
		(**i_interp).result = NSHC_ERR_GENERAL;

		}
}

/* ========================================== */

int script_open(void)
{
	int		err;
	FSSpec	fss;
	
	if ((**i_interp).fRef)
		return( noErr );
		
	HLock( i_shell );
	err = fss_OpenDF( gHasFSSpec, &(**i_interp).script_fss, fsRdPerm, &(**i_interp).fRef);
	HUnlock( i_shell );
	
	(**i_interp).result = err;	

	if (err) {
		pipe_putStr_err( i_shell, "\pscript: Error opening script file.\r" );
		(**i_interp).action = nsh_stop;
		}
	else
		(**i_interp).action = nsh_continue;
		
	return( err );
}

/* ========================================== */

void script_close(ShellH shell)
{
	int	result;
	InterpH	interp;
	
	interp = (InterpH)(**shell).interp;
	
	vlist_pop_bead( shell );

	if ((**interp).inState == redir_in)
		pipe_close_input( shell, redir_by_script );

	if ((**interp).inState == redir_pipe_in)
		pipe_delete_input( shell, redir_by_script );

	if ((**interp).outState)
		pipe_close_output( shell, redir_by_script );

	if ((**interp).fRef) {
		result = FSClose((**interp).fRef);
		if (result && !(**interp).result)
			(**interp).result = result;
		(**interp).fRef = 0;
		}
}

/* ========================================== */

void script_read_raw(long position)
{
	int		err;
	long	count;
	
	err = script_open();
		
	if (!err) {
	
		(**i_interp).fpos = position;
		
		if ((**i_interp).fpos)
			err = SetFPos( (**i_interp).fRef, fsFromStart, (**i_interp).fpos );
			
		if (!err) {
		
			count = LINE_MAX;
			HLock( i_shell );
			err = FSRead( (**i_interp).fRef, &count, &(**i_interp).fbuf );
			HUnlock( i_shell );
			
			if (err && (err != eofErr)) {
				pipe_putStr_err( i_shell, "\pscript: Error reading script file.\r" );
				(**i_interp).action = nsh_stop;
				}
			}
			
		if (err == eofErr){
			if ( FSClose( (**i_interp).fRef ) )
				pipe_putStr_err( i_shell, "\pscript: Error closing script file.\r" );
			(**i_interp).fRef = 0;
			(**i_interp).feof = 1;
			}
			
		if (!count)
			(**i_interp).action = nsh_stop;

		if (count == LINE_MAX) {
			while ( (**i_interp).fbuf[count - 1] != '\r' )
				count--;
			if (!count) {
				pipe_putStr_err( i_shell, "\pscript: Input line exceeds maximum chars.\r" );
				(**i_interp).action = nsh_stop;
				}
			}
			
		(**i_interp).fbuf_chars = count;
		(**i_interp).fbuf_ptr = 0;
		}
}

/* ========================================== */

void script_read_file(void)
{
	if ((**i_interp).feof)
		(**i_interp).action = nsh_stop;
	else
		script_read_raw( (**i_interp).fpos + (**i_interp).fbuf_ptr );
}

/* ========================================== */

void script_set_pos( long pos )
{
	(**i_interp).feof = 0;
	
	script_read_raw( pos );
}