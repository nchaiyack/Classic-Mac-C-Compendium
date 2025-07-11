/* PWD tool to print working directory.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#include <stdio.h>

main()
{
	char buffer[256];
	
	if (getwd(buffer) == NULL) {
		fprintf(stderr, "pwd: %s\n", buffer);
		exit(1);
	}
	puts(buffer);
	exit(0);
}
