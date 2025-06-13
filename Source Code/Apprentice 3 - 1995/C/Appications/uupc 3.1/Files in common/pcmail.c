
/* 		pcmail.c



		copyright (C) 1987 Stuart Lynne

		Copying and use of this program are controlled by the terms of the
		Free Software Foundations GNU Emacs General Public License.

			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.


		version		0.1		March 31/1987


pcmail

		pcmail address1 address2 ... < the.message

description

	An 822 compatible (hopefully) mail delivery system for pc's.

	Designed for delivering mail on a pc based system. It will put
	local mail (ie, not @ or ! in address) into files in the default
	mail directory.

	If remote it will put into an outgoing mailbag in the default mail
	directory. Performs a simple bundling of mail messages into one
	file with arguments prepended as To: arg header lines. And adds a
	Message-Lines: header which gives the number of lines in the
	content part of the message (after the first blank line).

		pcmail john jack jill@xyz.uucp < afile

		To: john
		To: jack
		To: jill@xyz.uucp
		X-Message-Lines: ?????
		Content-Length: ?????

		...
		...
		...

	Content-Length: is used without X- prepended to be compatible with AT&T
	Mail bundles. This is not 822 compatible per se, but is allowed.

	It also adds the from From and Date lines. Subject: lines may be inserted
	by placing them at the beginning of the message.

	A Unix version should lock the /usr/mail/mailbag file.

	Another program called rpcmail will unbundle the files created by
	pcmail and deliver each message to the local rmail. So conceptually

		(pcmail ..... < ...; pcmail .... < ...) | sz -> rz | rpcmail

	would deliver remote messages intact.

environment variables

	The following evironment variables are used:

		MAILBOX		current user's mailbox, 	"stuart"
		NAME		current user's name, 		"Stuart Lynne"
		DOMAIN		domain of this machine, 	"slynne.mac.van-bc.can"
		MAILDIR		where is mail kept,			"mpw:mail"
		ALIAS		user's mail aliases			".alias"
		MBOX		filewhere is mail kept		""					BH-C
112
compiling

	Compiled by itself it will operate as a standalone program. If the
	compiler option:

		-DNOMAIN

	is used, it will compile as a routine:

		pcmail (argc, argv)
		char **argv;
		int argc;

	and can be used internally in other programs.


Customization

	PCMAIL		mailbag for remote mail
	FAKEUUX		emulate uux, make appropriate files in SPOOLDIR

	RMAIL		rmail

	DEBUG1		first level Debug trace


*/

#include <stdio.h>
#ifdef	 THINK_C
#include <unix.h>
#endif

#include "pcmail.h"

#include "host.h"
#include "ndir.h"

#ifdef THINK_C					/* BH-C */
#include "script.h"				/* BH-C */
#endif THINK_C					/* BH-C */

/*
#endif
*/

#define FORWARD		"Forward to"


#define	SBUFSIZ		124
#define RMAILCMDSIZ		120


FILE   *mailfile;
#ifdef Upgrade
FILE   *aliasfile;
#endif Upgrade
FILE   *tempfile;

char	buf[BUFSIZ];
char	miscbuff[255];
long int lines = 0;
long int bytes = 0;
long int sequence = 0;

time_t tloc;
char chartime[128];	/* current time in characters */
char *thetime;

char	tfilename[255];
char	mfilename[255];
char	mailsent[255];

#ifdef Upgrade
char	afilename[255];
#endif Upgrade

int local = TRUE;

char viaremote[BUFSIZ] = "";
char thisremote[BUFSIZ] = "";

char 	remotes[BUFSIZ];

char gotitfrom[BUFSIZ] = "";
char gotfromuser[BUFSIZ] = "";


char uucp[] = "uucp";

char *fgets();
int fputs();

#ifdef NOMAIN

#ifdef RMAIL
#define main	rmail
#else
#define	main	lmail
#endif

#define exit	return

extern int debuglevel;

#else
int debuglevel;
#endif

#ifndef RMAIL
char	Subject[132] = "";
#endif

char *mcurdir;
char s_mcurdir[128];	/* current directory path (save malloc call) */
char * getcwd();
int chdir();

#ifdef MULTIFINDER
#define UnImplTrapNum 0x9F
#define StripAddressTrapNum 0x55

/* #include "pcmail.proto.h"	BH-C */

#include "pcmail.proto.h"
long int Strip(long int);
long int Strip(long int foo)
{
	if (NGetTrapAddress( StripAddressTrapNum,OSTrap) !=
	     NGetTrapAddress(UnImplTrapNum,ToolTrap)) {
		return (long int) StripAddress((void *) foo);
	} else {
		return (Lo3Bytes & (long int) foo);
	 }
}	
#endif

