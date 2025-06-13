/* ==========================================

	interp.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "pipe.h"

#include "interp.proto.h"
#include "flow.proto.h"
#include "parse.proto.h"
#include "pipe.proto.h"
#include "script.proto.h"

/* ========================================== */

ShellH	i_shell;
InterpH	i_interp,i_root_interp;

/* ========================================== */

extern	Boolean	gHasFSSpec;	// true if the calls are available

/* ========================================== */

// to allocate interpreter data blocks (one for the console and
// one for each active shell

InterpH interp_new(ShellH shell)
{
	InterpH	temp;
	InterpH	this;
		
	this = (InterpH)NewHandleClear( sizeof( t_interp_rec ) );
	
	if (!this) return(this);
	
	(**this).next = (**shell).interp;	
	(**shell).interp = (Handle)this;

	return(this);
}

/* ========================================== */

// to dispose the topmost interpreter data block

InterpH interp_dispose(ShellH shell)
{
	Handle	temp;
	Handle	next;

	script_close(shell);

	temp = (**shell).interp;
	
	flist_del_all( (InterpH)temp );

	next = (**(InterpH)temp).next;
	
	(**(InterpH)next).result = (**(InterpH)temp).result;
	
	DisposeHandle(temp);
	
	(**shell).interp = next;
	return( (InterpH)next );
}

/* ========================================== */

// to create the first interp record for a new shell

int interp_init(ShellH shell)
{
	InterpH	temp;
	
	(**shell).interp = NULL;

	temp = interp_new( shell );
	
	(**shell).root_interp = (Handle)temp;
	
	return( temp == NULL );
}

/* ========================================== */

// cancel all active scripts and stop the console

void interp_cancel(ShellH shell)
{
	InterpH	interp;
	
	interp = (InterpH)(**shell).interp;
	
	while ((**interp).next)
		interp = interp_dispose(shell);

	flist_del_all( interp );
	
	(**interp).action = nsh_idle;
}

/* ========================================== */

// state machine - continue

void interp_continue(void)
{
	t_cmd_type	ct;
	
	(**i_interp).action = nsh_continue;
	
	ct = parse_pass_1();

	if ((**i_interp).action == nsh_continue) {
	
		if ( ct == ct_tbd )
			ct = parse_pass_2();
			
		switch (ct) {
			case ct_internal:
			case ct_external:
				(**i_shell).nshc.action = nsh_start;
				break;
			case ct_script:
				script_start();
				break;
			case ct_not_found:
				pipe_puts_err( i_shell, (**i_shell).nshc.arg_buf );
				pipe_putStr_err( i_shell, "\p: Command not found\r" );
				break;
			case ct_flow:
#if PROFESSIONAL
				flow();
#else
				pipe_putStr_err( i_shell, "\pFlow Of Control is not available in this version of the nShellª.\r" );
				(**i_interp).result = NSHC_ERR_GENERAL;
				(**i_interp).action = nsh_stop;
#endif
				break;
			default:
				break;
			}
			
		}	
}

/* ========================================== */

// state machine - stop

void interp_stop(void)
{
	if ((**i_interp).next)
		i_interp = interp_dispose(i_shell);
	else {
		flist_del_all( i_interp );
		(**i_interp).action = nsh_idle;
		}
}

/* ========================================== */

// state machine

void interp_a_bit(ShellH shell)
{
	int		result;
	InterpH	root;
	
	i_shell = shell;
	i_interp = (InterpH)(**shell).interp;

	// close command level pipes, if any

	root = (InterpH)(**shell).root_interp;
	
	if ((**root).inState == redir_in)
		pipe_close_input( i_shell, redir_by_command );

	if ((**root).inState == redir_pipe_in)
		pipe_delete_input( i_shell, redir_by_command );

	if ((**root).outState)
		pipe_close_output( i_shell, redir_by_command );
		
	// state machine
	
	switch((**i_interp).action) {
		case nsh_start:
		case nsh_continue:
			interp_continue();
			break;
		case nsh_stop:
			interp_stop();
			break;
		case nsh_idle:
			break;
		}
		
	i_shell = NULL;
	i_interp = NULL;
}
