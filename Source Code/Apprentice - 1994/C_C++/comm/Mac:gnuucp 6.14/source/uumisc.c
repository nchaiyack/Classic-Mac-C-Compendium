/*
 * @(#)uumisc.c 1.6 87/09/29	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 *
 * uumisc.c
 *
 * Miscellaneous uucp variables and routines which are used by several
 * uucp main programs.
 */
#include "includes.h"		/* System include files, system dependent */
#include "uucp.h"		/* Uucp definitions and parameters */

char	Myname[MAX_HOST];	/* Our uucp hostname, set from usenet.ctl */
char	Mynamealias[MAX_HOST];	/* Our uucp hostname, set from usenet.ctl */
char    Forwarder[MAX_HOST];
char	host_name[MAX_HOST] = "gnuucp";	/* Other guy's host name */
char	*uuControl;		/* Control file name */
char	who[NAMESIZE] = "-";	/* Who sent us a file */
char	*Spool;			/* Spool directory root name */
char	*Alias;			/* Directory where aliases are kept */
char	*Loginfile;	    /* File to log calls to us */
char	*Sysfile;		/* L.sys file */
char	*Userfile;		/* USERFILE */
char	*Pubdir;		/* Public Directory */
char	*Logfile;		/* Logging file */
Boolean AppendLogfile = FALSE;  /* Continously add to log file over sessions */
char	*MLogfile;		/* Logging file for mail */
char	*Username;		/* Username for single user machine */
char	*Timezone;		/* Timezone string ("EDT", "PST"....) */
char	*UUCPlogname;    /* Login name for UUCP */
char	*UUCPpasswd;	/* Password for UUCP */
char	*Mail;			/* Directory where mail is delivered */
long	ByteTimeout;    /* Length of time xgetc waits for a char (in secs) */
long	ConnectWait;	/* Max time we wait after dialing */
long	PacketSize = 64;/* Preferred Packet size for this side */
char 	*DialPrefix;	/* Normally ATDT, but can be used to set other parameters */
char 	*DialPostfix;	/* Normallty ATH, but can be used to reset other parameters */

int	debug = -1;		/* -1 indicates ctl file should set it */
int	ourpid = 0;		/* Our process ID */
int	logfd;			/* file desc of uucp logfile */

struct port *ports, *porttail;	/* Chain of serial port definitions */

#ifdef VMS
void *malloc();
#else
#ifndef MAC
char	*malloc();
#endif
#endif
char	*gimmestring();		/* Forward declaration */

/*
 * Read the control file and grab a few parameters.
 */
extern short apref;
 
