#include <stdio.h>
#include <stdlib.h>
#include <console.h>


	void
main (argc, argv)

	int	   argc;
	char **argv;
{
	int		   genarch (int argc, char **argv);


	argc = ccommand (&argv);

	genarch (argc, argv);
}


#define main	genarch
#include "genarch.c"
	