int
sendit(int argc, char **argv, char *addr, char *remotes) {
	int s1, s2;
	int err = 1;
	char routelist[BUFSIZ], tryroute[BUFSIZ], composite[BUFSIZ];
	
	char *bang, *atsign, *percent;
	char *addx;					/* BH-C */

/*
	Recognize and strip off our own nodename and domain name, whether by l@nd, s%, or sk!
*/

	strcpy(composite, nodename);
	strcat(composite, ".");
	strcat(composite, domain);

#ifdef NOTDEF
	fprintf(stderr, "Address %s, node %s, domain %s\n", addr, nodename, domain);
	fflush(stderr);
#endif
	
	atsign = strrchr(addr, '@');
	
	while (atsign != NULL && (IUMagIDString(domain, atsign+1, strlen(domain), strlen(atsign+1)) == 0 ||
	                                        IUMagIDString(composite, atsign+1, strlen(composite), strlen(atsign+1)) == 0 ||
	                                        IUMagIDString(nodename, atsign+1, strlen(nodename), strlen(atsign+1)) == 0))  {
		*atsign = '\0';
		atsign = strrchr(addr, '@');
	}
	
	bang = strchr(addr, '!');

	while (bang != NULL && (IUMagIDString(nodename, addr, strlen(nodename), bang-addr) == 0 ||
	                                      IUMagIDString(domain, addr, strlen(domain), bang-addr) == 0  ||
	                                      IUMagIDString(composite, addr, strlen(composite), bang-addr) == 0)) {
#ifdef NOTDEF
	        SysBeep(1);
#endif
		addr = bang+1;
		bang = strchr(addr, '!');
	}
	
	percent = strrchr(addr, '%');
	
	while (percent != NULL && (IUMagIDString(domain, percent+1, strlen(domain), strlen(percent+1)) == 0 ||
	                                           IUMagIDString(composite, percent+1, strlen(composite), strlen(percent+1)) == 0)) {
		*percent = '\0';
		percent = strrchr(addr, '%');
	}
	
	
	if(atsign || bang || percent) {
		if ( debuglevel > 5 )
			fprintf( stderr, "pcmail: send to remote\n" );
		
		if (atsign || percent) {
			/* BH-C v */
			if (atsign) {
				addx = atsign + 1;
				sprintf(routelist, ",%s,", routevia);			/* domain names must be lowercase */
				sprintf(tryroute, ",%s:", addx);
				for (addx = tryroute; *addx != '\0'; addx++)	{/* make address lower case */
					if ( isupper(*addx) )
						*addx = *addx + 32;
				}
				atsign = tryroute; addx = NULL;
				while ( (atsign != NULL) && ((addx = strstr(routelist,atsign)) == NULL) ) { /* find partial domain */
					atsign = strchr(atsign,'.');
					*atsign  = ',';
				}
				strcpy(thisremote, mailserv);
				if (addx != NULL) {
					addx++; /* skip ',' */
					addx = strchr(addx, ':');
					atsign = strchr(addx, ',');
					if (addx != NULL) {
						addx++;
						if (atsign != NULL) {
							if (addx < atsign) {
								strncpy(thisremote, addx,atsign - addx);
								thisremote[atsign - addx] = '\0';
							}
						} else
							strcpy(thisremote,addx);
					}
				}
			} else
			/* BH-C ^ */
			strcpy(thisremote, mailserv);
		} else {
			strncpy(thisremote, addr, bang - addr);
			thisremote[bang - addr] = '\0';
			sprintf(routelist, ",%s,", routevia);
			sprintf(tryroute, ",%s,", thisremote);
			if (strstr(routelist, tryroute) == NULL) {
				/* BH-C v */
				sprintf(tryroute, ":%s,", thisremote);
				if (strstr(routelist, tryroute) == NULL)
				/* BH-C ^ */
				strcpy(thisremote, mailserv);
			}
		}

		/* BH-C v */
		if ( debuglevel > 4 )
			fprintf( stderr, "pcmail: viaremote - %s\n",thisremote );
		/* BH-C ^ */

		s1 = strlen(remotes);
		s2 = strlen(addr);

		/* can we cram one more address on line?  same host? */
		if( s1 > 0 && ((s1 + s2 + 1) > RMAILCMDSIZ || strcmp(thisremote, viaremote) != 0)) {
			/* dump it then, too bad */
			err &= sendone(argc, argv, remotes, TRUE);
			remotes[0] = '\0';
			GetSequenceNumber(); /* update seq # for new sending */
		}

		/* add *argvec to list of remotes */
		strcat(remotes, " ");
		strcat(remotes, addr);
		strcpy(viaremote, thisremote);
	}
	else {
		if (debuglevel > 5)
			fprintf(stderr, "pcmail: calling sendone %s\n", addr);
		err &= sendone(argc, argv, addr, FALSE);
	}
	return (err);
}

