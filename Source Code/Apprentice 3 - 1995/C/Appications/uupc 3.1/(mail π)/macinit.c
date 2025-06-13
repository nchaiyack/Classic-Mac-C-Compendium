/*			macinit.c




*/

#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#include	<stdio.h>

#include "macinit.proto.h"

#include "macinit.proto.h"


macinit(void)
{
	SysEnvRec SysEnv;
	int i;
	
	SysEnvirons(1, &SysEnv);
	
	if ((i = OpenRFPerm("\pUUPC Settings", 0, fsRdPerm)) == -1 &&
		(i = OpenRFPerm("\pUUPC Settings", SysEnv.sysVRefNum, fsRdPerm)) == -1) {
		fprintf( stderr, "ERROR: Can't find \"UUPC Settings\" file here or in System Folder\r" );
	}
	return 0;
}

