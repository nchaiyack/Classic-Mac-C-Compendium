#include <stdio.h>
#include "Main.h"	/* REQUIRED for UNIX file conversion! */

int main(int argc, char ** argv) {	

	int i;
	
	for (i = 0; i < argc; i++) {
		printf("%02d\t%s\n", i, argv[i]);
	}
	
	return 0;
}