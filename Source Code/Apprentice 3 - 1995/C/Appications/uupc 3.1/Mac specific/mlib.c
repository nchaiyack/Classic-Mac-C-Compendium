/*		lib.c


		macintosh library
		

	Things to do in uu host

		serial I/O
		
		directory stuff
			opendir, readdir, closedir

		prolog and epilog

		system call

*/

#include <stdio.h>
#include "host.h"
#ifndef  THINK_C
#include <sgtty.h>

#include "mlib.proto.h"

int get_one(void)
{
	char c;
	
	struct sgttyb stty, sttyo;
	ioctl( 0, TIOCGETP, &stty );
	sttyo = stty;
	stty.sg_flags |= CBREAK;
	stty.sg_flags &= ~ ECHO;
	ioctl( 0, TIOCSETP, &stty );
	c = fgetc( stdin );
	ioctl( 0, TIOCSETP, &sttyo );
	return( c );
}
#else
# include <console.h>
get_one(void) {
	int ch;
	csetmode(C_CBREAK, stdin);
	ch = getc(stdin);
	csetmode(C_ECHO, stdin);
	return(ch); 
}
#endif
