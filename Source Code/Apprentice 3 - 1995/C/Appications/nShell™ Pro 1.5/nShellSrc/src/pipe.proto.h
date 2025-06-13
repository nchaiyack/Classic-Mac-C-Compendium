/* ==========================================

	pipe.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#ifndef __INTERP__
#include "interp.h"
#endif

void  pipe_close_input( ShellH shell, int scope );
void  pipe_close_output( ShellH shell, int scope );
void  pipe_delete_input( ShellH shell, int scope );
int   pipe_fetch_line(ShellH shell, InterpH interp, char *buf, int *count);
int   pipe_getc( ShellH shell );
int   pipe_getc_file(ShellH shell, InterpH interp);
int   pipe_getc_wind( ShellH shell );
int   pipe_gets(ShellH shell, char *s, int limit);
int   pipe_getStr(ShellH shell, Str255 s);
void  pipe_make_name( ShellH shell, Str255 path );
OSErr pipe_open_input( ShellH shell, Str255 filename, int inState, int scope );
OSErr pipe_open_output( ShellH shell, Str255 filename, int outState, int scope, int append );
int   pipe_open_util( ShellH shell, Str255 filename, int append );
void  pipe_putc( ShellH shell, char c);
void  pipe_putc_err( ShellH shell, char c);
void  pipe_putc_file( ShellH shell, InterpH interp, char c);
void  pipe_putc_wind( ShellH shell, char c);
void  pipe_puts( ShellH shell, char *p );
void  pipe_putStr( ShellH shell, Str255 string );
void  pipe_putStr_err( ShellH shell, Str255 string );
void  pipe_puts_err( ShellH shell, char *p );
void  pipe_puts_err_util( ShellH shell, char *p, long count );
void  pipe_puts_file( ShellH shell, InterpH interp, char *p, long count );
void  pipe_puts_util( ShellH shell, char *p, long count );
void  pipe_puts_wind( ShellH shell, char *p, long count );
