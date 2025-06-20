/* ==========================================

	pipe.h

	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#define __PIPE__

typedef enum {	redir_none,
				redir_in,
				redir_pipe_in,
				redir_out,
				redir_outerr,
				redir_out_pipe,
				redir_outerr_pipe,
				redir_out_null,
				redir_outerr_null,
				redir_out_tty,
				redir_outerr_tty } t_redir_state;

typedef enum {	redir_by_command, redir_by_script } t_redir_scope;
