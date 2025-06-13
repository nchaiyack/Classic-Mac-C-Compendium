
#include <stdio.h>

#include "getargs.proto.h"

getargs(char *cp, char *flds[])
{
	register int count = 0;

	while ( *cp != '\0' ) {
		while ( *cp == ' ' || *cp == '\t')
			*cp++ = '\0';
		if (*cp == '\n')
			*cp = '\0';
		if (*cp != '\0' ) {
			flds[count++] = cp;
			while (*cp != ' ' && *cp != '\t' &&
					 *cp != '\n' && *cp != '\0' )
				cp++;
		}
	}
	flds[count] = NULL;
	return(count);
}

#ifdef testtoken
	/* tokenize command line */
	while ( *cp != '\0' ) {
		/*debuglevelMsg("\PCheck isspace(*cp++)");*/
		if ( isspace(*cp) ) {
			/* terminate previous token */
			*cp++ = '\0';
			/* eat white space */
			while ( isspace(*cp) ) cp++;
		}
		else {
			/* put address of token into argv, inc argc */
			*argvp++ = cp;
			argcp++;
			fprintf( stderr, "shell: token %s\n", cp );
			/*debuglevelMsg("\PCheck !isspace(*cp++)");*/
			while ( !isspace(*cp) ) cp++;
			
		}
	}
#endif

