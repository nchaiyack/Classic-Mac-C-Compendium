/*		mac.c

		macintosh host 

*/

#include <stdio.h>
#include <stdarg.h>
#include "host.h"
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

# ifdef THINK_C
# include <console.h>
#endif THINK_C

#define	MAIN	mailmain

char *curdir;
char * getcwd();

int CHDIR();
extern MAIN (int, char **);
int	debuglevel;		/* debugging level */

SysEnvRec SysEnv;
int settingsFileVRefNum;
long int settingsFileDirID;

#ifdef Upgrade
long int systimeout = 0;
#endif Upgrade

#ifdef STANDALONE
char *flds[10];
char line[128];

#define DMAILBOX	"mailbox"
#define	DNAME		""
#define DHOME		"/usr/home/guest"
#define	DDOMAIN		"mac.uucp"
#define	DMAILDIR	"/usr/spool/mail"
#define	DCONFDIR	"/usr/lib/uucp"
#define	DSPOOLDIR	"/usr/spool/uucp"
#define DPUBDIR		"/usr/spool/uucppublic"
#define	DMAILSERVICE	"crash"
#define	DNODENAME	"shappy"
#define	DDEVICE		"modem"
#define	DSPEED		"2400"
#define	DTEMPDIR	"/usr/tmp"

#include "host.proto.h"

FILE *fe;

SysEnvRec SysEnv;
int settingsFileVRefNum;
long int settingsFileDirID;

void sgenv(char **thename, char *envname, char *dflt)
{
	char line[BUFSIZ];

	fprintf( stderr, "genv: %s %s\n", envname, dflt ); /**/
	
	if (fgets( line, BUFSIZ, fe ) == NULL) {
		fprintf( stderr, "genv: %s not found, using %s\n", envname, dflt ); /* */
		strcpy( *thename, dflt);
	}
	else {
		strcpy( *thename, line);
		fprintf( stderr, "genv: %s %s\n", envname, *thename ); /**/
	}
}

void sloadenv(void)
{
	fprintf( stderr, "sloadenv\n" );
	if ( (fe = FOPEN( "/usr/lib/uucp/defaults", "r" )) == (FILE *) NULL ) {
		fprintf( stderr, "can't open /usr/lib/uucp/defaults  %d\n", errno );
		exit( -1 );
	}
	fprintf( stderr, "sloadenv - fopened\n" );
	/* get environment var's */
	sgenv( &name, NAME, DNAME );
	sgenv( &mailbox, MAILBOX, DMAILBOX );
	sgenv( &nodename, NODENAME, DNODENAME );
	sgenv( &home, HOME, DHOME );
	sgenv( &domain, DOMAIN, DDOMAIN );
	sgenv( &maildir, MAILDIR, DMAILDIR );
	sgenv( &confdir, CONFDIR, DCONFDIR );
	sgenv( &spooldir, SPOOLDIR, DSPOOLDIR );
	sgenv( &pubdir, PUBDIR, DPUBDIR );
	sgenv( &mailserv, MAILSERVICE, DMAILSERVICE );
	sgenv( &device, DEVICE, DDEVICE );
	sgenv( &speed, SPEED, DSPEED );
	sgenv( &tempdir, TEMPDIR, DTEMPDIR );
}
#endif

jmp_buf	dcpexit;

char *gets();

main(int argc, char **argv) {
	char volname[255];
	int vrefnum;

	/* macinit() has to happen before ccomand in THINK_C cos ccomand
	 * may change the current folder through redirected input comming from
	 * a file that is not in the pcmail application folder. So open
	 * resource file first
	 */
	macinit();
	
	SysEnvirons(1, &SysEnv);

#ifdef THINK_C
	GetVol(volname, &vrefnum);
# ifdef IMMEDEXIT
	console_options.pause_atexit = (short)0;
# endif IMMEDEXIT
	argc = ccommand(&argv);
	SetVol(volname, vrefnum);
#endif THINK_C

#ifdef STANDALONE
	fprintf( stderr, "Please enter parameters: " );
	if ( gets( line ) == (char *) NULL )
		exit( -1 );
	getargs( line, flds );
	argv = flds;
	/* mac specific prolog */
	sloadenv();
#else
	/* mac specific prolog */
	loadenv();
#endif
	
	
	curdir = getcwd( NULL, 0 );

#ifdef CWDSPOOL
	CHDIR( spooldir );
#endif

	/* setup longjmp for error exit's */
	if ( setjmp( dcpexit ) == 0 ) {

		MAIN( argc, argv );

	}
	

	/* mac specific epilog */
	chdir( curdir );
	exit(0);
}

/**/
/*
 *  p r i n t m s g
 *
 *  Print error message on standard output if not remote.
 */
/*VARARGS1*/
printmsg(int level, char *fmt, ...)
{
	char	msg[BUFSIZ*2];
	va_list ap;
	if (level == 0) {
		va_start(ap, fmt);
		vsprintf( msg, fmt, ap );
		va_end(ap);
		fputs(msg, stdout);
		fputc('\n', stdout);
	}
}

