/* 		genv.c
 
 
 
		copyright (C) 1987 Stuart Lynne
 
		Copying and use of this program are controlled by the terms of the
		Free Software Foundations GNU Emacs General Public License.
 
 
		version		0.1		March 31/1987
 
	
 
environment variables
 
	The following evironment variables are defined:
 
		MAILBOX		current user's mailbox, 	"sl"
		NAME		current user's name, 		"Stuart Lynne"
		HOME		current user's home dir		"/usr/sl"
		DOMAIN		domain of this machine, 	"mac.van-bc.can"
		MAILDIR		where mail is kept,			"/usr/mail"
		CONFDIR		where config data is  kept,	"/usr/lib/uucp"
		SPOOLDIR	where spooled files are 	"/usr/spool/uucp"
		PUBDIR		public spool directory		"/usr/spool/uucppublic"
 		MAILSERVICE	who do we send remote mail	"van-bc"
 		NODENAME	what is our local nodename	"slmac"
 		DEVICE		what is the default device	".a"
 		SPEED		what is the default speed	"1200"
 		TEMPDIR		what is temp directory		"/tmp"
 		TIMEDIFF	time difference from GMT	"(+0 GMT)"
 		DLSTIMEDIFF	dls time difference from GMT"(+1 GMT)"		BH-C
 		ROUTEVIA	list of neighbors			"uunet,uupsi,ucbvax"
 		SHORTNAME	alternate shorter nodename	"slm"
 		PASSWORD	inbound-call password		"uucp"
 		SLEEPTIME	minutes between wakeups		"5"
		MBOX		file where mail is kept		""				BH-C
 
*/
 

#define	MAILBOX		"MAILBOX"
#define	NAME		"NAME"
#define	HOME		"HOME"
#define	DOMAIN		"DOMAIN"

#define	MAILDIR		"MAILDIR"
#define	CONFDIR		"CONFDIR"
#define SPOOLDIR	"SPOOLDIR"
#define	PUBDIR		"PUBDIR"
#define	MAILSERVICE	"MAILSERVICE"
#define MAILCOPY	"MAILCOPY"
#define SIGNATURE	"SIGNATURE"
#define ROUTEVIA	"ROUTEVIA"
#define SHORTNAME	"SHORTNAME"
#ifdef Upgrade
#define ALIAS		"ALIAS"
#define PASSWORD	"PASSWORD"
#define SLEEPTIME	"SLEEPTIME"
#endif Upgrade
#define	MBOX		"MBOX"			/* BH-C */

#define NODENAME	"NODENAME"
#define	DEVICE		"DEVICE"
#define	SPEED		"SPEED"

#define	TEMPDIR		"TEMPDIR"
#define	TIMEDIFF	"TIMEDIFF"
#define	DLSTIMEDIFF	"DLSTIMEDIFF"	/* BH-C */
 

 
extern char	mailbox[];
extern char	name[];
extern char home[];
extern char	domain[];
extern char	maildir[];
extern char	confdir[];
extern char spooldir[];
extern char pubdir[];
extern char	mailserv[];
extern char nodename[];
extern char device[];
extern char speed[];
extern char phone[];
extern char tempdir[];
extern char	timediff[];
extern char	dlstimediff[];		/* BH-C */
extern char	signature[];
extern char	mailcopy[];
extern char routevia[];
extern char shortname[];
#ifdef Upgrade
extern char alias[];
extern char password[];
extern char sleeptime[];
#endif Upgrade
extern char mbox[];				/* BH-C */

 
void loadenv();
void genv();
void mkfilename();

