/* ==========================================

	multi.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */
   
#define	MAXSHELL	20		// boundless optimism
#define	SHELL_CHARS	32000	// constrained optimism

#ifndef __NSHC__
#include "nshc.h"			// must be included before others to define constants
#endif

/* ========== Data For One Shell ========== */

typedef struct {

	FSSpec			shell_fss;		// the transcript file, if any
	
	WindowPtr		WindPtr;		// managing the shell window
	ControlHandle 	Scroll;			// vert scroll bar for text
	TEHandle		Text;			// the visible text
	int				Finger;			// points to next char to process
	int				Lines;			// used to manage text display
	int				Changed;		// shell changed since last save
	int				ShellNo;		// position in shells array
	
	t_nsh_state		action;			// managing shell functions
	
	Str255			cmd_path;		// path to an external command
	nshc_hdl		cmd_hndl;		// handle of a loaded command
	
	t_nshc_parms	nshc;			// data for a single command
	
	Handle			interp;			// a linked list of interpreter data
	Handle			root_interp;	// a handle to the root interpreter
	
	Handle			vlist;			// a linked list of shell variables
	
} t_shell_data;

typedef	t_shell_data	**ShellH;
