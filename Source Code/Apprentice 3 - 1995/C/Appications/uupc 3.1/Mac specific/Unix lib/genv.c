/* on */
#define Upgrade		/* for alias stuff */
/* 		genv.c
 
 
 
		copyright (C) 1987 Stuart Lynne
 
		Copying and use of this program are controlled by the terms of the
		Free Software Foundations GNU Emacs General Public License.
 
 
		version		0.1		March 31/1987
 
 05mar90 - increased length of char arrays holding profile information to handle
 		   longer file names.  [garym]
 07apr90 - put profile file in System Folder and changed name to "UUPC Settings"
 
*/
 
#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#ifdef UNIX
#include <sys/types.h>
#endif
#ifdef slmacaztec
#include <mac/types.h>
#endif
 
#include <stdio.h>
#include <string.h>
 
#include "genv.h"

#define DMAILBOX	"root"
#define	DNAME		"System Manager"
#define DHOME		"root"
#define	DDOMAIN		"mac.uucp"
#define	DMAILDIR	"/mail"
#define	DCONFDIR	"/mail/spool"
#define	DSPOOLDIR	"/mail/spool"
#define DPUBDIR		"/mail/spool/uucppublic"
#define	DMAILSERVICE	"uunet"
#define	DNODENAME	"mac"
#define	DDEVICE		"modem"
#define	DSPEED		"2400"
#define	DTEMPDIR	"/tmp"
#define DMAILCOPY	"mail.sent"
#define DSIGNATURE	".signature"
#define DTIMEDIFF	"GMT"
#define DDTIMEDIFF	""			/* BH-C */
#define DROUTEVIA	""
#define DSHORTNAME	""
#ifdef Upgrade
# define DALIAS		".alias"
# define DPASSWORD	"uucp"
# define DSLEEPTIME "5"
#endif Upgrade
#define DMBOX		""			/* BH-C */
 
#define	TFILENAME	"tmpfile"
#define	FILENAME 	"%s/%s"

void genv();

extern SysEnvRec SysEnv;
extern int settingsFileVRefNum;
extern long int settingsFileDirID;

#ifndef THINK_C

#include "genv.proto.h"
char *getenv();
 
 
char	rmailbox[64];
char	rname[64];
char	rhome[64];
char	rdomain[64];
char	rmaildir[64];
char	rconfdir[64];
char	rspooldir[64];
char	rpubdir[64];
char	rmailserv[16];
char 	rnodename[16];
char 	rdevice[16];
char 	rspeed[8];
char 	rphone[64];
char	rtempdir[64];
char	rroutevia[512];
char	rshortname[16];
char	rpassword[64];
char	rsleeptime[8];
char	rmbox[16];				/* BH-C */

char	*mailbox = rmailbox;
char	*name = rname;
char	*home = rhome;
char	*domain = rdomain;
char	*maildir = rmaildir;
char	*confdir = rconfdir;
char	*spooldir = rspooldir;
char	*pubdir = rpubdir;
char	*mailserv = rmailserv;
char 	*nodename = rnodename;
char 	*device = rdevice;
char 	*speed = rspeed;
char 	*phone = rphone;
char	*tempdir = rtempdir;
char	*shortname = rshortname;
char	*password = rpassword;
char	*sleeptime = rsleeptime;
char	*mbox = rmbox;			/* BH-C */

void genv(char **thename, char *envname, char *dflt)
{
	char * temp;
	
	if ((temp = getenv( envname )) == NULL) {
		/* fprintf( stderr, "genv: %s not found, using %s\n", envname, dflt ); /* */
		temp = dflt;
	}
	strcpy( *thename, temp );
	/*  fprintf( stderr, "genv: %s %s\n", envname, *thename ); /**/
}
#else

char	mailbox[80];
char	name[80];
char	home[80];
char	domain[80];
char	maildir[80];
char	confdir[80];
char	spooldir[80];
char	pubdir[80];
char	mailserv[16];
char 	nodename[16];
char 	device[128];
char 	speed[128];
char 	phone[128];
char	tempdir[80];
char	timediff[16];
char	dlstimediff[16];		/* BH-C */
char	signature[80];
char	mailcopy[80];
char	routevia[512];
char	shortname[16];
#ifdef Upgrade
char	alias[80];
char	password[64];
char	sleeptime[8];
#endif Upgrade
char	mbox[16];				/* BH-C */