main(int argc, char **argv) {
	long int position;
	register int header = 1;
	register int amount;

	register int argcount;
	register char **argvec;
	int remote;
	int err = 1;
	DIR	*spoolDir;
#ifdef Upgrade
	int gotalias;
	char *bang, *atsign, *percent;
	char *addr, addrbuf[256];
#endif Upgrade

#ifndef NOMAIN
	/* get environment var's */
	err = 1;
	HOSTINIT;
	loadenv();

 	if (argc <= 1) {
		fprintf( stderr, "pcmail usage: pcmail addresses < message\n" );
		exit(1);
	}
	debuglevel = 1;

#endif
#ifdef RMAIL
	local = FALSE;
#else
	local = TRUE;
#endif

	if ( debuglevel > 5 ) {
		fprintf( stderr, "pcmail: argc %d ", argc );
		argcount = argc;
		argvec = argv;
		while (argcount--) {
			fprintf( stderr, " \"%s\"", *argvec++ );
			}
		fprintf( stderr, "\n" );

		tloc = time((time_t *)NULL );
		thetime = ctime(&tloc);
		fprintf( stderr, "thetime: %s\n",thetime );
	}

#ifdef MSDOS
 	mcurdir = getcwd( s_mcurdir, sizeof(s_mcurdir) );
#else
	mcurdir = getcwd( s_mcurdir, sizeof(s_mcurdir) );
#endif
	chdir( spooldir );
	/*
	spoolDir = opendir (spooldir);
	*/
	
	GetSequenceNumber();

	/* open a temporary file */
	/* sprintf( tfilename, TFILENAME, sequence ); */
 	sprintf( tfilename, TFILENAME, sequence );
 	mkfilename( miscbuff, tempdir, tfilename );
 	if (*tempdir != SEPCHAR)
 		mkfilename(tfilename, mcurdir, miscbuff);
 	else strcpy(tfilename, miscbuff);

 	if ( debuglevel > 5 )
 		fprintf( stderr, "pcmail: opening %s\n", tfilename );

	tempfile = FOPEN( tfilename, "w", 't' );
 	if (tempfile == (FILE *)NULL)  {
		fprintf( stderr, "pcmail: can't open %s\n", tfilename );
		exit(1);
	}

	/* copy stdin to tempfile, counting content lines and bytes */
	header = 1;
	while (fgets( buf, BUFSIZ - 1, stdin ) != (char *)NULL) {
		if (header != 0) {
			if (strlen( buf ) == 1) {
				header = 0;
				fprintf( tempfile, "\n" );
				continue;
			}
			/*
			else if (strchr( buf, ':' ) == NULL) {
				header = 0;
				fprintf( tempfile, "\n" );
			}
			*/
		}
		if (header == 0) {
			lines++;
			bytes += strlen( buf );
		}
		fputs( buf, tempfile );
#ifdef MULTIFINDER
		Check_Events(0);
#endif
	}
#ifndef RMAIL
	/* copy stdin to tempfile, counting content lines and bytes */
	/* get signature */
	mkfilename( miscbuff, home, signature );
	if (*home != SEPCHAR)
 		mkfilename(mfilename, mcurdir, miscbuff);
 	else strcpy(mfilename, miscbuff);

	if (debuglevel > 4)
 	   fprintf( stderr, "pcmail: opening sigfile %s\n", mfilename );
	mailfile = FOPEN( mfilename, "r", 't' );
	if (mailfile != (FILE *)NULL) {
		fputs( "\n--\n", tempfile );
		while (fgets( buf, BUFSIZ - 1, mailfile ) != (char *)NULL) {
			lines++;
			bytes += strlen( buf );
			fputs( buf, tempfile );
		}
		fclose( mailfile );
 	}
#endif

 	fclose( tempfile );

	if ( debuglevel > 4 ) {
		fprintf( stderr, "pcmail: stdin copied to tmp %ld %ld\n",
	                 bytes, lines );
 		fprintf( stderr, "pcmail: args %d\n", argc );
	}

	/* loop on args, copying to appropriate postbox,
	   do remote only once
	   remote checking is done empirically, could be better
	*/
	remotes[0] = '\0';


#ifndef RMAIL
	if ( strcmp( argv[1], "-s" ) == SAME ) {
		argv++;argv++;
		argc--;argc--;
		if ( argc == 0 )
			return( -1 );
		strcpy( Subject, *argv );
		}
#endif
	argcount = argc;
	argvec = argv;

	while (--argcount > 0) {
		argvec++;
		strcpy(addrbuf, *argvec);
		addr = addrbuf;
		if ( debuglevel > 5 )
			fprintf( stderr, "pcmail: arg# %d\ %s\n",
							argcount, *argvec );
/*
	Recognize and strip off our own nodename and domain name, whether by l@nd, s%, or sk!
*/

		atsign = strrchr(addr, '@');
		
		while (atsign != NULL && IUMagIDString(domain, atsign+1, strlen(domain), strlen(atsign+1)) == 0) {
			*atsign = '\0';
			atsign = strrchr(addr, '@');
		}
		
		bang = strchr(addr, '!');
	
		while (bang != NULL && (IUMagIDString(nodename, addr, strlen(nodename), bang-addr) == 0 ||
			                             IUMagIDString(domain, addr, strlen(domain), bang-addr)  == 0)) {
			addr = bang+1;
			bang = strchr(addr, '!');
		}
		
		percent = strrchr(addr, '%');
		
		while (percent != NULL && IUMagIDString(domain, percent+1, strlen(domain), strlen(percent+1)) == 0) {
			*percent = '\0';
			percent = strrchr(addr, '%');
		}
		/* get alias file */
		mkfilename(miscbuff, home, alias);
		if(*home != SEPCHAR)
	 		mkfilename(afilename, mcurdir, miscbuff);
	 	else
	 		strcpy(afilename, miscbuff);

		if(debuglevel > 4)
	 	   fprintf(stderr, "pcmail: opening alias file %s\n", afilename);
		gotalias = FALSE;
		if((aliasfile=FOPEN(afilename, "r", 't')) != (FILE *)NULL) {
			char str[BUFSIZ], *args[255];
			int i, nargs;
			char *theAlias, *newAlias;
#ifdef MULTIFINDER
			Handle aliasHandle;
			aliasHandle = NULL;
#endif
			theAlias = (char *) NULL;

			while(fgets(str, BUFSIZ - 1, aliasfile) != (char *)NULL && !feof(aliasfile)) {
#ifdef MULTIFINDER
				aliasHandle = NewHandle(strlen(str)+1);
				if (aliasHandle) {
					MoveHHi(aliasHandle);
					HLock(aliasHandle);
					theAlias = (char *) Strip((long int) *aliasHandle);
				} else {
					theAlias = (char *) NULL;
				}
#else
				theAlias = malloc(strlen(str) + 1);
#endif
				if (theAlias == (char *) NULL) {
					fprintf(stderr, "Can't get string buffer for alias!");
					break;
				}
				strcpy(theAlias, str);
				while(strlen(theAlias) > 0 && theAlias[strlen(theAlias)-1] <= ' ') {
					theAlias[strlen(theAlias)-1] = '\0';
				}
				while (theAlias[strlen(theAlias)-1] == '\\') {
					theAlias[strlen(theAlias)-1] = ' ';
					if (fgets(str, 255, aliasfile) != (char *)NULL && !feof(aliasfile)) {
#ifdef MULTIFINDER
						HUnlock(aliasHandle);
						SetHandleSize(aliasHandle, strlen(theAlias) + strlen(str) + 1);
						if (MemError() == noErr) {
							MoveHHi(aliasHandle);
							HLock(aliasHandle);
							newAlias = (char *) Strip((long int) *aliasHandle);
						} else {
							newAlias = (char *) NULL;
						}
#else
						newAlias = realloc(theAlias, strlen(theAlias) + strlen(str) + 1);
#endif
						if (newAlias == (char *) NULL) {
							fprintf(stderr, "Can't get string buffer for alias!");
							break;
						}
						theAlias = newAlias;
						strcat(theAlias, str);
						while(strlen(theAlias) > 0 && theAlias[strlen(theAlias)-1] <= ' ') {
							theAlias[strlen(theAlias)-1] = '\0';
						}
					}
				}
				if((nargs=getargs(theAlias, args)) > 2) {
					if(IUMagIDString(args[0], "alias", strlen(args[0])+1, 6) == 0 && 
						IUMagIDString(args[1], addr, strlen(args[1])+1, strlen(addr)+1) == 0) {
						gotalias = TRUE;
						for(i=2 ; i < nargs ; i++) {
							err &= sendit(argc, argv, args[i], remotes);
						}
#ifdef MULTIFINDER
						DisposHandle(aliasHandle);
#else
						free(theAlias);
#endif
						theAlias = (char *) NULL;
						break;
					}
				}
				if (theAlias) {
#ifdef MULTIFINDER
					DisposHandle(aliasHandle);
#else
					free(theAlias);
#endif
					theAlias = (char *) NULL;
				}
			}
			fclose(aliasfile);
		}
		if(gotalias == TRUE)
			continue;
		err &= sendit(argc, argv, *argvec, remotes);
	}
	/* dump remotes if necessary */
	if ( strlen( remotes ) > 0 )
		err &= sendone( argc, argv, remotes, TRUE );

#ifndef RMAIL

	if (strlen(mailcopy) > 0) {
		/* wants copy of mail kept */
		mkfilename( miscbuff, maildir, mailcopy );
		if (*maildir != SEPCHAR)
	 		mkfilename(mailsent, mcurdir, miscbuff);
	 	else strcpy(mailsent, miscbuff);
	
		if ( debuglevel > 4 )
		   fprintf( stderr, "pcmail: copfile = %s\n", mailsent );
		err &= sendone( argc, argv, mailsent, FALSE );
	}
#endif

	UNLINK( tfilename );
	chdir( mcurdir );
/*
	closedir(spoolDir);
*/
	return(err);
}

