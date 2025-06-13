/*****************************************************************
*	Play.c -- Gregory E. Allen
******************************************************************
*	v1.00 -- 31 Mar 94; Initial release
******************************************************************
*	Name:
*		Play -- Play a System 7 Sound file
*	Synopsis:
*		Play file…
*	Description:
*		"Play" plays the sound file.
*		(actually plays all sound resources in a file)
******************************************************************
*	©1994, Gregory E. Allen <gallen@arlut.utexas.edu>
*	This file may be distributed freely for non-profit use,
*	as long as this header message accompanies it.
*****************************************************************/

#include	<Types.h>
#include 	<ctype.h>
#include 	<fcntl.h>
#include 	<string.h>
#include 	<stdio.h>
#include	<ErrMgr.h>
#include	<CursorCtl.h>
#include	<Errors.h>
#include	<Devices.h>
#include	<Serial.h>
#include	<Files.h>
#include	<Events.h>
#include 	<Strings.h>
#include 	<Resources.h>
#include 	<Sound.h>


/* Variables local to this file */

static	char	errorBuffer[256];

static	char	*usage = "# Usage - %s file….\n";

main(int argc, char *argv[])
{
	long			status;
	long			parms;
	long			files;
	long			length;
	long			max;
	FInfo			fi;
	short			input;
	short			err;
	char			pName[256];
	short			i,n;
	Handle			snd;
	
	status = files = 0;
	max = strlen("Total");
	InitCursorCtl(nil);
		
	for (parms = 1; parms < argc; parms++) {
		length = strlen(argv[parms]);
		if (*argv[parms] != '-') {
			argv[++files] = argv[parms];
			if (max < length)
				max = length;
		} else {
			fprintf(stderr,"### %s - \"%s\" is not an option.\n", argv[0], argv[parms]);
			fprintf(stderr, usage, argv[0]);
			return 1;
		}
	}
	if (files == 0) {
		fprintf(stderr,"# %s - No file specified.\n", argv[0]);
	} else {
		for (parms = 1; parms <= files; parms++) {
			strncpy(pName,argv[parms],255);
			c2pstr(pName);
			if( err = GetFInfo( pName, 0, &fi )) {
				fprintf(stderr,"### %s - Unable to open file '%s'.\n", argv[0], argv[parms]);
				fprintf(stderr,"# %s\n", GetSysErrText(err, errorBuffer));
				status = 2;
			} else {
				if ( fi.fdType != 'sfil' )
					fprintf(stderr,"### '%s' is not a System 7 Sound.\n", argv[parms]);
				if ( (input = OpenResFile( pName )) == -1 ) {
					fprintf(stderr,"### Error opening resource file '%s'.\n", argv[parms]);
					status = 2;
				} else {
					if ( !(n = Count1Resources('snd ')) )
						fprintf(stderr,"### '%s' Contains no sound resources.\n", argv[parms]);
					for (i=1; i<=n; i++ )
						if (snd = Get1IndResource('snd ', i))
							if (err = SndPlay(NULL, snd, 0))
								fprintf(stderr,"# Error playing sound resource.\n");
					CloseResFile(input);
				}
			}
		}
	}
	fflush(stdout);
	return status;
}
