/* ===========================================

	interp.h

	Copyright (c) 1993,1994,1995 Newport Software Development
	
   =========================================== */

#define __INTERP__

/* =========================================== */

#define INBUF_SIZE 128

/* =========================================== */

typedef enum {
	ct_not_found,
	ct_null,
	ct_flow,
	ct_tbd,
	ct_internal,
	ct_external,
	ct_script } t_cmd_type;

typedef enum {
	int_cmd_none,
	int_cmd_set,
	int_cmd_unset,
	int_cmd_env,
	int_cmd_pathchk,
	int_cmd_which,
	int_cmd_cd,
	int_cmd_pwd,
	int_cmd_path,
	int_cmd_exit,
	int_cmd_export } t_int_cmd;
				
typedef enum {
	flow_cmd_none,
	flow_cmd_if,
	flow_cmd_then,
	flow_cmd_else,
	flow_cmd_endif,
	flow_cmd_while,
	flow_cmd_until,
	flow_cmd_do,
	flow_cmd_done } t_flow_cmd;
				
/* =========================================== */

typedef struct {
		
		// this record is controlled by these variables
		
		int			action;
		int			result;
		
		// line buffer
		
		int			source;					// 0 = console, 1 = file
		
		char		buf[ LINE_MAX ];
		int			buf_chars;
		int			buf_ptr;

		// control flow parameters
		
		int			flow_count;
		Handle		flow_list;
		
		// script file parameters
		
		FSSpec		script_fss;
		short		fRef;

		char		fbuf[ LINE_MAX ];
		long		fpos;
		int			fbuf_chars;
		int			fbuf_ptr;
		int			feof;

		// Redirection parameters
		
		Str255			inName;			// the stdin file, if any
		int				inRefNum;		// file ref, non-zero when file open
		int				inState;		// see pipe.h for states
		
		char			inBuf[ INBUF_SIZE ];
		int				inBufChars;
		int				inBufPtr;
		int				inBufEOF;
		
		Str255			outName;		// the stdout file, if any
		int				outRefNum;		// file ref, non-zero when file open
		int				outState;		// see pipe.h for states
	
		// Redirection parameters - for the children
		
		Str255			pipeName;		// the pipe file, if any
		int				pipeState;		// see pipe.h for states
		
		// list management
		
		Handle		next;
		
		} t_interp_rec;
		
typedef t_interp_rec **InterpH;

/* =========================================== */
