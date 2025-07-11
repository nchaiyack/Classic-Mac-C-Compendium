/*			dcp.c

			Revised edition of dcp

			Stuart Lynne May/87

			Copyright (c) Richard H. Lamb 1985, 1986, 1987
			Changes Copyright (c) Stuart Lynne 1987
			
			Portions Copyright � David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/
/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* This program implements a uucico type file transfer and remote
execution type protocol.
*/
#include "dcp.h"				/**/
#include <Power.h>
#include <GestaltEqu.h>			/* BH-C */
#include <stdarg.h>

FILE	*logfile;            	/* system log file */
FILE	*syslog;            	/* system log file */
FILE	*fw;                 	/* cfile pointer */
char	state;					/* system state*/
char	cfile[80];              /* work file pointer */
int		remote;                 /* -1 means we're remote ..7*/
int		del_file_flag;			/* means delete file(s) at end of xfer */
int		msgtime;                /* timout setting */
char	fromfile[132];
char	rmtfname[132];
char	hostfile[132];			/* host version of fromfile */
char	tofile[132];
int		fp;                     /* current disk file ptr */
int		size;                   /* nbytes in buff */
int		pktsize;				/* packet size for this protocol*/
int     xfer_problems;
int  	useHardwareFlowControl;
FILE	*fsys;
char	Rmtname[20];
char	rmtname[20];
char	gotname[20];
char	callcause[64];
char	* cctime;
char	proto[5];
/* char	loginseq[256]; */
char 	sysline[BUFSIZ*2];
char	s_systems[64];
char	s_logfile[64];
char	s_syslog[64];
char	s_schedule[64];
char	s_statfile[64];
char 	*flds[60];
int 	kflds;
int		failureCount;
time_t	theTime;
time_t	runStartSecs;

int		PacketTimeout = 5;
int		MaxErr = 20;
int		xfer_retries_left;


struct HostStats remote_stats; /* host status, as defined by hostatus */
status_record Last;
ConnectionHandler *currentConnection;

unsigned int	checksum();


/**/
/* new usage

	dcp	[-xn] -r0		slave mode
	dcp	[-xn] -shost 	call host
	dcp	[-xn] -sall		call all hosts
	dcp	[-xn] -sany		call any hosts as required by C. files
	dcp [-xn] -scron	call hosts at times specified in cron file
	dcp [-xn] -sneed	call hosts per cron-file schedule or if work is pending
	dcp [-xn] -sauto	call hosts per cron-file schedule or if work is pending,
	                    and accept incoming calls between times

*/

complain(char *s)
{
	fprintf( stderr, "Please set your %s environment variable.", s );
}

int cant(char *file)
{
	fprintf( stderr, "Can't open: \"%s\"\n", file );
	return (FAILED);
}

