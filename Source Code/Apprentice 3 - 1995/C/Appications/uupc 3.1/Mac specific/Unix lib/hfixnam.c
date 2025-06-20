
#ifdef TEST
#include <stdio.h>
#endif
#include <pb.h>

#include "hfixnam.proto.h"


hfixnam(char *path, char *npath)
{

	HPrmBlkRec pb;
	char vname[256];

	strcpy( vname, "" );
	strcpy( npath, "" );
	
	pb.ioNamePtr = vname;

#ifdef TEST
	fprintf( stderr, "hfixnam: before \"%s\"\n", path );
#endif

	pb.ioVRefNum = fixnam( path, npath );

#ifdef TEST
	fprintf( stderr, "hfixnam: vrefnum %d\n", pb.ioVRefNum );
	fprintf( stderr, "hfixnam: after \"%s\"\n", npath );
#endif

	pb.u.hvp.ioVolIndex = 0;

	if (PBGetVInfo(&pb, 0) == 0) {
		ptoc( vname );

#ifdef TEST
		fprintf( stderr, "hfixnam: \"%s\"\n", vname );
#endif

		strcat( vname, npath );
		strcpy( npath, vname );
	}
	
	return pb.ioVRefNum;

}


#ifdef TEST
#include <errno.h>

main(void)
{
	char command[100];
	char ncommand[100];
	int vrefnum;

	while ( gets( command ) != NULL) {
		fprintf( stderr, "test: %s \n", command );
		vrefnum = hfixnam( command, ncommand );
		fprintf( stderr, "test: %d %s\n", vrefnum, ncommand );
	}
}
#endif