/*
	Get the current file-sequence number, and write out an updated one
*/
void GetSequenceNumber(void)
{
	FILE   *tempfile;
	char	miscbuff[255];
	char	tfilename[255];
	/* get sequence number */
	mkfilename( miscbuff, confdir, SFILENAME);
	if (*confdir != SEPCHAR)
		/* make it an absolute pathname */
		mkfilename(tfilename, mcurdir, miscbuff);
	else strcpy(tfilename, miscbuff);

 	if ( debuglevel > 4 )
 	   fprintf( stderr, "pcmail: opening %s\n", tfilename ); /* */
	tempfile = FOPEN( tfilename, "r", 't' );
	if (tempfile != (FILE *)NULL) {
		fscanf( tempfile, "%ld", &sequence );
		fclose( tempfile );
	}
 	else {
 		fprintf( stderr, "pcmail: can't find %s file, creating\n",
 					tfilename );
 		sequence = 1;	/* start at 1 */
 	};

	/* update sequence number */
	if ( debuglevel > 5 )
		fprintf( stderr, "pcmail: new sequence # %ld\n", sequence );

	tempfile = FOPEN( tfilename, "w", 't' );
	if (tempfile != (FILE *)NULL) {
		/* BH-C */
		if ( sequence > 9999 ) fprintf( tempfile, "%ld\n", 0 );
		else
		/* BH-C */
		fprintf( tempfile, "%ld\n", sequence+1 );
		fclose( tempfile );
	}
	/* sequence = sequence % 10000; BH-C */ /* Limit to four digits in filenames */
}


