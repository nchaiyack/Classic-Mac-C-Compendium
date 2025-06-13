
/* ==========================================

	multi2.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "resource.h"
#include "interp.h"

#include "cmd.proto.h"
#include "file.proto.h"
#include "flow.proto.h"
#include "interp.proto.h"
#include "multi.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"
#include "wind.proto.h"

extern	int		generation;
extern	int		next_name;

extern	ShellH	shells[MAXSHELL];

/* ========================================== */

void mf_new( void )
{
	ShellH	shell;
	
	shell = multi_allocate();
			
	if (shell)
		multi_set_name( shell );
}

/* ========================================== */

void mf_1st_new( void )
{
	ShellH	shell;
	
	shell = multi_allocate();
			
	if (shell) {
		multi_set_name( shell );
#if PROFESSIONAL
		pipe_putStr( shell, "\pWelcome to nShell-Pro™\r\rCopyright (c) 1994, 1995 Newport Software Development\r\r");
#else
		pipe_putStr( shell, "\pWelcome to the nShell™\r\rCopyright (c) 1994, 1995 Newport Software Development\r\r");
#endif
		}
}

/* ========================================== */

void mf_open( void )
{
	FSSpec	fss;
	
	if ( file_sfget( &fss ) )
		mf_open_file( &fss );
}

/* ========================================== */

void mf_open_file( FSSpec *fss )
{
	ShellH	shell;

	shell  = multi_allocate();

	if (shell) {
		SetWTitle((**shell).WindPtr, fss->name );
		pStrCopy( (**shell).shell_fss.name, fss->name );
		(**shell).shell_fss.parID = fss->parID;
		(**shell).shell_fss.vRefNum = fss->vRefNum;
		if ( file_open( shell ) )
			multi_dispose(shell);
		else {
			(**shell).Finger = (**(**shell).Text).selStart;
			wind_show_sel(shell);
			ShowWindow((**shell).WindPtr);
			if (path_update( shell ))
				pipe_putStr_err( shell, "\pmulti file: Could not initialize shell variables." );
			}
		}
}

/* ============================================= */

void mf_open_script( FSSpec *fss )
{
	ShellH	shell;
	InterpH	interp;
	Str255	path;

	shell  = multi_allocate();

	if (shell) {
	
		multi_set_name( shell );
		ShowWindow((**shell).WindPtr);
		
		interp = interp_new( shell );
		
		if (interp) {
		
			pStrCopy( (**interp).script_fss.name, fss->name );
			(**interp).script_fss.parID = fss->parID;
			(**interp).script_fss.vRefNum = fss->vRefNum;
			(**interp).source = 1;
			(**interp).action = nsh_start;
			
			if ( path_from_dir( fss->vRefNum, fss->parID, path ) )
				pipe_putStr_err( shell, "\pmulti file: Could not get path to script.\r" );
			else
				if ( vlist_set( shell, "\pPWD", path ) )
					pipe_putStr_err( shell, "\pmulti file: Could not set path to script.\r" );
		
			if ( vlist_push_bead( shell ) )
				pipe_putStr_err( shell, "\pmulti file: Could not allocate variable data.\r" );

			}
		else 
			pipe_putStr_err( shell, "\pmulti file: Could not allocate interpreter data.\r" );

		(**shell).Changed = 1;
		
		}
}

/* ========================================== */

int mf_save( ShellH shell )
{
	if ((**shell).shell_fss.vRefNum == 0)
		return( mf_save_as(shell) );
	else
		return( file_save(shell) );
}

/* ========================================== */

int mf_save_as( ShellH shell )
{
	int error;
	
	error = file_save_as( shell );
	
	if ( !error )
		SetWTitle((**shell).WindPtr, (**shell).shell_fss.name);
		
	return(error);
}

/* ========================================== */

void mf_revert( ShellH shell )
{
	int answer;
	Str255	prompt;
	
	prompt[0] = 0;
	pStrAppend( prompt, "\pRevert to last saved version of “");
	pStrAppend( prompt, (**shell).shell_fss.name);
	pStrAppend( prompt, "\p”?");
	answer = Ask(prompt,0);

	if (answer == askYES)
		if ( file_open( shell ) )
			multi_dispose(shell);
		else {
			(**shell).Finger = (**(**shell).Text).selStart;
			(**shell).action = nsh_start;
			(**shell).Changed = 0;
			}
}