read_params(ctl)
	char *ctl;
{
	FILE	*fd;
	char	buf[MAX_CTLLINE];
	register struct port *newport;
	register char *cmd;
	register char	*p;
	short		vRefNumTmp;
	GetVol(0L, &vRefNumTmp);
	SetVol(0L, apref);
    if (ctl) {
		/* Remove any possible setuid if user spec'd a control file */
		/* FIXME: this is system dependent....sigh
		setuid(getuid()); */
	} else {
		/* Use the system default control file */
		ctl = sysdep_control;
	}
	uuControl = ctl;
	if (! (fd = fopen(ctl, "r"))) {
		printf("Can't find configuration file %s\n", ctl);
		SetVol(0L, vRefNumTmp);
  		return FAIL;
	}

	/* find path to inbound news */
	Spool = "";
	Alias = "";
	Mail = "";			
	Sysfile = "";
	Username = "";
	Userfile = "";
	Pubdir = "";
	Logfile = "";
	Loginfile = "";
	MLogfile = "";
	UUCPlogname = "";
	UUCPpasswd = "";
	DialPrefix = "ATV1DT";
	DialPostfix = "+++\d\d\dATH\r\dats0=0\r";
	Timezone = "";
	strcpy(Myname, "");
	strcpy(Mynamealias, "");
	strcpy(Forwarder, "");
	ByteTimeout = BYTE_TIMEOUT;
	ConnectWait = CONNECT_WAIT;
	while ((long)NULL != (long)fgets(buf, sizeof buf, fd)) {

		cmd = strtok(buf, CTL_DELIM);
		if (!cmd) continue;		/* Skip blank lines */
		for (p = cmd; *p; p++)
			if (isupper(*p)) *p = tolower(*p);

		/* Now figure out which one it is */

		if (strcmp(cmd, "alias") == 0) {
			Alias = gimmefilestring();
		} else if (strcmp(cmd, "spool") == 0) {
			Spool = gimmefilestring();
		} else if (strcmp(cmd, "sysfile") == 0) {
			Sysfile = gimmefilestring();
		} else if (strcmp(cmd, "username") == 0) {
			Username = gimmestring();
		} else if (strcmp(cmd, "uucplogname") == 0) {
			UUCPlogname = gimmestring();
		} else if (strcmp(cmd, "uucppasswd") == 0) {
			UUCPpasswd = gimmestring();
		} else if (strcmp(cmd, "mail") == 0) {
			Mail = gimmefilestring();
		} else if (strcmp(cmd, "userfile") == 0) {
			Userfile = gimmefilestring();
		} else if (strcmp(cmd, "pubdir") == 0) {
			Pubdir = gimmefilestring();
		} else if (strcmp(cmd, "loginfile") == 0) {
			Loginfile = gimmefilestring();
		} else if (strcmp(cmd, "logfile") == 0) {
			Logfile = gimmefilestring();
		} else if (strcmp(cmd, "appendlogfile") == 0) {
			{
				char *applog;
				applog = gimmestring();
				{
					if (applog != NULL && (applog[0] == 'Y' || applog[0] == 'y'))
						AppendLogfile = TRUE;
						else
						AppendLogfile = FALSE;
				free(applog);
				}
		}
		} else if (strcmp(cmd, "timezone") == 0) {
			Timezone = gimmestring();
		} else if (strcmp(cmd, "mlogfile") == 0) {
			MLogfile = gimmefilestring();
		} else if (strcmp(cmd, "dialprefix") == 0) {
			DialPrefix = gimmefilestring();
		} else if (strcmp(cmd, "dialpostfix") == 0) {
			DialPostfix = gimmefilestring();
		} else if (strcmp(cmd, "nodename") == 0) {
			strcpy(Myname, strtok((char *)NULL, CTL_DELIM) ) ;
		} else if (strcmp(cmd, "nodenamealias") == 0) {
			strcpy(Mynamealias, strtok((char *)NULL, CTL_DELIM) ) ;
		} else if (strcmp(cmd, "forwarder") == 0) {
			strcpy(Forwarder, strtok((char *)NULL, CTL_DELIM) ) ;
		}
		  else if (strcmp(cmd, "debug") == 0) {
			if (debug < 0) {
				debug = atoi(strtok((char *)NULL, CTL_DELIM));
			}
		}
		  else if (strcmp(cmd, "bytetimeout") == 0) {
				{
					ByteTimeout = atoi(strtok((char *)NULL, CTL_DELIM));
			}
		}
		  else if (strcmp(cmd, "connectwait") == 0) {
				{
					ConnectWait = atoi(strtok((char *)NULL, CTL_DELIM));
			}
		}
		  else if (strcmp(cmd, "packetsize") == 0) {
				{
					PacketSize = atoi(strtok((char *)NULL, CTL_DELIM));
					if (PacketSize != 64 && PacketSize != 128)
						{
							printf("Bad value for packetsize: %ld.\n", PacketSize);
							printf("Using 64 instead.\n");
							PacketSize = 64;
							}
					switch (PacketSize)
						{
							case 64:
								segsiz = 2;
								break;
							case 128:
								segsiz = 3;
								break;
							default:
								segsiz = 2;
								break;
							}
			}
		}
			else if (strcmp(cmd, "port") == 0) {
			newport = (struct port *)malloc(sizeof *newport);
			p = strtok((char *)NULL, CTL_DELIM);
			strcpy(newport->portname, p);
			p = strtok((char *)NULL, CTL_DELIM);
			strcpy(newport->modemname, p);
			p = strtok((char *)NULL, CTL_DELIM);
			strcpy(newport->devname, p);
			p = strtok((char *)NULL, CTL_DELIM);
			newport->baud = atol(p);
			/* Add it to the end of the chain */
			newport->chain = (struct port *)0;
			if (ports)
				porttail->chain = newport;
			else
				ports = newport;
			porttail = newport;
		}
	}

	fclose(fd);
	SetVol(0L, vRefNumTmp);
	return SUCCESS;
}

/* Scan out next arg as a string, return it or NULL if none there. */
char *
gimmestring()
{
	register char *p, *q;
	p = strtok((char *)NULL, CTL_DELIM);
	if (p == NULL)
		return p;
    q = (char *)malloc(strlen(p)+1);
	strcpy(q, p);
	return q;
}