/* creates the chartime string */
dodate(){		/* BH-C */

	MachineLocation		mLoc;
	static char			*dw[7] = {"Sun","Mon","Tue","Wed",
							  	 "Thu","Fri","Sat"},
						*mth[12] = {"Jan","Feb","Mar","Apr",
								    "May","Jun","Jul","Aug",
								    "Sep","Oct","Nov","Dec"};
	struct tm				*timeinfo;
	char					tmpst[128];
	long					GMTDelta,
						hours;
	
	timeinfo = localtime(&tloc);
	sprintf(chartime, "%3.3s, %02.2d %3.3s %02.2d %02.2d:%02.2d:%02.2d",
				dw[timeinfo->tm_wday],
				timeinfo->tm_mday,
				mth[timeinfo->tm_mon],
				timeinfo->tm_year,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec);
				
	ReadLocation(&mLoc);
	GMTDelta = mLoc.gmtFlags.gmtDelta & 0x00ffffff;
	if ( (GMTDelta >> 23) & 1 )
		GMTDelta = GMTDelta | 0xff000000;
	GMTDelta = GMTDelta / 60;
	hours = GMTDelta / 60;
	GMTDelta = GMTDelta + (40 * hours);
	
	if ( GMTDelta < 0 ) {
		GMTDelta = -GMTDelta;
		sprintf(tmpst,"-%04.4ld",GMTDelta);
	} else
		sprintf(tmpst,"+%04.4ld",GMTDelta);
	
	if ( mLoc.gmtFlags.dlsDelta ) {
		if ( dlstimediff[0] > ' ' )
			sprintf(chartime, "%s %s",chartime,dlstimediff);
		else
			sprintf(chartime, "%s %s%s",chartime,tmpst,dlstimediff);
	} else {
		if ( timediff[0] > ' ' )
			sprintf(chartime, "%s %s",chartime,timediff);
		else
			sprintf(chartime, "%s %s%s",chartime,tmpst,timediff);
	}
}

char fpat1[] = "%c.%.7s9%04ld";
char fpat1j[] = "%c.%.7s%c%04ld";
char fpat2[] = "S %s %s %s - %s 0666 %s";