int dcpmain(int argc, char **argv)
{
	char	line[132];
	int	Line_Open = FALSE;
	int Status = OK;			/* OK */
	int active, first;
	int realremote;
	int waitforever;
	int callup_for_inbound;
	int quit, shutdown;
	int could_slave, did_slave;
	int once;
	int runTimeout;
	long	startSecs;
	long	wakeupSecs;
	long	oldWakeupSecs;
	long	powerManagerFlags;
	Byte	wakeupFlag;
	long	Start_Ticks;
	long	Session_Start_Ticks;
	long	minutes_to_sleep;
	OSErr	wakeupError;
	
	if ( name == NULL || *name == '\0' ) {
		complain( NAME );
		return (FAILED);
	}
	if ( nodename == NULL || *nodename == '\0' ) {
		complain( NODENAME );
		return (FAILED);
	}
	time(&theTime);
	if (Gestalt(gestaltPowerMgrAttr,&powerManagerFlags) != noErr) {
		powerManagerFlags = 0;
	}

	mkfilename( line, spooldir, LOGFILE );
	if (strchr(spooldir, SEPCHAR) == NULL && strchr(spooldir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_logfile, curdir, line);
	else
		strcpy(s_logfile, line);
		
	mkfilename( line,  spooldir, SYSLOG  );
	if (strchr(spooldir, SEPCHAR) == NULL && strchr(spooldir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_syslog, curdir, line);
	else
		strcpy(s_syslog, line);
		
	mkfilename( line, confdir,  SYSTEMS );
	if (strchr(confdir, SEPCHAR) == NULL && strchr(confdir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_systems, curdir, line);
	else
		strcpy(s_systems, line);
		
	mkfilename( line, confdir, SCHEDULE );
	if (strchr(confdir, SEPCHAR) == NULL && strchr(confdir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_schedule, curdir, line);
	else
		strcpy(s_schedule, line);

	mkfilename( line, spooldir, STATFILE );
	if (strchr(spooldir, SEPCHAR) == NULL && strchr(spooldir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_statfile, curdir, line);
	else
		strcpy(s_statfile, line);
		
	Status = OK;
	remote = MASTER;
	debuglevel = 0;
	active = FALSE;
	first = TRUE;
	quit = FALSE;
	shutdown = FALSE;
	once = FALSE;
	runTimeout = -1;
	Session_Start_Ticks = TickCount();

	strcpy( Rmtname, "any" );		/* default */
	fw = (FILE *) NULL;
	
	while ( argc ) {
		if ( **argv == '-') {
			switch(argv[0][1]) {
			case 'x':
				debuglevel = atoi( &argv[0][2] );
				printmsg(1, "Debug level set to %d", debuglevel);
				break;
			case 's':
				strcpy (Rmtname, &argv[0][2]);
				break;
			case 'r':
				remote = atoi( &argv[0][2] );
				break;
			case 'q':
				quit = TRUE;
				printmsg(1, "Quit-after-run flag set");
				break;
			case 'S':
				shutdown = TRUE;
				printmsg(1, "Shutdown-after-run flag set");
				break;
			case 'o':
				once = TRUE;
				printmsg(1, "One-run flag set");
				break;
			case 't':
				sscanf(argv[0]+2, "%d", &runTimeout);
				printmsg(1, "Timeout set to %d minutes", runTimeout);
				break;

			default:
				break;
			}
		}
		argv++;
		argc--;
	}

	realremote = remote;

main_loop:

	if (dcxqt())
		printmsg( 0, "ERROR in DCXQT" );

	if (logfile != (FILE *) NULL)  
		printmsg( 0, "logfile already open" );
	if (syslog != (FILE *) NULL)  
		printmsg( 0, "syslog already open" );

	/* reopen log each time */
	if ( (logfile = FOPEN( s_logfile, "a", 't' )) == NULL )
	   return(cant(s_logfile));
	if ( (syslog  = FOPEN( s_syslog,  "a", 't' )) == NULL )
	   return(cant(s_syslog));
	
start_next_run:

#ifdef Upgrade
	systimeout = 0;
	active = FALSE;
	did_slave = FALSE;
	could_slave = FALSE;
	gotname[0] = 0;
	remote = realremote;
	time(&runStartSecs);
	Last.name[0] = '\0';
	rmtname[0] = '\0';
#endif Upgrade
	fp = -1;
	if (fw != (FILE *)NULL) {
		fclose(fw);
		fw = (FILE *)NULL;
	}
	
#ifdef MULTIFINDER
	Start_Ticks = TickCount();
#endif
	
	if ( remote == MASTER ) {
 		printmsg( 1, "Calling %s, debuglevel=%d", Rmtname, debuglevel );
		state = 'I';
		xfer_problems = 0;

		if (fsys != (FILE *) NULL)  
			fprintf( stderr, "fsys already open" );
		if (( fsys = FOPEN( s_systems, "r", 't' )) == (FILE *)NULL ) {
			printmsg(0, "Error %d, can't open %s file!", errno, s_systems);
			Status = FAILED;
			goto abort;
		}

		while (TRUE) {
#ifdef MULTIFINDER
			Check_Events(MF_DELAY);
#endif
			printmsg( 4, "Mstate = %c", state );
			switch (state) {
			case 'I':
				if (Check_Events(MF_DELAY)) goto abort;
				systimeout = 0;
				state = getsystem(Rmtname);
				if (state != 'A' && strcmp(Rmtname, gotname) == SAME) {
					state = 'I'; /* dplatt, don't repeat successful call on different line */
				}
				break;
			case 'T':
				state = 'I';
				break;
			case 'S':
				active = TRUE;
				state = callup();
				break;
			case 'P':
				(*currentConnection->AllowInterrupts)(FALSE);
				state = startup();
				break;
			case 'D':
				state = master();
				break;
			case 'Y':
				state = sysend();
				if (xfer_problems > 0) {
					printmsg( 0, "Bad Things happened during that call");
					failureCount ++;
					Status = FAILED;
				} else {
					Status = OK;
				}
				Update_Status(Status);
				if ((strcmp(Rmtname, "any")  != SAME) &&
				    (strcmp(Rmtname, "all")  != SAME) &&
				    (strcmp(Rmtname, "cron") != SAME) &&
				    (strcmp(Rmtname, "need") != SAME) &&
				    (strcmp(Rmtname, "auto") != SAME)) {
				    state = 'A';
				} else {
					strcpy(gotname, rmtname); /* remember who we got */
				}
				break;
			case 'G':
				if ((strcmp(Rmtname, "any")  == SAME) ||
				    (strcmp(Rmtname, "all")  == SAME) ||
				    (strcmp(Rmtname, "cron") == SAME) ||
				    (strcmp(Rmtname, "need") == SAME) ||
				    (strcmp(Rmtname, "auto") == SAME))
					state = 'I';
				else
					state = 'Y';
				break;
			case 'X':
				closeline();
				Line_Open = FALSE;
				Status = FAILED;	/* I think this is always failure */
				printmsg( 0, "Call failed");
				failureCount ++;
				Update_Status(FAILED);
				state = 'I';
				break;
			}
			if (state == 'A') {
				if (Line_Open) {
					printmsg(0, "Disconnecting");
					closeline();
					Line_Open = FALSE;
				}
				rmtname[0] = 0;		/* zap name so it won't be in log anymore */
				if (fw != (FILE *)NULL) {
					fclose(fw);
					fw = (FILE *)NULL;
				}
				if (fp != -1) {
					close(fp);
					fp = -1;
				}
				break;
			}
		}
		if (logfile != (FILE *)NULL) {
			fflush(logfile);
		}
	}

	if (fsys != (FILE *)NULL) {
		fclose( fsys );
		fsys = (FILE *)NULL;
	}

	if (Main_State != Call_Systems) goto abort;
	
	if (active) {
		if (dcxqt())
			printmsg( 0, "ERROR in DCXQT" );
	}
	
	if (remote == SLAVE || strcmp(Rmtname, "auto") == SAME) {
	
		/**** SLAVE MODE (wait for calls) ****/
		
		waitforever = TRUE;
		if (strcmp(Rmtname, "auto") == SAME || once) {
			waitforever = FALSE;
		}
		
		if (fsys != (FILE *) NULL)  
			fprintf( stderr, "fsys already open" );
		if (( fsys = FOPEN( s_systems, "r", 't' )) == (FILE *)NULL ) {
			printmsg(0, "Error %d, can't open %s file!", errno, s_systems);
			Status = FAILED;
			goto abort;
		}

		remote = SLAVE;
		callup_for_inbound = FALSE;
		do {
			state = getsystem("INCOMING");
			if (state == 'S' || state == 'T') could_slave = TRUE;
		} while (state != 'S' && state != 'A');
		rmtname[0] = '\0';

		if (state != 'S') {
			goto no_slave;
		}
		did_slave = TRUE;
			
		if (active || first) {
			printmsg(0, "Awaiting incoming call");
		}
		while (TRUE) {
#ifdef MULTIFINDER
			if (Check_Events(MF_DELAY)) goto killincoming;
#endif
			printmsg( 4, "Sstate = %c", state );
			switch (state) {
			case 'S':
				state = callup();
				callup_for_inbound = TRUE;
				break;
			case 'P':
				Line_Open = TRUE;
				state = login(waitforever, callup_for_inbound);
				break;
			case 'I':
				active = TRUE;
				(*currentConnection->AllowInterrupts)(FALSE);
				PlayNamed("\pIncoming");
				state = startup();
				break;
			case 'R':
				state = slave();
				break;
			case 'Y':
				state = sysend();
				break;
			case 'X':
				state = 'A';
				break;
			}
			if (state == 'A') {
				break;
			}
		}
killincoming:
		closeline();
		Line_Open = FALSE;
no_slave: ;
	}
	
	if (fsys != (FILE *)NULL) {
		fclose( fsys );
		fsys = (FILE *)NULL;
	}

	if (logfile != (FILE *)NULL) {
		fflush(logfile);
	}

	if (Main_State != Call_Systems) goto abort;
	
	if (dcxqt())
		printmsg( 0, "ERROR in DCXQT" );
	
	if (active) printmsg( 0, "Session completed");
	
	if (once || runTimeout < 0 || (TickCount() - Session_Start_Ticks) / 3600 >= runTimeout) {	
		if (quit) {
			printmsg(0, "Quitting");
			Main_State = Quit_Program;
			goto abort;
		}
		if (shutdown) {
			printmsg(0, "Shutting down");
			Main_State = Shutdown_Program;
			goto abort;
		}
	}
	
	if (once) {
		printmsg(1, "Exiting due to one-run flag");
		goto abort;
	}
	
	if (runTimeout >= 0 && (TickCount() - Session_Start_Ticks) / 3600 >= runTimeout) {
		printmsg(1, "Exiting due to timeout");
		goto abort;
	}
	
	if (strcmp(Rmtname, "auto") == SAME || realremote == SLAVE) {
		/* in these modes we never exit;  most of the time is spent in slave mode,
		   waiting for an inbound call to occur. */
		if (!did_slave) {
			if (active || first) {
				if (!could_slave) {
					printmsg(0, "No INCOMING entry in Systems file, cannot accept calls");
					if (realremote == SLAVE) goto abort;
				} else {
					printmsg(0, "Ineligible for incoming calls at this time; sleeping");
				}
			}
			goto sleep_a_while;
		}
		first = FALSE;
		goto start_next_run;
	}
		
	if (strcmp(Rmtname,"cron") == SAME || strcmp(Rmtname,"need") == SAME) {
		/* in cron and need mode, we never exit, we just keep waiting for a time to call
		   some system */

		if (active || first) printmsg(0, "Sleeping to await next scheduled call");
		
sleep_a_while:
		
		first = FALSE;

		/* close logfile while we're not using it */
		if (syslog != (FILE *)NULL) {
			fclose(syslog);
			syslog = (FILE *)NULL;
		}
		if (logfile != (FILE *)NULL) {
			fclose(logfile);
			logfile = (FILE *)NULL;
		}
		
#ifdef MULTIFINDER
		sscanf(sleeptime, "%ld", &minutes_to_sleep);
		if (minutes_to_sleep < 1) minutes_to_sleep = 1;
		/* wait for a few minutes and then see if there is anything to do yet */
		Start_Ticks = TickCount();
		if (powerManagerFlags & (1L << gestaltPMgrExists)) {
			printmsg(1, "Reading current time");
			ReadDateTime(&startSecs);
			wakeupSecs = startSecs + minutes_to_sleep * 60 + 2;
			printmsg(1, "Reading current wakeup time");
			if (noErr == (wakeupError = GetWUTime(&oldWakeupSecs, &wakeupFlag))) {
				if ((wakeupFlag & 1) == 0 ||
				 oldWakeupSecs <= startSecs ||
				 oldWakeupSecs > wakeupSecs) {
				 	printmsg(1, "Setting new wakeup time");
					if (noErr != (wakeupError = SetWUTime(wakeupSecs))) {
						printmsg(0, "Error %d trying to set wakeup timer", wakeupError);
					}
				}
			} else {
				printmsg(0, "Error %d trying to read wakeup timer", wakeupError);
			}
		}
		printmsg(0, "Sleeping");
		while (TickCount()-Start_Ticks < minutes_to_sleep * 3600) {
			if (Check_Events(60)) goto abort;	/* allow a 1 second wait */
		}
#endif
		goto main_loop;
	}

	/* scan and process any recieved files */
abort:
	if (syslog != (FILE *)NULL) {
		fclose(syslog);
		syslog = (FILE *)NULL;
	}
	if (logfile != (FILE *)NULL) {
		fclose(logfile);
		logfile = (FILE *)NULL;
	}
	
	if (Line_Open) closeline();
	return Status;
}


/**/
/*
**
**
**master
**
**
*/
master(void)
{
	state = 'I';
	while (TRUE) {
		printmsg( 4, "Top level state (master mode) %c", state );
		switch (state) {
		case 'I':
			state = sinit();
			break;
		case 'B':
			state = scandir();
			break;
		case 'S':
			state = send();
			break;
		case 'Q':
			state = sbreak();
			break;
		case 'G':
			state = receive();
			break;
		case 'C':
			state = schkdir();
			break;
		case 'Y':
			state = endp(TRUE);
			break;
		case 'H':
			state = endp(FALSE);
			break;
		case 'P':
			return('Y');
		case 'A':
			return('A');
		case 'X':
			return('X');
		default:
			return('A');
		}
	}
}


/**/
/*
**
**
**slave
**
**
*/
slave(void)
{
	state = 'I';
	while (TRUE) {
		printmsg( 4, "Top level state (slave mode) %c", state );
		switch (state) {
		case 'I':
			state = rinit();
			break;
		case 'F':
			state = receive();
			break;
		case 'C':
			state = schkdir();
			break;
		case 'T':
			state = 'B';
			break;
		case 'B':
			state = scandir();
			break;
		case 'S':
			state = send();
			break;
		case 'Q':
			state = sbreak();
			break;
		case 'G':
			return('Y');
		case 'Y':
			state = endp(TRUE);
			break;
		case 'H':
			state = endp(FALSE);
			break;
		case 'P':
			return('Y');
		case 'A':
			return('A');
		default:
			return('A');
		}
	}
}


/**/
/*
 *  r e c e i v e
 *
 *  This is the state table switcher for receiving files.
 */

receive(void)
{

	state = 'F';/* Receive-Init is the start state */

	while (TRUE) {
		printmsg( 4, " receive state: %c", state );
		switch (state)/* Do until done */ {
		case 'F':
			xfer_retries_left = MAX_XFER_RETRIES;
			state = rfile();
			break; /* Receive-File */
		case 'D':
			state = rdata();
			break; /* Receive-Data */
		case 'R':
			if (--xfer_retries_left < 0) {
				printmsg(0, "Retry limit reached, bailing out");
				if (fp && fp != -1) {
					close(fp);
					fp = -1;
				}
				return ('Y');
			}
			printmsg(0, "retrying transfer");
			makenull(fp);
			(*filepkt)(); /* re-init for file transfer */
			state = 'D'; /* received-data-corrupt, so try again */
			xfer_problems ++;
			break;
		case 'C':
			return('C');/* Complete state */
		case 'S':               /* Send-data */
			state = sdata();
			break;
		case 'Z':               /* Send-End-of-File */
			state = seof();
			break;
		case 'A':
			return('Y');/* "Abort" state */
		case 'H':
			return ('H'); /* Semi-polite hangup */
		default:
			return('Y');
		}
	}
}


/**/
/*
 *  s e n d
 *
 *  Sendsw is the state table switcher for sending files.  It loops until
 *  either it finishes, or an error is encountered.  The routines called
 *  by sendsw are responsible for changing the state.
 *
 */
send(void)
{
	fp = -1;                /* reset file getter/opener */
	state = 'F';/* Send initiate is the start state */

	while (TRUE)/* Do this as long as necessary */ {
		printmsg( 4, "send state: %c", state );
		switch (state) {
		case 'F':
			xfer_retries_left = MAX_XFER_RETRIES;
			state = sfile();
			break; /* Send-File */
		case 'D':
			state = sdata();
			break; /* Send-Data */
		case 'Z':
			state = seof();
			break; /* Send-End-of-File */
		case 'R':                       /* Receive-data */
			state = rdata();
			if (state == 'D')          /* re-interpret the state! */
				 state = 'R';
			else if (state == 'R')
				 state = 'T';			/* uglier yet... dplatt */
			break;
		case 'T':
			if (--xfer_retries_left < 0) {
				printmsg(0, "Retry limit reached, bailing out");
				if (fp && fp != -1) {
					close(fp);
					fp = -1;
				}
				return ('Y');
			}
			printmsg(0, "retrying transfer");
			makenull(fp);
			(*filepkt)(); /* re-init for file transfer */
			state = 'R'; /* received-data-corrupt, so try again */
			xfer_problems ++;
			break;
		case 'B':
			return ('B'); /* Complete */
		case 'A':
			return ('Y'); /* "Abort" */
		case 'H':
			return ('H'); /* Semi-polite hangup */
		default:
			return ('Y'); /* Unknown, fail */
		}
	}
}


/**/
/* A command formatter for DCP. RH Lamb */
/* sets up stdin and stdout on various machines */
/* There is NO command checking so watch what you send and who you */
/* let accsess your machine. "C rm /usr/*.*" could be executed. */
dcxqt(void)
{
	int	i;
	char	command[CMDBUFSIZE], input[60], output[60], reqd[60], line[CMDBUFSIZE];
	char	*cp;
	int status = 0;
	
	while (dscandir()) {
		strcpy( line, cfile );
		fw = FOPEN( line, "r", 't' );/* imported X file */
		strcpy(cfile, line);
		printmsg( 2, "dcxqt:%s %ld", cfile, fw );
		input[0]   = '\0';
		output[0]  = '\0';
		command[0] = '\0';
		reqd[0] = '\0';
		gotitfrom[0] = '\0';
		gotfromuser[0] = '\0';
		while ( fgets( line, BUFSIZ, fw ) != (char *)NULL ) {

#ifdef MULTIFINDER
			status = Check_Events(MF_DELAY);
#endif
			cp = index( line, '\n' );
			if ( cp != (char *)NULL )
				*cp = '\0';

			printmsg( 8, "dcxqt: %s", line );
			switch (line[0]) {
			case 'U':
				sscanf(line, "U%*[ ]%s%*[ ]%s", gotfromuser, gotitfrom);
				printmsg (5, "<%s> user=%s site=%s", line, gotfromuser, gotitfrom);
				break;
			case 'I':
				strcpy( input,   line+2 );
				break;
			case 'O':
				strcpy( output,  line+2 );
				break;
			case 'C':
				strcpy( command, line+2 );
				break;
			case 'F':
				strcpy( reqd, line+2 );
				break;
			case 'R':
			case '#':
			case 'Z':
			case 'N':
				break;	/* not implemented */
			default :
				break;
			}
		}
		if (fw != (FILE *)NULL) {
			fclose(fw);
			fw = (FILE *)NULL;
		}
		
		if (strlen(gotitfrom) > 0 && strlen(gotfromuser) > 0) {
			printmsg(0, "%s!%s: %s", gotitfrom, gotfromuser, command );
		} else {
			printmsg(0, "XQT %s", command );
		}

		if (shell(command, input, output)) {
			UNLINK(cfile);
			if (strlen(input) > 0) {
		 		importpath( hostfile, input );
		 		UNLINK(hostfile);
		 	}
			if (strlen(output) > 0) {
				importpath( hostfile, output );
	 			UNLINK(hostfile);
	 		}
	 	} else {
	 		return(1); /* Trouble;  bail out */
	 	}
 		if (status) return(0);	/* cancel operations */
	}
	return(0);
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
#ifdef	THINK_C
	static	char	loghdr[50]={0};
	struct tm		*t;
	va_list ap;
#endif	THINK_C
	if ( debuglevel >= level ) {
#ifdef  THINK_C
		time(&theTime);
		t = localtime(&theTime);
#ifdef MULTIFINDER
		printf( "%02d:%02d:%02d ", t->tm_hour, t->tm_min, t->tm_sec );
#else
		if ( remote == MASTER  || strcmp(Rmtname, "auto") == SAME) {
			printf( "%02d:%02d:%02d ", t->tm_hour, t->tm_min, t->tm_sec );
		}
#endif
#endif
		va_start(ap, fmt);
		vsprintf( msg, fmt, ap );
		va_end(ap);
#ifdef MULTIFINDER
		fputs(msg, stdout);
		fputc('\n', stdout);
#else
		if ( remote == MASTER || strcmp(Rmtname, "auto") == SAME) {
			fputs(msg, stdout);
			fputc('\n', stdout);
		}
#endif
#ifdef	THINK_C
			if (level <= 5 && logfile) {
				fprintf(logfile, "%-8s %02d/%02d-%02d:%02d (%s)\n",
							/* (remote == MASTER)?nodename : rmtname, */
							rmtname,
							t->tm_mday, t->tm_mon+1,
							t->tm_hour, t->tm_min,
							msg);
			}
#else	THINK_C
		if (logfile) {
			fputs( msg, logfile );
			fputc( '\n', logfile);
		}
#endif	THINK_C
	}
}