char *
gimmefilestring()
{
	register char *p, *q;
	long len;
	p = strtok((char *)NULL, FILE_DELIM);
	if (p == NULL)
		return p;
    for (;*p == ' ';p++); /* Skip whitespace at beginning */
    len = strlen(p);
    for (;p[len-1] == ' ';p[len-1] = '\0'); /* Skip whitespace at end */
	q = (char *)malloc(len+1);
	strcpy(q, p);
	return q;
}

/*
 * Scan out an "argv" from a blank-separated string.
 *
 * Result is argc, or -1 if there are too many args.
 */
int
getargs(string, argv, maxargc)
	char	*string;
	char	**argv;
	int	maxargc;
{
	int	argc = 0;

	while (*string == ' ') string++;

	for (; *string; ) {
		argv[argc++] = string;		/* Record position */
		if (argc >= maxargc) return -1;
		while (*string && *string != ' ') string++;
		if (*string == '\0') break;
		*string++ = '\0';		/* Null terminate it */
		while (*string == ' ') string++;
	}

	argv[argc] = (char *)0;			/* Null after last one */
	return argc;
}

/*
 * Log file writing subroutine.
 *
 * Makes incredibly ugly log entries that look *just like* Unix uucp's
 * incredibly ugly log entries.
 *
 * Once we don't care about compatability, we should do this much better.
 */
logit(one, two)
	char *one, *two;
{
	char logbuf[(NAMESIZE*4)+SLOP+50];	/* Temp buffer for logs */
	int len;
	int open_modes;
	if (logfd <= 0) {
		/* Open the log file if not already open */
		if (Logfile == 0 || *Logfile == 0)
			return;		/* If none specified, no logging. */
		open_modes = O_CREAT|O_WRONLY|O_TEXT;
		if (AppendLogfile == TRUE)
			open_modes = open_modes | O_APPEND;
			else
			open_modes = open_modes | O_TRUNC;
		logfd = open(Logfile, open_modes);
		if (logfd < 0) {
			if (DEBUG_LEVEL(0)) perror(Logfile);
			exit(EXIT_ERR);
		}
	}

	sprintf(logbuf, "%s %s (%s) %s (%s)\n",
		who, host_name, time_and_pid(), one, two);

	DEBUG(0, "%s", logbuf);

	len = strlen(logbuf);
	if (len != write(logfd, logbuf, len)) {
		if (DEBUG_LEVEL(0)) {
			fprintf(stderr, "Can't write to log, terminating!\n");
			perror(Logfile);
		}
		exit(EXIT_ERR);		/* Terminate if we can't log */
	}

#ifdef LOGCLOSE
	if (close(logfd) != 0) {
		if (DEBUG_LEVEL(0)) {
			fprintf(stderr, "Can't close log, terminating!\n");
			perror(Logfile);
		}
		exit(EXIT_ERR);		/* Terminate if we can't log */
	}
	logfd = -1;
#endif
}

mlogit(one, two)
	char *one, *two;
{
	char logbuf[(NAMESIZE*4)+SLOP+50];	/* Temp buffer for logs */
	int len;

	if (logfd <= 0) {
		/* Open the log file if not already open */
		if (Logfile == 0 || *Logfile == 0)
			return;		/* If none specified, no logging. */
		logfd = open(MLogfile, O_CREAT|O_WRONLY|O_APPEND|O_TEXT);
		if (logfd < 0) {
			if (DEBUG_LEVEL(0)) perror(MLogfile);
			exit(EXIT_ERR);
		}
	}

	sprintf(logbuf, "%s %s (%s) %s (%s)\n",
		who, host_name, time_and_pid(), one, two);

	DEBUG(0, "%s", logbuf);

	len = strlen(logbuf);
	if (len != write(logfd, logbuf, len)) {
		if (DEBUG_LEVEL(0)) {
			fprintf(stderr, "Can't write to log, terminating!\n");
			perror(MLogfile);
		}
		exit(EXIT_ERR);		/* Terminate if we can't log */
	}

#ifdef LOGCLOSE
	if (close(logfd) != 0) {
		if (DEBUG_LEVEL(0)) {
			fprintf(stderr, "Can't close log, terminating!\n");
			perror(MLogfile);
		}
		exit(EXIT_ERR);		/* Terminate if we can't log */
	}
	logfd = -1;
#endif
}

void
cuserid(str)
char *str;
{
	strcpy(str, Username);
	}