/* sendone copies file plus headers to appropriate postbox
   NB. we do headers here to allow flexibility later, for example
   in being able to do bcc, per host service processing etc.
*/
sendone(int argc, char **argv, char *address, int remote) {
	register char 	*cp;
/* BH-C
	struct tm		*timeinfo;
	static char		*dw[7] = {"Sun","Mon","Tue","Wed",
							  "Thu","Fri","Sat"},
					*mth[12] = {"Jan","Feb","Mar","Apr",
								"May","Jun","Jul","Aug",
								"Sep","Oct","Nov","Dec"};
*/
								
	char	icfilename[32];		/* local C. copy file */
	char	ixfilename[32];		/* local X. xqt file */
	char	idfilename[32];		/* local D. data file */
	char	rxfilename[32];		/* remote X. xqt file */
	char	rdfilename[32];		/* remote D. data file */
 	char	tmfilename[32];		/* temporary storage */
#if MSDOS
 	char	cixfilename[32];		/* canonical ixfilename */
 	char	cidfilename[32];		/* canonical idfilename */
#endif
	char forwardto[256];
	char *wherefrom;
	MachineLocation mLoc;			/* BH-C */

 	if ( remote ) {
 		/* sprintf all required file names */
		sprintf( tmfilename, fpat1, 'C', viaremote,  sequence );
		importpath( icfilename, tmfilename );
#ifdef MSDOS
 		sprintf( cidfilename, fpat1, 'D', viaremote, sequence );
		importpath( idfilename, cidfilename );
		sprintf( cixfilename, fpat1, 'D', nodename, sequence );
 		importpath( ixfilename, cixfilename );
#else
		sprintf( tmfilename, fpat1, 'D', viaremote, sequence );
		importpath( idfilename, tmfilename );
		sprintf( tmfilename, fpat1, 'D', nodename, sequence );
		importpath( ixfilename, tmfilename );
#endif
		sprintf( rdfilename, fpat1j, 'D', nodename, 'B', sequence );
		sprintf( rxfilename, fpat1j, 'X', nodename, 'A', sequence );
 	}
 	else {
		/* postbox file name */
 		if ( index( address, SEPCHAR ) == (char *)NULL )
			/* BH-C v */
 			if ( mbox[0] ) {
 				sprintf(idfilename,"%s:%s",maildir,address);
 				mkfilename( idfilename, idfilename, mbox );
 			} else
 			/* BH-C ^ */
			mkfilename( idfilename, maildir, address );
		else
			strcpy( idfilename, address );
	}

	if ( debuglevel > 5 )
 		fprintf( stderr, "pcmail: sendone: %s\n", idfilename );

 	if ( remote == FALSE ) {
 	   if ( debuglevel > 5 )
 	      fprintf( stderr, "pcmail: sendone: check for remote\n" );
		/* check for forwarding */
		if ( (mailfile = FOPEN( idfilename, "r", 't' )) != (FILE *)NULL ) {
			cp = fgets( buf, BUFSIZ - 1, mailfile );
			fclose( mailfile );
			if (cp != (char *)NULL)
				if (strncmp( buf, FORWARD, 10 ) == 0) {
					strcpy( forwardto, buf+11 );
					if (strchr(forwardto, '!') != NULL || strchr(buf, '@') != NULL) {
						return( sendone( argc, argv, forwardto, TRUE ) );
					} else {
						return( sendone( argc, argv, forwardto, FALSE ) );
					} 
				}
		}
	}

	/* open mailfile */
	if ( (mailfile = FOPEN( idfilename, "a", remote?'b':'t' )) == (FILE *)NULL ) {
		fprintf( stdout, "pcmail: cannot append to %s\n", idfilename );
		return( 0 );
	}

	if ( debuglevel > 5 )
		fprintf( stderr, "pcmail: append to mailfile\n" );

	tloc = time( (time_t *)NULL );
	thetime = ctime(&tloc);
 	(void)strcpy(chartime, thetime);	/* make our own copy */
 	thetime = chartime;	/* and work with our own copy */
	thetime[strlen(thetime)-1] = '\0';
	/* timeinfo = localtime(&tloc);		BH-C */
	
#ifdef RMAIL
	tempfile = FOPEN( tfilename, "r", 't' );
	if ( tempfile == (FILE *)NULL) {
		fprintf( stdout, "pcmail: can't re-open %s\n", tfilename );
		return( 0 );
	}
	fgets( buf, BUFSIZ - 1, tempfile );
	if ( strncmp( buf, "From ", 5 ) == 0 ) {
		sscanf(buf, "From %s ", miscbuff);
/*		printf("From [%s]\n", miscbuff); */
	}
	if (strlen(gotitfrom) > 0) {
		wherefrom = gotitfrom;
	} else {
		wherefrom = mailserv;
	}
	if (strchr(miscbuff, '@') != NULL) {
		fprintf(mailfile, "From %s %s", miscbuff, thetime);
	} else {
		fprintf(mailfile, "From %s!%s %s", wherefrom, miscbuff, thetime );
	}
	fputc( '\012', mailfile );
	fprintf( mailfile, "Received: from %s by %s with UUPC;", wherefrom, domain);
	fputc( '\012', mailfile );
	
	dodate();						/* BH-C */
	
/* BH-C	
	sprintf(chartime, "%3.3s, %02.2d %3.3s %02.2d %02.2d:%02.2d:%02.2d %s",
				dw[timeinfo->tm_wday],
				timeinfo->tm_mday,
				mth[timeinfo->tm_mon],
				timeinfo->tm_year,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec,
				timediff);
*/
	fprintf( mailfile, "          %s", thetime);
	fputc( '\012', mailfile );
#else /* RMAIL */
	fprintf( mailfile, "From %s %s", mailbox, thetime );
	if ( remote )
		fprintf( mailfile, " remote from %s", nodename );
	fputc( '\012', mailfile );
	fprintf( mailfile, "Received: by %s (pcmail);", domain);
	fputc( '\012', mailfile );
	
	dodate();						/* BH-C */
	
/* BH-C	
	sprintf(chartime, "%3.3s, %02.2d %3.3s %02.2d %02.2d:%02.2d:%02.2d %s",
				dw[timeinfo->tm_wday],
				timeinfo->tm_mday,
				mth[timeinfo->tm_mon],
				timeinfo->tm_year,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec,
				timediff);
*/
	fprintf( mailfile, "          %s", thetime);
	fputc( '\012', mailfile );
	fprintf( mailfile, "Date: %s", thetime);
	fputc( '\012', mailfile );
	/* add Date:, From: and Message-ID: headers */
	fprintf( mailfile, "From: %s <%s@%s>", name, mailbox, domain );
	fputc( '\012', mailfile );
	fprintf( mailfile, "Message-Id: <%ld@%s>", sequence, domain );
	fputc( '\012', mailfile );
	
	/* add To: headers */
	fprintf(mailfile, "To:");
	while(--argc > 0)
		fprintf(mailfile, " %s", *++argv);
	fputc('\012', mailfile);	/* better Ken? */

	if ( strlen( Subject ) > 0 ) {
		fprintf( mailfile, "Subject: %s", Subject );
		fputc( '\012', mailfile );
	}
#ifdef PCMAIL
	/* add Message-Lines: and Content-Length: headers */
	fprintf( mailfile, "X-Message-Lines: %ld", lines );
	fputc( '\012', mailfile );
	fprintf( mailfile, "Content-Length: %ld", bytes );
	fputc( '\012', mailfile );
#endif /* PCMAIL */
	/* copy tempfile to postbox file */
	tempfile = FOPEN( tfilename, "r", 't' );
	if ( tempfile == (FILE *)NULL) {
		fprintf( stdout, "pcmail: can't re-open %s\n", tfilename );
		return( 0 );
	}
#endif /* RMAIL */
	if (debuglevel > 4)
	   fprintf( stderr, "pcmail: copy tempfile %s to %s\n",
	                    tfilename, idfilename );

	while (fgets( buf, BUFSIZ - 1, tempfile ) != (char *)NULL) {
		if ( strncmp( buf, "From ", 5 ) == 0 )
			fputc( '>', mailfile );
		cp = &buf[ strlen(buf)-1 ];
		if ( *cp == '\n' )
			*cp = '\0';
		fputs( buf, mailfile );
		fputc( '\012', mailfile );
 	}
	if (!remote) fputc( '\012', mailfile );

	/* close files */
	fclose( mailfile );
	fclose( tempfile );

	/* all done unless going to remote via uucp */
	/* must create the job control files */
	if ( remote == TRUE ) {

		/* create remote X xqt file */
		mailfile = FOPEN( ixfilename, "w", 'b' );
		if 	(mailfile == (FILE *)NULL) {
			fprintf( stdout, "pcmail: cannot append to %s\n", ixfilename );
			return( 0 );
		}
		fprintf( mailfile, "U %s %s", uucp, nodename );
		fputc( '\012', mailfile );
		fprintf( mailfile, "F %s", rdfilename );
		fputc( '\012', mailfile );
		fprintf( mailfile, "I %s", rdfilename );
		fputc( '\012', mailfile );
		fixaddress(address+1);
		fprintf( mailfile, "C rmail %s", address );
		fputc( '\012', mailfile );
		fclose( mailfile );

		/* create local C copy file */
		mailfile = FOPEN( icfilename, "w", 't' );
		if 	(mailfile == (FILE *)NULL) {
			fprintf( stdout, "pcmail: cannot append to %s\n", icfilename );
			return( 0 );
		}

#if MSDOS
 		fprintf( mailfile, fpat2, cidfilename, rdfilename,
 					uucp, cidfilename, uucp );
 		fputc( '\012', mailfile );
 		fprintf( mailfile, fpat2, cixfilename, rxfilename,
 					uucp, cixfilename, uucp );
 		fputc( '\012', mailfile );
#else
		fprintf( mailfile, fpat2, idfilename, rdfilename,
					uucp, idfilename, uucp );
		fputc( '\012', mailfile );
		fprintf( mailfile, fpat2, ixfilename, rxfilename,
					uucp, ixfilename, uucp );
		fputc( '\012', mailfile );
#endif
		fclose( mailfile );

	} /* if ( remote == TRUE ) */

	return( 1 );
}

