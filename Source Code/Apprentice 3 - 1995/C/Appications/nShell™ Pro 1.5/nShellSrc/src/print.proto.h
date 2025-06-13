/* ==========================================

	print.proto.h

	Copyright (c) 1993,1994,1995 Newport Software Development
	
	This code is not currently linked into the nShell app.  I had intended
	to allow printing of shell histories (or selections thereof), but never
	got around to it.
	
   ========================================== */

#ifndef __PRINTTRAPS__
#include <PrintTraps.h>
#endif

void print_cleanup( void );
int  print_error( Str255 action );
int  print_how_many( void );
void print_setup( void );
int  print_setup_handle( void );
void print_shell( ShellH shell );
int  print_spooler( TEHandle shellTE );
