/* 		lib.c

*/

#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "host.h"
#ifdef	 THINK_C
# include <unix.h>
# include <fcntl.h>
#endif

#ifndef NULL
#define NULL 0L
#endif

#include "lib.proto.h"

MKDIR(char *path)
{
	char * cp = path;
	char *colon, *slash, *sep, sepchar;
	OSErr err;
	int first = 1;

	if ( *cp == '\0' )
		return( 0 );

	/* see if we need to make any intermediate directories */
	while (cp && *cp) {
		slash = index (cp, SEPCHAR);
		colon = index (cp, DIRCHAR);
		sep = slash;
		if (colon && (!slash || colon < slash)) {
			sep = colon;
		}
		if (sep) {
			sepchar = *sep;
			if (sepchar != DIRCHAR || !first /* || sep != path */) {
				*sep = '\0';
				err = mkdir( path );
				if (err != noErr && err != dupFNErr) {
					printmsg(0, "mkdir error %d on %s", err, path);
				}
				*sep = sepchar;
			}
			sep++;
			first = 0;
		}
		cp = sep;
	}

	/* make last dir */
	err = mkdir(path);
	if (err != noErr && err != dupFNErr) {
		printmsg(0, "mkdir error %d on %s", err, path);
	}
	return( err );

}

CHDIR(char *path)
{
	char * cp = path;

	if ( *cp == '\0' )
		return( 0 );

	MKDIR( path );

	/* change to last directory */
	return( chdir( path ) );

}

int OPEN (char *name, int mode)
{
	char   nname[255];
	int		results;
	cnvMac(name, nname);
	mapMacCaseness(nname);
	results = open( nname, mode );
	return results;
}

FILE * FOPEN(char *name, char *mode, char ftype)
{

	char * last;
	FILE * results;
	char   nname[255];
	char   opmode[5];
	int	   len;

	/* are we opening for write or append */

	FILEMODE( ftype );
	strcpy(opmode, mode);
	if (ftype == 'b') {
		len = strlen(opmode);
		*(opmode+len) = ftype;
		*(opmode+len+1) = '\0';
	}
#if 0
	DebugStr("\pIn FOPEN");
#endif
	cnvMac(name, nname);
	mapMacCaseness(nname);
	results = fopen( nname, opmode );

	if ( results != (FILE *) NULL ) {		/* fixed [garym 3/21/90 */
		/* success, if not reading, set file info */
		if (*mode != 'r') {
			FInfo	fInfo;
			CtoPstr(nname);
			if (GetFInfo((StringPtr)nname, 0, &fInfo) == noErr) {
				fInfo.fdCreator = 'MPS ';
				(void)SetFInfo((StringPtr)nname, 0, &fInfo);
			}
		}
		return( results );
	}
	/* are we opening in sub-directory */
	last = rindex( name, SEPCHAR );

	/* lets just verify that all sub-dir's exist */
	if ( last != (char *) NULL ) {
		*last = '\0';
		MKDIR( name );
		*last = '/';
	}

	/* now try open again */
	return( fopen( nname, opmode ));

}

int CREAT(char *name, int mode, char ftyp)
{

	char *	last;
	int 	results;
	char	nname[255];
	FILE *fp;
	
	/* are we opening for write or append */
	FILEMODE( ftyp );
	cnvMac(name, nname);
	mapMacCaseness(nname);
	mode = O_CREAT|O_TRUNC|O_WRONLY;	/* ignore Unix perms */
	if (ftyp == 'b') mode |= O_BINARY;
	else mode |= O_TEXT;	
	results = creat( nname, mode );

	if ( results != -1 )
		return( results );	/* success */

	/* are we opening in sub-directory */
	last = rindex( name, '/' );

	/* lets just verify that all sub-dir's exist */
	if ( last != (char *) NULL ) {
		*last = '\0';
		MKDIR( name );
		*last = '/';
	}

	/* now try open again */
	results = creat( nname, mode );
	if ( results != -1 )
		return( results );	/* success */
		
	/* maybe it already exists, see if we can open it */
	fp = FOPEN( nname, "w", ftyp );
	if ( fp != NULL ) {
		fclose(fp);
		results = 0;		/* it already exists, no problem */
	} else {
		results = -1;
	}
		
	return( results );

}

int	UNLINK(char *path)
{
	char	mpath[255];
	cnvMac(path, mpath);
	mapMacCaseness(mpath);
	return(unlink(mpath));
}


extern int debuglevel;
extern int remote;

#define MASTER 1


int getargs(char *line, char **flds)
{
	int i = 0;
	char *s, quoteChar;
	int quoteit, backslash;
	
	quoteChar = 0;

	while ( (*line != '\0') && (*line != '\n') )
	{
	   if ( isspace(*line) )
	   {
	      line++;
	      continue;
	   }
	   if (*line == '"' || *line == '\'') {
	     quoteit = 1;
	     quoteChar = *line;
	     line++;
	   } else {
	     quoteit = 0;
	   }
	   backslash = 0;
	   *flds++ = line;
	   i++;
	   while (*line != '\0') {
	     if (!quoteit && !backslash && isspace(*line)) break;
	     if (backslash) {
	     	backslash = 0;
	     } else {
	     	if (*line == '\\') {
	     	   backslash = 1;
	     	} else if (*line == quoteChar) {
	     	   quoteit = !quoteit;
	     	}
	     }
	     line++;
	   }
	   if (quoteChar != 0 && *(line-1) == quoteChar) {
	   		*(line-1) = '\0';
	   }
	   if (isspace(*line)) *line++ = '\0';
	}
	return(i);
}


