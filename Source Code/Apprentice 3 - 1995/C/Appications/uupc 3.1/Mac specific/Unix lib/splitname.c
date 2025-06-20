
#include "splitname.proto.h"


/* splitname

		split and and copy directory and filename from 
		pathname to dir and name
*/
splitname(char *path, char *dir, char *name)
{
	register char *cp;
	register int i;

	strcpy( dir, path );
	i = strlen( dir);
	cp = dir + i;


	while ((i-- > 0) && (*--cp != '/') && (*cp != ':') );

	if (*cp == '/') cp++;
	else if (*cp == ':') cp++;
	
	strcpy( name, cp );
	*cp = '\0';

}

#ifdef TEST
#include <stdio.h>

main (void)

{
	char buf[100];
	char dir[100], name[100];

	while (gets( buf ) != NULL ) {
		splitname( buf, dir, name );
		printf( "%s -> \"%s\"   \"%s\"\n", buf, dir, name );
	}
}

#endif