/* Get address into percent form for rmail, and delete mailhost name */
fixaddress(char *address)
{
	char		temp[BUFSIZ];
	register	char	*p, *q, *r;
	
	q = address;
	temp[0] = '\0';
	while ( (p = index(q, ' ')) != NULL ) {
		*p = '\0';
		fixpercent(q);
#ifdef	PERCENT_ADDRS
		if ((r = strrchr(q, '@')) != NULL) *r = '%';
		if ((r = strrchr(q, '%')) != NULL ) {
			if (strcmp(r+1, mailserv) == 0) *r = '\0';
			if ((r = strrchr(q, '%')) != NULL) *r ='@';
		}
#else
		if ((r = strchr(q, '!')) != NULL) {
			if (strncmp(q, mailserv, (int)(r - q)) == 0)
				q = r + 1;
		}
#endif	PERCENT_ADDRS
		strcat(temp, q);
		strcat(temp, " ");
		q = ++p;
	}
	/* do the last one */
	fixpercent(q);
#ifdef	PERCENT_ADDRS
	if ((r = strrchr(q, '@')) != NULL) *r = '%';
	if ((r = strrchr(q, '%')) != NULL ) {
		if (strcmp(r+1, mailserv) == 0) *r = '\0';
		else if (strncmp(r+1, mailserv, strlen(mailserv)) == 0 &&
		            strcmp(r+strlen(mailserv)+1, ".uucp") == 0) *r = '\0';
		if ((r = strrchr(q, '%')) != NULL) *r ='@';
	}
#else
		if ((r = strchr(q, '!')) != NULL) {
			if (strncmp(q, mailserv, (int)(r - q)) == 0)
				q = r + 1;
		}
#endif	PERCENT_ADDRS
	strcat(temp, q);
	strcpy(address, temp);
}

