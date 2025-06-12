/*
 * @(#)uucp.h 1.9 87/09/29	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 *
 * Header file for gnuucp.
 *
 * Pieces that look like they might be taken from Unix uucp are
 * cribbed with the aid of public domain uucp modules (like the F-protocol)
 * that use that interface.
 */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define Ifn	FOO	/* file descriptor of the "phone line" */

#define	ASSERT(cond, msg, moremsg, huh) /**/
#define	prefix(s, l)	\
		(strncmp(s, l, strlen(s)) == SAME)

#define	DEBUG(level, msg, moremsg) \
	{if ((long)debug >= (long)level) printf(msg, moremsg);}
#define	DEBUG_LEVEL(level) \
	   ((long)debug >= (long)level)

/* I may have these flipped from what Unix uses... */
/* BUT: my code depends on these values in if (foo()) statements, sigh */
#define	SUCCESS	0
#define	FAIL	1

/* #define	MAXMSGLEN	((NAMESIZE*4)+SLOP) */	/* ?>?? FIXME */
#define	MAXMSGLEN	(4096+SLOP)	/* ?>?? FIXME */
#define	MAXMSGTIME	60	/* Timeout period for rdmsg */
#define	MAX_HOST	20	/* Host name length (uucp does 7) */
#define	MAXBASENAME	7	/* Host name length in queue file names */
#define	SYSNSIZE	MAXBASENAME	/* uuq uses */
#define	CMDPRE		'C'		/* Prefix on command files */
#define	X_LOCK		"XQT"	/* Lock name for uuxqt runs */
#define	MAX_LSYS	500	/* Max length of an L.sys line in chars */
#define	MAX_CTLLINE	100	/* Max length of a usenet.ctl line */

#define	CTL_DELIM	" \t\n\r"	/* Delimiters for usenet.ctl */
#define FILE_DELIM "\t\n\r" /* Don't use up space when we parse a filename */

extern int debug;			/* Debugging level */

/*
 * Timeout for raw characters -- if we don't hear a char within BYTE_TIMEOUT
 * seconds, we assume the other side has gone away.  Has nothing to do with
 * retransmission timeouts (if any!).
 */
#define	BYTE_TIMEOUT	15	/* seconds */
#define CONNECT_WAIT 15
#define	ONEDAY		(24*60*60)	/* seconds */

/* Misc stuff */
#define	SAME		0		/* if (strcmp(a,b) == SAME) ... */
#define	SLOP		10		/* Slop space on arrays */

/* STST system status file codes */
#define	SS_WRONGTIME	99		/* Wrong time to call -- not used */
#define	SS_INPROGRESS	3		/* Talking */
#define SS_FAIL		4		/* Dial failed */
#define	MAXRECALLS	20		/* Max # of recalls if STST there */

/*
 * Definition of serial ports that we can call out on.
 */
#define	PORTNAME_SIZE	20		/* Arbitrary */
struct port {
	char	portname[PORTNAME_SIZE]; /* Name of port type, e.g. ACU */
	char	modemname[PORTNAME_SIZE]; /* Name of dialer type, e.g. hayes */
	char	devname [PORTNAME_SIZE]; /* Name of device, e.g. cua0, com1 */
	long	baud;			/* A baud rate that works */
	struct port *chain;		/* Chain to next entry */
};

/* Variables that uucp uses for various things */
extern char	Progname[NAMESIZE];			/* Our program name e.g. "uuq" */
extern char	*Spool;			/* Our spool directory */
extern char *Alias;			/* Directory for aliases */
extern char	*uuControl;		/* Our control file */
extern char	Myname[MAX_HOST];	/* Our host name */
extern char	Mynamealias[MAX_HOST];	/* Our host name */
extern char	host_name[MAX_HOST];	/* Other guy's host name */
extern char Forwarder[MAX_HOST]; /*Default forwarder for mail not on this host */
extern int	ourpid;			/* Our process ID */
extern char	*Sysfile;		/* L.sys file */
extern char	*Userfile;		/* USERFILE */
extern char *Mail;			/* Our mail directory */
extern char *Loginfile;		/* File to log logins to our machine */
extern char	*Pubdir;		/* Public Directory */
extern char *UUCPlogname;   /* Login name for remote UUCP's */
extern char *UUCPpasswd;	/* password for remote UUCP's */
extern struct port *ports;		/* List of serial ports */
extern char	who[NAMESIZE];		/* User who sent/requested a file */
extern int	logfd;			/* File descriptor of log file */
extern long ByteTimeout;    /* Max time (in Secs that xgetc waits for a char */
extern long ConnectWait;
extern long PacketSize;
extern char *Timezone;
extern Boolean AppendLogfile;
extern char *DialPrefix;
extern char *DialPostfix;