#ifdef THINK_C
# include "unixlibproto.h"
# include "stdlib.h"
# include "string.h"
#endif THINK_C

#include "ndir.h"
#include <stdio.h>

#ifndef	THINK_C
#include <pb.h>
#endif	THINK_C
#ifdef TEST
#include <stdio.h>
#define _DEBUG
#include <max/debug.h>
#endif

#include "ndir.proto.h"

/*
 * support for Berkeley directory reading routine on a V7 file system
 */
 
/*
 * open a directory.
 */

DIR *
opendir(char *name) 
{
	static DIR dir;
	register DIR *dirp = &dir;
	register int fd;

	char 		buf[255];
	CInfoPBRec	cpb;
	HParamBlockRec hpbr;
	WDPBRec		wdpb;
	int 		err, currDir;

	/* remember current dir */
	(void) GetVol((StringPtr) buf, &currDir);
	/* fix name, and get volume reference number */

	cnvMac(name, buf);
	
	cpb.dirInfo.ioVRefNum = 0;
	cpb.dirInfo.ioNamePtr = (StringPtr)CtoPstr(buf);
	cpb.dirInfo.ioFDirIndex = 0;
	cpb.dirInfo.ioDrDirID = 0L;

#ifdef TEST
	fprintf( stderr, "opendir: %s\n", buf );
#endif

	if ((err = PBGetCatInfo( &cpb, 0 )) != 0 ) {
		printmsg(0, "opendir: PBGetCatInfo error %d on %s", err, name );
		return((DIR *)NULL);
	}
/**** repeated calls to malloc fill up system heap [3/24/90 garym]
	if((dirp=(DIR *)malloc(sizeof(DIR))) == (DIR *)NULL) {
		fprintf(stderr, "opendir: couldn't malloc %d got 0x%x\n", sizeof(DIR), dirp);
		return((DIR *)NULL);
	}
****/
	hpbr.volumeParam.ioVRefNum = 0;
	hpbr.volumeParam.ioVolIndex = -1;
	hpbr.volumeParam.ioNamePtr = (StringPtr) buf;
	if ((err = PBHGetVInfo(&hpbr, FALSE)) != noErr) {
		printmsg(0, "opendir: PBHGetVInfo error %d on %s", err, name );
		return((int)NULL);
	}	
	dirp->ioVRefNum = hpbr.volumeParam.ioVRefNum;
	dirp->ioDrDirID = cpb.dirInfo.ioDrDirID;
	dirp->ioFDirIndex = 1;
	dirp->currdir = currDir;
#ifdef	UNDEF
	wdpb.ioCompletion = NULL;
	wdpb.ioNamePtr = NULL;
	wdpb.ioVRefNum = cpb.dirInfo.ioVRefNum;
	wdpb.ioWDProcID = APPSIGNATURE;
	wdpb.ioWDDirID = cpb.dirInfo.ioDrDirID;
	wdpb.ioWDIndex = wdpb.ioWDVRefNum = 0;
	if (PBOpenWD(&wdpb, FALSE) != noErr) {
		return((DIR *)NULL);
	}
	dirp->ioVRefNum = wdpb.ioVRefNum;	/* remember the WD id */
	wdpb.ioVRefNum = 0;
	if (SetVol((int)NULL, dirp->ioVRefNum ) != noErr) {
		return((DIR *)NULL);
	}
#endif	UNDEF
#ifdef TEST
	fprintf( stderr, "opendir: # %ld\n", cpb.u.di.ioDrDirID );
#endif

	return(dirp);
}


/*
 * get next entry in a directory.
 */
struct direct *
readdir(DIR *dirp)
{
	CInfoPBRec cpb;
	int err;
	static struct direct dir;

#ifdef TEST
	fprintf( stderr, "readdir: v:%d d:%ld i:%d\n", 
			dirp->ioVRefNum, dirp->ioDrDirID, dirp->ioFDirIndex );
#endif
	strcpy( dir.d_name, "" );
	cpb.dirInfo.ioNamePtr = (StringPtr)dir.d_name;
	cpb.dirInfo.ioVRefNum = dirp->ioVRefNum;
	cpb.dirInfo.ioDrDirID = dirp->ioDrDirID;
	cpb.dirInfo.ioFDirIndex = dirp->ioFDirIndex++;

	if ((err = PBGetCatInfo( &cpb, 0 )) != 0 ) {
#ifdef TEST
		fprintf( stderr, "readdir: err %d\n", err );
#endif
		return NULL;
	}

	dir.d_ino = 0;
	PtoCstr( dir.d_name );

#ifdef TEST
	fprintf( stderr, "readdir: OK \"%s\"\n",  dir.d_name);
#endif

	dir.d_namlen = strlen(dir.d_name);
	dir.d_reclen = DIRSIZ(&dir);
	return (&dir);
	
}

/*
 * close a directory.
 */
void
closedir(DIR *dirp) {
	WDPBRec	wdpb;
#ifdef	UNDEF	
	(void) (NULL, dirp->currdir);
	wdpb.ioCompletion = NULL;
	wdpb.ioNamePtr = NULL;
	wdpb.ioVRefNum = dirp->ioVRefNum;
	wdpb.ioWDProcID = APPSIGNATURE;
	wdpb.ioWDDirID = dirp->ioDrDirID;
	wdpb.ioWDIndex = wdpb.ioWDVRefNum = 0;
	PBCloseWD (&wdpb, FALSE);
#endif	UNDEF
/********** HOO BOY *************
 If this is doing what I think it's doing, it's REALLY crazy-badness!
	if(dirp != (DIR *)NULL)
		free((char *)dirp);
	else
		fprintf(stderr, "closedir: tried to free NULL *directory\n");
 ********************************/
}


#ifdef TEST
#include <errno.h>

main(void)
{
	char command[100];

	struct DIR *dirp;
	struct direct *dp;
	
	while(gets(command) != NULL) {

		fprintf( stderr, "test: %s\n", command );

		if ((dirp = opendir( command )) == NULL ) {
			fprintf( stderr, "couldn't open dir %s\n", command );
		}
		while ((dp = readdir(dirp)) != NULL) 
			fprintf( stderr, "%s", dp->d_name );

		closedir( dirp );
	}
}
#endif
