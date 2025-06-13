/* ==========================================

	interp.proto.h
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

#ifndef __INTERP__
#include "interp.h"
#endif

void    interp_a_bit(ShellH shell);
void    interp_cancel(ShellH shell);
void    interp_continue(void);
InterpH interp_dispose(ShellH shell);
int     interp_init(ShellH shell);
InterpH interp_new(ShellH shell);
void    interp_stop(void);