#ifdef	PERCENT_ADDRS
/*	converts addresses to RFC 822 percent form
	%,@ takes precedence over ! e.g.,
	a!b!c%y@z -> c%b.uucp%a.uucp%y@z
*/
fixpercent(char *addr)
{
	char	front[BUFSIZ], back[BUFSIZ];
	register char	*p;

	if ( (p = strrchr(addr, '%')) != NULL ||
		 (p = strrchr(addr, '@')) != NULL) {

		*p = '\0';
		if (strchr(addr, '!') != NULL) fixpercent(addr);
		if (strchr(p+1, '!') != NULL) fixpercent(p+1);
		*p = '%';
	}
	else if ( (p = strchr(addr, '!')) != NULL) {
		*p++ = '\0';
		strcpy(front, addr);
		strcpy(back, p);
		fixpercent(back);
		strcpy(addr, back);
		strcat(addr, "%");
		strcat(addr, front);
		if (strchr(front, '.') == NULL) {
			strcat(addr, ".uucp");
		}
	}
}
#else
/*	converts addresses to uucp ! form
	%,@ takes precedence over ! e.g.,
	a!b!c%y@z -> z!y!a!b!c
*/
fixpercent(char *addr)
{
	char	front[BUFSIZ];
	register char	*p;

	if ( (p = strrchr(addr, '@')) != NULL ||
		 (p = strrchr(addr, '%')) != NULL) {
		*p = '\0';
		strcpy(front, p+1);
		strcat(front, "!");
		fixpercent(addr);
		strcat(front, addr);
		strcpy(addr, front);
	}
}
#endif	PERCENT_ADDRS

#ifndef AMIGA
#ifdef RMAIL
rnews(int argc, char *argv[])
{
	struct tm	*thetm;
	char	filename[132];
	char	format[128];
	FILE 	*f;
	char	buf[BUFSIZ];

	static int count = 0;
	int gotBytes;

	tloc = time( (time_t *)NULL );
	thetime = ctime(&tloc);
	tloc = time( (time_t *)NULL );

	thetm = localtime( &tloc );

	/* mkfilename( format, spooldir, NEWSDIR );	*/
	sprintf( filename, NEWSDIR,
		thetm->tm_year % 100, thetm->tm_mon+1,
		thetm->tm_mday, thetm->tm_hour,
		thetm->tm_min,  thetm->tm_sec,  count
		);

	count++;

	if ( debuglevel > 5 )
		fprintf( stderr, "rnews: %s\n", filename );

	if ( (f = FOPEN( filename, "w", 'b' )) == (FILE *)NULL ) {
		fprintf( stderr, "rnews: can't open %s %d\n", filename, errno );
		return( 0 );
	}

	while ( (gotBytes = fread( buf, 1, BUFSIZ, stdin)) != 0) {
		fwrite( buf, 1, gotBytes, f );
#ifdef MULTIFINDER
		Check_Events(0);
#endif
	}

	fclose( f );
	return (1);
}

#endif /* RMAIL */
#endif /* AMIGA */