int		settingsFileRefNum;

/* off
#define DEBUG /* */

/* use the resource mgr for now; profile opened in init code */
void genv(char *thename, char *envname, char *dflt) {
	Handle	strH;
	Str255	envbuf;
	
	strcpy((char *)envbuf, envname);
	CtoPstr((char *)envbuf);	/* convert to Pascal string */
	if ((strH = GetNamedResource( 'STR ', envbuf )) == NULL) {
/* 		fprintf( stderr, "genv: %s not found, using %s\n", envname, dflt ); */
		strcpy(thename, dflt);
	}
	else {
		if (*strH == 0) {
			fprintf(stderr, "genv: %s is a null string\n", envname);
		}
		BlockMove((*strH)+1, thename, (Size)(**strH));
		*(thename + (int)(**strH)) = '\0';
		/* copy and convert to C string */
	}
# ifdef DONTDOTHIS
	fprintf( stderr, "genv: %s %s\n", envname, *thename );
# endif DONTDOTHIS
}
#endif

void loadenv(void)
{
	int i;
	OSErr osErr;
	FCBPBRec fcbPB;
	Str255 rfName;
	
	if ((i = OpenRFPerm("\pUUPC Settings", 0, fsRdPerm)) == -1 &&
		(i = OpenRFPerm("\pUUPC Settings", SysEnv.sysVRefNum, fsRdPerm)) == -1) {
		fprintf( stderr, "ERROR: Can't find \"UUPC Settings\" file here or in System Folder\r" );
		return;
	}
	
	fcbPB.ioNamePtr = rfName;
	fcbPB.ioVRefNum = 0;
	fcbPB.ioFCBIndx = 0;
	fcbPB.ioRefNum  = i;
	
	if ((osErr = PBGetFCBInfo(&fcbPB, FALSE)) != noErr) {
		fprintf( stderr, "ERROR: Can't pin down location of \"UUPC Settings\" file\r" );
	}
	
	settingsFileVRefNum = fcbPB.ioFCBVRefNum;
	settingsFileDirID   = fcbPB.ioFCBParID;
	
	settingsFileRefNum = i;
	
	/* get environment var's */
	genv( name, NAME, DNAME );
	genv( mailbox, MAILBOX, DMAILBOX );
	genv( home, HOME, DHOME );
	genv( domain, DOMAIN, DDOMAIN );
	genv( maildir, MAILDIR, DMAILDIR );
	genv( confdir, CONFDIR, DCONFDIR );
	genv( spooldir, SPOOLDIR, DSPOOLDIR );
	genv( pubdir, PUBDIR, DPUBDIR );
	genv( mailserv, MAILSERVICE, DMAILSERVICE );
	genv( nodename, NODENAME, DNODENAME );
	genv( device, DEVICE, DDEVICE );
	genv( speed, SPEED, DSPEED );
	genv( tempdir, TEMPDIR, DTEMPDIR );
	genv( timediff, TIMEDIFF, DTIMEDIFF);
	genv( dlstimediff, DLSTIMEDIFF, DDTIMEDIFF);	/* BH-C */
	genv( mailcopy, MAILCOPY, DMAILCOPY);
	genv( signature, SIGNATURE, DSIGNATURE);
	genv( routevia, ROUTEVIA, DROUTEVIA);
	genv( shortname, SHORTNAME, DSHORTNAME);
#ifdef Upgrade
	genv( alias, ALIAS, DALIAS);
	genv( password, PASSWORD, DPASSWORD);
	genv( sleeptime, SLEEPTIME, DSLEEPTIME);
#endif Upgrade
	genv( mbox, MBOX, DMBOX);			/* BH-C */

	CloseResFile(i);	/* so we can run pcmail while uupc is running in background */
	
	if (strlen(shortname) == 0) {
		strcpy(shortname, nodename);
	}
}
 
void mkfilename(char *filename, char *dirname, char *name)
{
	if (strchr(name, ':') != NULL) {
		if (*name == ':') {
			sprintf(filename, "%s%s", dirname, name );
		} else {
		    strcpy(filename, name);
		}
	}
	else sprintf( filename, FILENAME, dirname, name );	
	/* fprintf( stderr, "New filename %s\n", filename );  /**/	
}


#ifdef TEST
main(void)
{
	loadenv();
}
#endif

