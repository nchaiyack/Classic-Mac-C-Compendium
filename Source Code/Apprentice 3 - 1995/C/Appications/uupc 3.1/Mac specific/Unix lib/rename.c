/* Copyright (C) 1984 by Manx Software Systems, Inc. */

#ifndef	THINK_C
#include	<quickdraw.h>
#include	<pb.h>

#define		_DEBUG
#include	<max/debug.h>
#endif	THINK_C
#include	<errno.h>
#include <stdio.h>

#include "rename.proto.h"

long setdir(char *path, char *dir, char *name)
{
	CInfoPBRec cpb;
	int err;

	if (*path == '/') (void)
	splitname( path, dir, name );
	
#ifdef TEST
	fprintf( stderr, "setdir: \"%s\" \"%s\" \"%s\"\n", name, dir, path );
#endif
	CtoPstr( dir );
	cpb.dirInfo.ioNamePtr = (StringPtr)dir;
	
	cpb.dirInfo.ioVRefNum = 0;
	cpb.dirInfo.ioFDirIndex = 0;
	cpb.dirInfo.ioDrDirID = 0L;

	if ((err = PBGetCatInfo( &cpb, 0 )) != 0 ) {

#ifdef TEST
	fprintf( stderr, "setdir: err %d\n", err );
#endif
		
	}

	return( cpb.dirInfo.ioDrDirID );

}

rename(char *old, char *new)
{
	register int i, err;
	long int freeb;
	int vrefnum;
	
	HParamBlockRec	pb;
	CMovePBRec 		cm;
		
	char nbuf[255], ndir[64], nname[32];
	char obuf[255], odir[64], oname[32];


	importpath(obuf, old);
	importpath(nbuf, new);
	
#ifdef TEST
	fprintf( stderr, "rename: obuf|%s -> nbuf|%s\n", obuf, nbuf );
#endif

	/* get dir refnums, fix path names, get dir name, and name */
	cm.ioNewDirID = setdir( nbuf, ndir, nname );
	cm.ioDirID = setdir( obuf, odir, oname );	
	
	/* use name only */
	if ( cm.ioDirID != cm.ioNewDirID ) {
#ifdef TEST
		fprintf( stderr, "rename: catmove\n" );
		fprintf( stderr, "rename: %ld|%s -> %ld|%s\n", 
				pbcm.cm.ioDirID, obuf, pbcm.cm.ioNewDirID, ndir );
#endif
		/* use full path name including volume name */
		CtoPstr(obuf);
		cm.ioNamePtr = (StringPtr)obuf;
		cm.ioNewName = (StringPtr)ndir;
		cm.ioVRefNum = 0;		/* use default directory */
		err = PBCatMove(&cm, FALSE);
	}
	else
		err = 0;
	if (err) {
		if (err == -48)
			err = EEXIST;
		errno = err;
		return(-1);
	}
	else {
#ifdef TEST
	fprintf( stderr, "rename: rename\n" );
	fprintf( stderr, "rename: %ld|%s -> %ld|%s\n", 
			pbcm.cm.ioDirID, oname, pbcm.cm.ioNewDirID, nname);
#endif
		CtoPstr(oname);
		pb.ioParam.ioNamePtr = (StringPtr)oname;
		pb.ioParam.ioVersNum = 0;
		CtoPstr(nname);
		pb.ioParam.ioMisc = nname;
		pb.fileParam.ioDirID =  cm.ioNewDirID;
		err = PBHRename(&pb, FALSE);
		if (err) {
			if (err == -48)
				err = EEXIST;
			errno = err;
			return(-1);
		}
		else
			return(0);
	}
}

#ifdef TEST
#include <errno.h>

main(void)
{
	char command[100];

	char f1[100], f2[100];

	while(gets(command) != NULL) {
		sscanf( command, "%s%s", f1, f2 );

		fprintf( stderr, "test: %s %s\n", f1, f2 );

		if ( rename( f1, f2 ) == 0 )
			fprintf( stderr, "test: OK\n\n" );
		else
			fprintf( stderr, "test: ERROR %d\n\n", errno );
	}
}
#endif
