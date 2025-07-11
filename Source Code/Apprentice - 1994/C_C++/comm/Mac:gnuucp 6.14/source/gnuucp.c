/*
 * @(#)gnuucp.c 1.29 87/09/29	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 *
 * Derived from:
 * i[$]uuslave.c	1.7 08/12/85 14:04:20
 * which came from the ACGNJ BBS system at +1 201 753 9758.  Original
 * author unknown.  Many people, too numerous to list here, have contributed
 * changes and ports to this program.
 */

/*
6. 11 	- 	Color Icons
			57,600 baud modem support
			getstring fix
6. 12   - 	Support Taylor UUCP by turning of ACK's 
		  	after a CLOSE message has arrived.
		  	Make MAX_STRING 1024
*/

char version[] = "Version Fort Pond Research-6.14";
#include <console.h>
#include <Serial.h>
#include <SegLoad.h>
#include <signal.h>
#include <profile.h>
#include <pascal.h>
#include <Memory.h>
extern int inRefNum;
/*

This program implements the uucp (Unix-to-Unix CoPy) protocol, used to
transfer mail, files, and Usenet news among Unix machines.  UUCP comes
free with Unix (unless you get a sleazeoid version like Xenix, where
they charge you extra for it), but you don't get sources.  You can buy
a commercial program for MSDOS, called UULINK, which also implements
this protocol.  UULINK costs $300 and you still don't get sources.
This program not only runs on Unix and MSDOS, but on many other
machines, and comes with the Free Software Foundation copyright, which
guarantees you access to sources.

The protocol requires a full 8-bit data path with no characters inserted
or deleted (e.g. ^S and ^Q are used as DATA characters).  Simple serial
ports and modems do this; most complicated networks do not, at least without
setting up odd modes and such.  Telenet's PC Pursuit works fine though.

The basic flow of the protocol is that the calling machine will send down
a line of text saying what it wants to do (send a file, receive a file,
or hang up).  (The lines of text are encapsulated into packets; see below.)
The called machine responds with a "yes" or "no" answer, and if the answer
was yes, it sends or receives the file.  Files are terminated with a
packet containing 0 bytes of data.  Then the system that received the file
sends a "copy succeeded" or "copy failed" line to the other end, and 
they go back to "what do we do now".  A request to hang up should be
answered "no" if the called machine has some mail or files it wants to
send to the calling machine; the two machines reverse roles and the calling
machine goes into "what do we do now".  If a hangup request is answered "yes",
the call is terminated.

The data flow described above is actually sent by a lower level "protocol
module".  The default "g" protocol module sends the data in packets containing
checksums and acknowledgements.  Each packet can either hold a short
control message, e.g. an ack, or a data block.  The data blocks are
numbered with a 3-bit counter, and sent with a checksum.  If the sender
has not received an acknowledgement for a data block within a certain
time, it retransmits the block.  The size of a data block is negotiated
at the start of a call.  To send a block with fewer bytes, a "short
data" block is sent, which is just as big as a "long data" block, but
contains a 1- or 2-byte count of "how many bytes in this block are just
padding".  This is a cute trick since it always works (e.g. if you want
to send 1023 out of 1024 bytes, you only need one byte for the count;
while if you want to send 1 byte out of 1024 then you have enough space
for the count to be 2 bytes).

The short control messages are used to start the call and negotiate the
packet size and the "window size", to acknowledge or reject packets,
and to terminate the packet protocol at the end of a call.  The window
size is how many packets one side can send before it will stop and wait
for an acknowledgement from the other side.  A window size of 1 makes
for a half-duplex protocol (which is what gnuucp currently
implements), but also makes it easy to implement on micros that don't
handle serial lines with interrupts.  In window 1, you just keep
sending the same packet until the other side acknowledges it.  Unix
always uses a window size of 3, which is the max that can be dealt with
given the 3-bit packet numbers (for reasons that would take more space
than I want to spend here).  This gives much better throughput, but
requires full duplex serial port handling and more complicated
acknowledgement strategies.

In receiving data, the "g" protocol scans for a DLE (hex 10) which
indicates the start of a packet, and the next 5 bytes are read and
checked.  If they pass, this is a good packet and it is acted upon.
(If it's a data packet, we have to read in and check the data part
too.)  If the checks fail, all the bytes read so far should be rescanned
for another DLE, since it's possible that some characters were dropped
and the first DLE we saw was actually part of a data packet.

Other protocol modules have been implemented; e.g. the "f" protocol which
eliminates the packets and acknowledgements, for reduced overhead when
running over an error-free, flow controlled connection on an X.25 network.
There has been talk of a "z" protocol which implements ZMODEM.  So far,
none of these modules are supplied with gnuucp, though "f" is public domain
and "z" is intended to be, because they are not ported and tested yet.

At the start of a call, the caller sends the list of protocols it supports,
and the callee picks the best one it knows and says "use this one please".

At the low level, full 8-bit bytes are sent out and received on an async
serial port.
*/

#include "includes.h"		/* System include files, system dependent */
#include "uucp.h"		/* Uucp definitions and parameters */

#define	MAX_FLAGS	40

extern int errno;

char	Progname[NAMESIZE];			/* Our program name e.g. "uuq" */
char	ttynam[NAMESIZE],		/* Name of tty we use as serial port */
	srcnam[NAMESIZE],		/* Source file name */
	dstnam[NAMESIZE],		/* Dest file name */
	flags[MAX_FLAGS],		/* Flags from file xfer cmd */
	temp[NAMESIZE];			/* Temp file name */

int	fdtty,				/* Opaque handle to pass to protocol */
	ignore_time_restrictions = 0,	/* Call out even if L.sys sez no */
	mode;				/* File mode from file xfer cmd */

int   f_wait    = 0;	/* FIXME, unreferenced now */
int   loop    = 1;	/* Loop accepting logins if tty name specified */
int   curtemp = 0;

#define	MAX_STRING	1024	/* Max length string to send/expect */

/* Strings sent by the slave side to the master */
char msgo0[] = "\nlogin:";
char msgo1[] = "Password:";
/* char msgo2[] = "\20Shere\0"; Now we send it specially */
char msgo3[] = "\20ROK\0";
char msgo3a[]= "\20Pg\0";
char msgo4[] = "\20OOOOOOO\0";

/* Strings sent by the master to the slave */
char msgi0[] = "uucp\n";
char msgi1[] = "s8000\n";
/* char msgi2[] = "\20S*\0"; We now scan it specially FIXME */
char msgi3[] = "\20Ug\0";
char msgi4[] = "OOOOOO";

/*
 * Protocol switch data structure
 */
struct proto {
	char	p_id;
	int	(*p_turnon)(int);
	int	(*p_rdmsg)(char *, int);
	int	(*p_wrmsg)(char, char *, int);
	int	(*p_rddata)(int, FILE *);
	int	(*p_wrdata)(FILE *, int);
	int	(*p_turnoff)(void);
};

/* extern int gturnon(), grdmsg(), gwrmsg(char), grddata(), gwrdata(), gturnoff(); */
/*
 * This is how the "f" protocol would be declared:
 * extern int fturnon(), frdmsg(), fwrmsg(), frddata(), fwrdata(), fturnoff();
 */

struct proto ptbl[] = {
	/* "f" protocol to run on X.25 PADs */
  /*	{'f', fturnon, frdmsg, fwrmsg, frddata, fwrdata, fturnoff}, */
	/* Original "g" protocol for dialup async modem lines */
	{'g', gturnon, grdmsg, gwrmsg, grddata, gwrdata, gturnoff},
};

struct proto *curproto = &ptbl[0];

#define	turnon	(*curproto->p_turnon)
#define	rdmsg	(*curproto->p_rdmsg)
#define	wrmsg	(*curproto->p_wrmsg)
#define	rddata	(*curproto->p_rddata)
#define	wrdata	(*curproto->p_wrdata)
#define	turnoff	(*curproto->p_turnoff)

/*
 * Read from the serial port a null-terminated string.
 * The string FIXME should not overrun MAX_STRING.
 * String starts with first printing character.
 */
int
getstring(where)
	register char *where;
{
	register int data, count = 0;
	DEBUG(8, "Getstring:  ", 0);
	/* Read data until null character */
	while ( (long)((data = xgetc()) != (long)EOF)) {
		if (count < MAX_STRING)
			{
				data &= 0x7F;
				if (DEBUG_LEVEL(8)) 
					{
						printf("%02x%c ", data, isprint(data)? data: ' ');
						}
        		where[count++] = data;
				if (count == 1 && !isprint(where[0]))
					count = 0;
				if (data == 0x00 && count != 0) 
					{
						if (DEBUG_LEVEL(8)) putchar('\n');
						return SUCCESS;
						}
				}
			else
			  {
			  	where[MAX_STRING-1]= '\0';
			  	logit("String too long for buffer", where);
			  	break;
			  	}
		}
	if (DEBUG_LEVEL(8)) putchar('\n');
	return FAIL;
}


/*
 * Medium level input routine.
 *
 * Look for an input string for the send-expect sequence.
 * Return 0 for matching string, 1 for timeout before we found it.
 * FIXME:  we only time out if the other end stops sending.  If it
 *	   keeps sending, we keep listening forever.
 */
instr(s,n, level)
register char *s;
register int n;
int level;
{
	register int data;
	register char	*pattern;
	register int i;

	if (DEBUG_LEVEL(2)) {
		printf("Expecting ");
		for (i = 0; i < n; i++)
			printf("%02x%c ",s[i] & 0xFF, isprint(s[i])? s[i]: ' ');
		printf("\nR ");
	}

	pattern = s;

	while ((long)(data = xgetc()) != (long)EOF) {
		data &= 0x7F;			/* Ignore parity */

		if (DEBUG_LEVEL(2)) {
			printf("%02x%c ", data, isprint(data)? data: ' ');
		}

	tryfirst:

		if (data == *pattern) {
			/* Input matches next byte of pattern */
			pattern++;
			if (pattern >= &s[n]) {
				if (DEBUG_LEVEL(2)) putchar('\n');
				return SUCCESS;		/* Done */
			}
		} else {
			/* Input doesn't match pattern */
			if (pattern == s) continue;	/* First char */
			/* FIXME!  This doesn't work if pattern can match
			 * in more than one way initially, e.g.:
			 * "NECT" works when "CONNECT" is output, but
			 * "NNECT" fails when "CONNNECT" is output... */
			pattern = s;	/* Does cur char match first? */
			goto tryfirst;	/* FIXME */
		}
	}

	if (DEBUG_LEVEL(2))	putchar('\n');
	DEBUG(level, "Didn't see expected string '%s'\n", s);
	return FAIL;
}

/*
 * Debugging hack for stuff written to the modem.
 */
int
twrite(s, n)
	char *s;
	int	n;
{
	register int i;

	if (DEBUG_LEVEL(2)) {
		printf("Wrote:  ");
		for (i = 0; i < n; i++)
			printf("%02x%c ",s[i] & 0xFF, isprint(s[i])? s[i]: ' ');
		printf("\n");
	}

	return xwrite(/* 0, */s, n);
}

short apref;
extern char *argv;
char input_str[256];

extern int orig_debug;
/*
 * MAIN ROUTINE.
 *
 * This is called at program startup.  It parses the arguments to the
 * program (if any) and sets up to receive a call on the modem.
 *
 * If there are no arguments, we assume the caller is already on standard
 * input, waiting to do uucp protocols (past the login prompt), and we
 * just handle one caller.
 *
 * If there is an argument, it is the name of the tty device where we
 * should listen for multiple callers and handle login and password.
 */
main(argc,argv)
int argc;
char *argv[];
{
	int	ontheline = 1;
	clock_t start_time;
	clock_t end_time;
	int	i;
	int c;
	char tmp_str[256];
	char *poll_sys = (char *)NULL;  /* System name to poll, or none */
	int c1, c2;
	int retval;
	FILE *input_file;
	short vRefNum;
	char apname[256];
	short apWhatToDo;
	short apCount;
	AppFile appInfo;
	FInfo **apparm;
	SetApplLimit(GetApplLimit()-100000);
	_ftype = 'TEXT';
	_fcreator = 'ttxt';
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	srand(time(NULL));
	GetAppParms((unsigned char *)&apname, &apref, (Handle *)&apparm);
	CountAppFiles(&apWhatToDo, &apCount);
	if (apCount > 1)
		{
			printf("To many Documents selected on startup: %s\n", apname);
			exit(1);
			}
	if (apWhatToDo != appOpen)
		{
			logit("Can't print a connection file: %s\n", apname);
			exit(1);
			}
	
#ifdef MSDOS
	fprintf(stderr, "\n\n");
	fprintf(stderr, "UUCICO  : for MS-DOS             Ver 0787-2.05a\n");
	fprintf(stderr, "Base code from gnuucp - hoptoad version 1.27\n");
	fprintf(stderr, "Copyright 1987 Free Software Foundation, Inc.  Copying by GNU rules only,\n");
	fprintf(stderr, "e.g. copies must include source code as well as binaries!\n");
	fprintf(stderr, "Modified by Garry M. Paxinos\n");
	fprintf(stderr, "FidoNet : 135/6   the 'Eye of Osiris'   SEAdog/OPUS/Dutchie/uuslave\n");
	fprintf(stderr, "        : 135/17  Megasystems Online    OPUS/Dutchie\n");
	fprintf(stderr, "UUCP    : ...!{allegra|codas|ucf-cs}!novavax!ankh!pax\n");
	fprintf(stderr, "USNail  : % Megasystems Inc. 1075 Broken Sound Pkwy NW, Boca Raton FL 33431\n\n");

#endif
	if (signal(SIGINT, sigint) == SIG_ERR) {
		fprintf(stderr,"Couldn't set SIGINT\n");
		abort();
		}
	console_options.pause_atexit = true;
	printf("\n");
	SetWTitle((WindowPtr)(stdout->window), "\pMac/gnuucp");
	GetVol(0L, &vRefNum);
	if (apCount > 0)
    	{
    	   GetAppFiles(1, &appInfo);
		   PtoCstr((unsigned char *)appInfo.fName);
		   SetVol(0L, appInfo.vRefNum);
		   input_file = fopen((char *)appInfo.fName, "r");
		   if (input_file == NULL)
		   		{
		   			printf("Can't open input file: %s\n", (char *)appInfo.fName);
		   			exit(1);
		   			}
		   	if (fgets(input_str, 255, input_file) == NULL)
		   		{
		   			printf("Can't get input command line from dial file\n");
		   			exit(1);
		   			}
		   	if (input_str[strlen(input_str)-1] == '\n')
		   		input_str[strlen(input_str)-1] = '\0';
		   	PtoCstr((unsigned char *)apname);
		   	sprintf(tmp_str, "%s %s", apname, input_str);
		   	CtoPstr(apname);
		   	strcpy(input_str, tmp_str);
		   	tmp_str[0] = '\0';
		   	CtoPstr(input_str);
		   	sprintf(tmp_str, "%#s", input_str);
		   	argc = parse(tmp_str, input_str, &argv);
		    }
    	else
    	{
    		argc = ccommand(&argv);
    		}
	SetVol(0L, vRefNum);
  	_atexit(gnuucp_cleanup);
	debug = -1;		/* Let read_params set it if -x doesn't */
	ttynam[0] = '\0';
	/* FIXME, use getopt */
	/* scan command line arguments, kinda kludgy but it works */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		switch (argv[i][1]) {

		case 'w':
			f_wait++;
			printf("uucico: will wait for call after outbound\n");
			break;

		case 'x':
			debug = atoi(&argv[i][2]);
			break;

		case 'S':
			ignore_time_restrictions++;
		case 's':
			poll_sys = &argv[i][2];
			ontheline = 0;
			break;

		case 'e':
			loop++;
			printf("uucico: endless loop mode\n");
			break;

		case 'C':
			uuControl = &argv[i][2];
			break;

		/* Is -t needed for MSDOS?  Why?  -- hoptoad!gnu */
		case 't':
			curtemp++;
			printf("uucico: using ~uutemp.$$$ for temp file\n");
			break;
		}
	}

	/* If argument provided, use it as name of comm port */
	if (i < argc) {
		ontheline = 0;
		strcpy(ttynam, argv[i]);
	}
	if (read_params(uuControl) == FAIL)
		exit(EXIT_ERR);		/* Read control file */
	if (chdir(Spool)) {
		perror("Can't chdir to Spool directory");
		exit(EXIT_ERR);
	}
	/*
	 * If running via getty/login, our debug stdout had better
	 * go to a file, not to the usual stdout!
	 */
#ifdef HAS_GETTY
	if (DEBUG_LEVEL(0) && ontheline) {
		{
			char *mode;
			if (AppendLogfile == TRUE) 
				mode = "a";
			else
			    mode = "w";
			freopen(Loginfile, mode, stdout);
			}
	}
#endif
	/* setbuf(stdout, (char *)NULL);	Unbuffered debug output */

	/* If we are running with minimal debugging
	   then don't pause at program termination
	*/
	if (debug == 0) console_options.pause_atexit = false;
	
	/* Timestamp the long debug log */
	if (DEBUG_LEVEL(0)) {
		printf("\n\n\n\ngnuucp log on tty '%s' starting %s\n",
			ttynam, time_and_pid());
	}

	/* Log our presence so we humans reading the logs can find the
	   entries created by gnuucp. */
	logit("GNUUCP", version);
	if (poll_sys) {
		if (*poll_sys == '\0') poll_sys = (char *)NULL;
		do_rmail_queue();
		start_time = clock();
		call_system(poll_sys);
		end_time = clock();
		sprintf(tmp_str, "%ld chars/sec", 
			(total_data_processed*CLOCKS_PER_SEC)/(end_time-start_time));
		logit("Transfer rate:", tmp_str);
		uuxqt(debug);
		if (!f_wait) goto end;
	}

	do {
		/*
		 *  Set up serial channel, wait for incoming call. 
		 */

		do_rmail_queue();
		DEBUG(0, "\nRestarting\n", 0);
		/* openline(ttynam, 0);  FIXME, let user specify baudrate */

		start_time = clock();
		retval = do_session(ontheline, ttynam);

		end_time = clock();
		sprintf(tmp_str, "%ld chars/sec", 
			(total_data_processed*CLOCKS_PER_SEC)/(end_time-start_time));
		logit("Transfer rate:", tmp_str);
		uuxqt(debug);
		DEBUG(0, "\nEnd of call\n", 0);

	} while (loop && !ontheline && (retval == EXIT_OK));

end:
	return(0);
	}

debuggit()
{
#ifdef UNIX
	if (fork() == 0)
		execlp("forktest", "testing file descriptors", 0);
#endif
}

/*
 * Handle a single set of uucp expect/send strings.
 *
 * Return SUCCESS if the expect string is found, or FAIL if not.
 */
#ifdef IGNORE
sendexpect(expect, send)
	register char *expect;
	char *send;
{
	char *dasher, *dancer;

	while ((dasher = strchr(expect, '-'))
	    && (dancer = strchr(dasher+1, '-'))) {
		/* Two dashes found -- we have an expect-send-expect string */
		*dasher = '\0'; *dancer = '\0';
		if (instr(expect, dasher-expect,1) == SUCCESS)
			goto sendit;
		xlat_str(dasher+1);		/* Send the alternative */
		expect = dancer + 1;		/* Try next pair, if any */
	}
		
	if (expect[0] != '"' || expect[1] != '"' || expect[2] != '\0')
		if (instr(expect, strlen(expect), 1))
			return FAIL;

sendit:
	if (send) {
		xlat_str(send);
	}
	return SUCCESS;
}
#endif

sendexpect(expect, send)
	register char *expect;
	char *send;
{
	char *dasher, *dancer;
	/* Is there an initial -\r- string?? */
	if (expect[0] == '-' && strchr(expect+1,'-'))
		{
			dasher = strchr(expect, '-');
	    	dancer = strchr(dasher+1, '-');
	    	*dasher = '\0'; *dancer = '\0';
	    	xlat_str(dasher+1);
	    	expect = dancer+1;
	    	}
	while ((dasher = strchr(expect, '-'))
	    && (dancer = strchr(dasher+1, '-'))) {
		/* Two dashes found -- we have an expect-send-expect string */
		*dasher = '\0'; *dancer = '\0';
		if (instr(expect, dasher-expect, 1) == 0)
			goto sendit;
		xlat_str(dasher+1);		/* Send the alternative */
		expect = dancer + 1;		/* Try next pair, if any */
	}
		
	if (expect[0] != '"' || expect[1] != '"' || expect[2] != '\0')
		if (instr(expect, strlen(expect), 1))
			return FAIL;

sendit:
	if (send) {
		xlat_str(send);
	}
	return SUCCESS;
}

/*
 * translate embedded escape characters in a "send" string, and send 'em.
 */
xlat_str(msg)
	register char	*msg;
{
	register int	i  = 0,
		j  = 0;
	int	cr = 1;
	static char out[MAX_STRING+SLOP];

	gnusleep(1);		/* Old uucp did this, guess we'd better */
	while (msg[i]) {
		if (msg[i] == '\\') {
			switch (msg[++i]) {
			case 'r':            /* carriage return */
				out[j++] = 0x0d;
				break;
			case 'b':            /* Send Break */
				send_break ();
				break;
			case 'n':            /* line feed */
				out[j++] = 0x0a;
				break;
			case '\\':           /* back slash */
				out[j++] = '\\';
				break;
			case 't':            /* tab */
				out[j++] = '\t';
				break;
			case 'd':            /* delay */
				/* Print output, delay, resume */
				twrite(out, j);
				j = 0;
				if (DEBUG_LEVEL(2)) printf("(sleep)  ");
				gnusleep(1);
				break;
			case 's':            /* space */
				out[j++] = ' ';
				break;
			case 'c':            /* no CR at end */
				cr = 0;
				break;
			default:            /* don't know so skip it */
				break;
			}
			i++;
		} else {
			out[j++] = msg[i++];
		}
	if (j) twrite(out, j);
	j = 0;
	}

	if (cr)
		out[j++] = 0x0d;
	if (j) twrite(out, j);
}

/*
 * Line finder.  Given an "ACU" specifier from the L.sys file,
 * and a baud rate, determine a file name that we should try to lock
 * and open.
 *
 * On initial call, pass in (struct port *)0 as the port argument.
 * On subsequent calls, we skip past that port, which is presumed to
 * have been tried and failed.
 *
 * If we find a matching entry in the config file ports database,
 * return it; else return a dummy entry that might let the user get
 * by with specifying just the device name for hardwired lines.
 */
#ifdef FIXME
static struct port dummyport = {
	"",				/* Name -- filled in later */
	"none",				/* Modem type */
	"",				/* Dev name -- filled in later */
	0,				/* Baud rate -- filled in later */
	(struct port *)0};		/* Chain */
#endif FIXME

struct port *
findport(name, p)
	char	*name;
	struct port *p;
{

	if (p == 0)
		p = ports;
	else
		p = p->chain;

	for ( ; p; p = p->chain) {
		if (strcmp(name, p->devname) == 0)
			return p;
	}

	return p;			/* Didn't find it -- return null ptr */
}
	
struct port *
pickport(acu, baud, p)
	char	*acu;
	long	baud;
	struct port *p;
{

	if (p == 0)
		p = ports;
	else
		p = p->chain;

	for ( ; p; p = p->chain) {
		if (baud == p->baud &&
		    strcmp(acu, p->portname) == 0)
			return p;
	}

	return p;			/* Didn't find it -- return null ptr */

#ifdef FIXME
	strcpy(dummyport.portname, acu);	/* Fill in acu name */
	strcpy(dummyport.devname,  acu);	/* Fill in acu name */
	dummyport.baud = baud;
	return &dummyport;
#endif FIXME
}


/*
 * Simple dialer routine.  Needs replacement with a full blown
 * script driven dialer.  Next week maybe :-).  FIXME.
 */
dial_nbr(port, nbr)
	struct port *port;
	char  *nbr;
{
	char  dial[256];
	char  conn[256];
	clock_t start_time;
	clock_t end_time;
	SerStaRec serSta;
	int   i;

	/* Hardwired lines do no dialing. */
	if (strcmp("none", port->modemname) == 0)
		return SUCCESS;

	/* If it doesn't say hayes, we can't dial it. */
	if (strcmp("hayes", port->modemname) != 0)
		return FAIL;

	sprintf(dial, "%s %s\r", DialPrefix, nbr);
	if (DEBUG_LEVEL(2))
		printf("dialing %s at %ld baud: %s\n",
			port->devname, port->baud, dial);
	/* SerStatus(inRefNum, &serSta); */
	twrite(dial, strlen(dial));
	gnusleep(5);
	start_time = clock();
	/* gnusleep(ConnectWait); */
	/* while (!serSta.ctsHold)
			{
				HandleEvents();
				SerStatus(inRefNum, &serSta);
				} */
/* FIXME, we've got to be more flexible here... */
/* sprintf(conn, "CONNECT %s\r", baud);*/
	sprintf(conn, "CONNECT");

	
	for (end_time = clock(), i = FAIL; 
		(end_time - start_time)/CLOCKS_PER_SEC < ConnectWait;
		end_time = clock())
		{
			i = instr(conn, strlen(conn), 7);
			if (i == SUCCESS)
				break;
			}
	/* FIXME, scan out the baud rate, or whatever's before the CR, here.
	 */

	if (i == SUCCESS) 
		gnusleep(2);	/* Let the line settle. */

	return (i);
}

/*
 * Call a specific system, or all systems that have work pending.
 */
call_system(sys)
	char	*sys;
{
	FILE	*lsys;
	char	buf[MAX_LSYS];
	char 	tmp_str[256];
	char	sysnam[MAX_HOST];
	char	prev_name[MAX_HOST];
	int	called = FAIL;

	/*
	 * Unix uucico just reads the directory, and calls the systems
	 * in the order of the files in the directory.  We want more
	 * control than that, though I'm not sure that L.sys order is
	 * best either.  For example, in the first call after 11PM,
	 * I'd like to call the sites that haven't been callable before
	 * 11PM first, and finish up with the ones I've been able to call
	 * all day.  FIXME.
	 */
	/* sprintf(tmp_str,"%s:%s", Spool, "L.sys"); */
	if (! (lsys =fopen(Sysfile, "r"))) {
		DEBUG(0, "uucico: can't open L.sys, errno %d\n", errno);
		return 0;
	}
	sysnam[0] = '\0';		/* Initially, no previous sys */

	/* Once per system in L.sys... */
	/* FIXME, handle continuation lines (trailing "\") */
	while (get_sysline(buf, sizeof(buf), lsys))
		{
		/*
		 * Grab the system name.  If same as previous, and
		 * the previous call worked, skip it.
		 */
		strcpy(prev_name, sysnam);
		(void) sscanf((char *)buf, "%s", sysnam);
		if (!strcmp(sysnam, prev_name)) {
			if (called == SUCCESS) continue;
		}

		/*
		 * If a system name was specified, skip til we find it
		 * If none was specified, only call if there is work.
		 */
		if (sys) {
			if (0 != strcmp(sys, sysnam))
				continue;
		} else {
			DEBUG(4,"searching for outbound to %s\n", sysnam);

			if (!work_scan(sysnam, "C")) {
				DEBUG(3,"no work for %s\n", sysnam);
				called = SUCCESS;	/* Don't try further */
				continue;
			}

			DEBUG(3, "found work for %s\n", sysnam);
		}

		called = call_sysline(buf);

		if (called == SUCCESS && sys) break;
	}

	fclose(lsys);
	if (called == FAIL && sys)
		DEBUG(0, "Could not call system %s\n", sys);
	return 0;
}

char *get_sysline(buf, size, lsys)
char *buf;
int size;
FILE *lsys;
{
	char *start;
	start = buf;
	buf[0] = '\0';
	while (fgets(buf, size, lsys))
	{
		if (buf[strlen(buf)-1] == '\n')
			{
				buf[strlen(buf)-1] = '\0';
				}
		if (buf[0] == '#')
			continue;
		if (buf[strlen(buf)-1] == '\\')
			{
				buf[strlen(buf)-1] = ' ';
				buf = buf+strlen(buf);
				continue;
				}
		return(start);
		}
	if (strlen(buf) == 0) 
		return(NULL);
	return(start);
	}

/*
 * Call out to a system, given its L.sys line.
 */
int
call_sysline(lsysline)
	char	*lsysline;
{
	char	tempname[MAX_HOST + 30 + SLOP],
		*sysnam,
		*times,
		*acu,
		*sbaud,
		*telno,
		*send,
		*expect;
	struct port *port;
	char	logbuf[MAX_HOST+30+SLOP];
	static char	msgbuf[MAX_STRING+SLOP];
	long	baud;

	who[0] = '-'; who[1] = '\0';	/* No user now (for logit) */

	/* FIXME, use the values it is ignoring here */
	sysnam = strtok(lsysline, " ");
	times = strtok((char *)NULL, " ");	/* Time */
	acu = strtok((char *)NULL, " ");	/* ACU */
	sbaud = strtok((char *)NULL, " ");	/* Baud */
	telno = strtok((char *)NULL," ");	/* phone */

	strcpy(host_name, sysnam);

	if (!ignore_time_restrictions) {
		/* FIXME, check the time parameter and return FAIL if
		 * it does not allow calls now.  Meanwhile, bounce
		 * all calls unless -S is specified. */
		logit("WRONG TIME TO CALL", sysnam);
		return FAIL;
	}

	/* baud = atoi(sbaud); */
	sscanf(sbaud, "%ld", &baud);
	port = (struct port *)0;
	for (;;) {
		port = pickport(acu, baud, port);
		if (!port)
			return FAIL;		/* Tried them all */
		DEBUG(2, "Opening outgoing line %s\n", port->devname);
		if (openout(port) != SUCCESS) {
			logit("CAN'T USE", port->devname);
		} else {
			break;
		}
	}

	sprintf(logbuf, "%s %s %d", host_name, port->devname, port->baud);
	DEBUG(1, "Trying %s\n", logbuf);

	if ((dial_nbr(port, telno) != SUCCESS)) {
		ttyunlock();
		logit("FAILED", logbuf);
		return FAIL;
	}

	logit("DIALED", logbuf);

	/*
	 * Process send-expect strings.
	 */
	while (expect = strtok((char *)NULL, " ")) {
		send = strtok((char *)NULL, " ");
		if (sendexpect(expect, send) != SUCCESS)
			goto bort1;
	}

	/*
	 * FIXME, there should be a way to detect login/passwd
	 * failure here and keep doing the script rather than
	 * continuing to expect Shere at another login: prompt.
	 */
	sprintf(logbuf, "call to %s", host_name);
	logit("SUCCEEDED", logbuf);

	/* wait for Shere message, grab whatever is sent, send response */
	{
		char *msgptr;
		sprintf(tempname, "Shere");
		/* if (instr(tempname, strlen(tempname), 1))
			goto bort1; */
	again:
		if (getstring(msgbuf) != SUCCESS)
			goto bort1;
		/* printf("MSGBUF: %s\n", msgbuf); */
		msgptr = strstr(msgbuf, tempname);
		/* printf("msgptr: %s\n", msgptr); */
		if (msgptr == NULL) goto again;
		msgptr = msgptr + strlen(tempname);
		if (msgptr[0] == '=')
			{
				if (msgptr[1] != '\0')
					{
						if (!!strcmp(host_name, msgptr+1)) 
							{
								logit("WRONG HOST", msgptr+1);
								goto bort1;
								}
						}
				} 
			else 
			 {
			 	if (msgptr[0] != '\0') 
					{
						DEBUG(0, "strange Shere%s\n", msgbuf);
						}
				}
		/* printf("MSGBUF+ofs+count: %s\n", msgptr); */
		}
	sprintf(tempname, "\20S%s\0", Myname);
	twrite(tempname, strlen(tempname)+1); /* Including null */

	/* wait for ok message */
	if (getstring(msgbuf) != SUCCESS)
		goto bort1;
	if (msgbuf[0] != 'R')
		goto bort1;
	if (strcmp("OK", msgbuf+1)) {
		logit("HANDSHAKE FAILED", msgbuf+1);
		goto bort1;
	}

	/*
	 * Get Protocol string, make sure it supports our protocol.
	 * FIXME, should scan our table versus theirs.
	 */
	if (getstring(msgbuf) != SUCCESS)
		goto bort1;
	if (msgbuf[0] != 'P')
		goto bort1;
	if (!strchr(msgbuf+1, curproto->p_id)) {
		logit("PROTOCOL UNSUPPORTED", msgbuf);
		goto bort1;
	}
	twrite(msgi3, sizeof(msgi3)-1);	/* FIXME, build real reply */

	if (turnon(1))
		goto bort1;

	logit("OK", "startup");

	top_level(1);
	hangup(port);
	return SUCCESS;

bort1:
	hangup(port);
	return FAIL;
}

/* Handle a single uucp [slave] login session */
/* =Ned Horvath= ech@pegasus.att.com
   modified this to be compatible with SVR3 wrt the
   line end characters.
*/

do_session(ontheline, ttynam)
	int ontheline;
	char *ttynam;
	{
	static char	trash[MAX_STRING+SLOP];	/* Incoming trash buffer */
	char tmp_str[256];
	char ch;
	SerStaRec serSta;
	struct port *p;
	p = (struct port *)0;
	if (!ontheline) {
		serSta.ctsHold = 0;
		p = findport(ttynam, p);
		if (p == (struct port *)NULL)
			{
				logit("Can't find port to", "open");
				return(EXIT_ERR);
				}
		openin(p);
		/* Look for signs of life on line */
		while (instr("CONNECT",7, 7));
		twrite(msgo0,sizeof(msgo0)-1);
			sprintf(tmp_str, "%s", UUCPlogname);  /* ech: was "%s\r" */
			if (instr(tmp_str,strlen(tmp_str), 1)) 
				{
			badLogname:
					printf("uucico: invalid login name\n");
					goto bort;
					}
			switch (xgetc()) 
				{ /* ech: verify \r or \n */
					case '\r':
					case '\n':
						break;
					default:
						goto badLogname;
					}
			/* output password request, verify password */
			twrite(msgo1,sizeof(msgo1)-1);
			sprintf(tmp_str, "%s", UUCPpasswd);   /* ech: was "%s\r" */
			if (instr(tmp_str,strlen(tmp_str), 1)) 
				{
					badPW:
						printf("uucico: invalid password\n");
						goto bort;
					}
			switch (xgetc()) 
				{   /* ech: verify \r or \n */
					case '\r':
					case '\n':
						break;
					default:
						goto badPW;
					}
		printf("uucico: correct login\n");
	}
	if (p == (struct port *)NULL)
	{
		logit("Can't find port to", "open");
		return(EXIT_ERR);
		}
	/* output here message, wait for response */
	sprintf(tmp_str, "\20Shere=%s", Myname);
	twrite(tmp_str,strlen(tmp_str)+1); /* Include NULL at end */
/* FIXME, handle this kludge */
/*	if (instr(msgi2,sizeof(msgi2)-1), 1) */
	if (getstring(trash) != SUCCESS || trash[0] != 'S')
		goto bort;

	strcpy (host_name, strtok(trash, "\20S "));

	/* output ROK message, output protocol request, wait for response */
	twrite(msgo3,sizeof(msgo3)-1);

	/* FIXME, make the protocol list here, and use it */
	twrite(msgo3a,sizeof(msgo3a)-1);
	if (getstring(trash) != SUCCESS) goto bort;
	if (!strchr(trash+1, curproto->p_id)) 
		{
			logit("PROTOCOL UNSUPPORTED", trash);
			goto bort;
			}
	/* if (instr(msgi3,sizeof(msgi3)-1), 1)
		goto bort; */
	if (turnon(0)) goto bort;
	logit("OK", "startup");
	top_level(0);		/* Returns SUCCESS or FAIL, we dont care */

bort:
	hangup(p);
	printf("uucico: call complete\n");
	return(EXIT_OK);
}

/*
 * Handle transactions "at top level", as Unix uucp's debug log says.
 *
 * As master, we scan our queues for work and send requests to the
 * other side.  When done, we send a hangup request and switch to slave mode.
 *
 * As slave, we accept requests from the other side; when it is done,
 * it sends a hangup request, and we switch to master mode, if we have
 * any work queued up for that system.
 *
 * This repeats as long as either side has work to do.  When all the
 * queued work is done, we agree to hang up, terminate the packet protocol,
 * and return to the caller.  (We still haven't hung up the phone line yet.)
 *
 * A curious feature of the hangup protocol is that it is not a simple
 * question-answer.  The master says "H", asking about hangup.  The
 * slave responds "HY" saying OK.  The master then says "HY" also,
 * then both of them hang up.  Maybe this is to make sure the first HY
 * got ack'ed?  Anyway, an "H" is reported as HANGUP and an "HY" as
 * HANGNOW.  After we send an HY, we go back to listening for commands;
 * if the master sends something other than HY, we'll do it.
 */
#define	HANGUP	2		/* Signal to switch master/slave roles */
#define	HANGNOW	3		/* Signal to hang up now */
#define	COPYFAIL	4	/* File copy failed */

int
top_level(master_mode)
	int master_mode;
{
	char	buf[MAXMSGLEN];		/* For hangup responses */

	if (master_mode) {
		(void) work_scan(host_name, "C"); /* Kick off queue scan */
		goto master;
	}

	for (;;) {
		/* Slave side */
	slave:
		for (;;) {
			DEBUG(4, "*** TOP *** - slave\n", 0);
			switch (do_one_slave()) {
			case SUCCESS:
				break;
			case FAIL:
				return FAIL;
			case HANGUP:
				if (work_scan(host_name, "C")) {
					if (wrmsg('H', "N", fdtty))
						return FAIL;
					goto master;
				} else {
					if (wrmsg('H', "Y", fdtty))
						return FAIL;
					break;
				}
			case HANGNOW:
				goto quit;
			}
		}
	
		/* Master side */
	master:
		for (;;) {
			DEBUG(4, "*** TOP *** - master\n", 0);
			switch (do_one_master()) {
			case SUCCESS:
				break;
			case FAIL:
				return FAIL;
			case HANGUP:
				/* We wrote an H command, what's the resp? */
				if ((long)rdmsg(buf, fdtty) != (long)SUCCESS)
					return FAIL;
				if (buf[0] != 'H')
					return FAIL;
				if (buf[1] == 'N')
					goto slave;
				else {
					/* Write the final HY */
					if (wrmsg('H', "Y", fdtty))
						return FAIL;
					goto quit;
				}
			}
		}
	}

quit:
	/* Shut down the packet protocol */
	turnoff();

	/* Write the closing sequence */
	twrite(msgo4, sizeof(msgo4)-1);
	(void) instr(msgi4, sizeof(msgi4)-1, 1);

	twrite(msgo4, sizeof(msgo4)-1);

	logit("OK", "conversation complete");

	return SUCCESS;   /* Go byebye */
}

/*
 * We are slave; get a command from the other side and execute it.
 *
 * Result is SUCCESS, FAIL, HANGUP, or HANGNOW.
 */
int
do_one_slave()
{
	char msg[MAXMSGLEN];		/* Master's message to us */

	/* Get master's command */
	if ((long)rdmsg(msg, fdtty) != (long)SUCCESS)
		return FAIL;

	/* Print it for easy debugging */
	DEBUG(4,"Command: %s\n\n", msg);

	switch (msg[0]) {

	case 'S':
		if (msg[1] != ' ') break;
		return host_send_file(msg);

	case 'R':
		if (msg[1] != ' ') break;
		return host_receive_file(msg);

	case 'X':
		/* Cause uuxqt to run (on certain files?)
		 * See Protocol.doc for sketchy details.
		 */
		break;

	case 'H':
		if (msg[1] == '\0') return HANGUP;
		if (msg[1] == 'Y')  return HANGNOW;
		if (msg[1] == 'N')  return SUCCESS;	/* Ignore HN to slave */
		break;

	}

	/* Unrecognized packet from the other end */
	DEBUG(0, "Bad control packet refused: %s\n", msg);
	if (yesno(msg[0], 0, 0))	/* FIXME: return error code */
		return FAIL;
	return SUCCESS;
}

/*
 * Do one piece of work as master.
 *
 * FIXME:  we don't handle the flags, e.g. -c, properly!
 */
int
do_one_master()
{
	FILE	*fd;
	char	*sname;
	char	cmnd[10];		/* Command character */
	char 	buf[256];
	char	tmp_str[256];
	int	fail;
	int	num;
	char	notify[NAMESIZE];	/* A bit large...FIXME */
			/* FIXME: do the notify stuff */

	sname = work_next();
	if (!sname) {
		/* No more work, time to hang up. */
		if (wrmsg('H', "", fdtty))
			return FAIL;
		return HANGUP;
	}

	DEBUG(4, "Request file %s\n", sname);
	sprintf(tmp_str,"%s:%s", Spool, munge_filename(sname));
	fd = fopen(tmp_str, "rb");
	if (fd == NULL) {
		DEBUG(0, "uucico: couldn't open %s\n", sname);
		/* fail = local_receive_file(buf); */
		fail = local_receive_file();
		return SUCCESS;
	}
	setvbuf(fd, NULL, _IOFBF, 4096);
	while (fgets(buf, sizeof buf, fd)) {
		DEBUG(4, "Queued request: %s", buf);
		if (buf[1] != ' ') goto badnum;
		num = sscanf((char *)buf, "%s %s %s %s %s %s %o\n",
			cmnd, srcnam, dstnam, who, flags, temp, &mode, notify);

		switch (cmnd[0]) {
		case 'S':
			if (num < 7 || num > 8) goto badnum;
			fail = local_send_file(buf);
			break;

		case 'R':
			if (num != 5) goto badnum;
			break;

		default: badnum:
			DEBUG(0, "Unknown/invalid queued request: %s\n",
				 buf);
			goto badline;
		}

		/* FIXME, what does uucp do if one of N xfers fails? */
		if (fail == FAIL) {
			/* protocol over the wire is failing */
			fclose(fd);
			return FAIL;
		}
		if (fail) {
	badline:
			logit("ERROR IN WORK FILE", sname);
			logit("BAD LINE IS", buf);
		}
	}
	fclose(fd);
	/* Zap the queue file */
	sprintf(tmp_str,"%s:%s", Spool, munge_filename(sname));
	fail = remove(tmp_str);
	if (fail != 0) {
		logit("CAN'T REMOVE WORK FILE", sname);
		DEBUG(0, "Can't remove, errno %d\n", errno);
	} else {
		DEBUG(4, "Removed work file %s\n", sname);
	}
	return SUCCESS;
}

/* Send a "yes or no" packet with character 'c'. */
int
yesno(c, true, err)
	char c;
	int true;
	int err;
{
	char buf[20];

	buf[0] = true? 'Y': 'N';
	buf[1] = 0;
	if (err && !true) 
		sprintf(buf+1,"%d", err);

	return wrmsg(c, buf, fdtty);
}

/*
 * Master wishes to send a file to us -- we receive it.
 * Return 1 to abort the call, 0 to continue.
 */
int
host_send_file(msg)
char  *msg;
{
	FILE *fddsk;			/* Disk file pointer */
	char tmp_str[256];
	char	cmnd[10];		/* Command character */

	sscanf((char *)msg,"%s %s %s %s %s %s %o",
		cmnd, srcnam, dstnam, who, flags, temp, &mode);
	logit("-> REQUESTED", msg);
	strcpy (dstnam, munge_filename(dstnam));  /* Translate to local name */
	strcpy (temp, temp_filename(dstnam));     /* Create a handy temp file */

	/* FIXME: deal with file modes now that we fopen. */
	sprintf(tmp_str, "%s:%s", Spool, temp);
	fddsk = fopen(tmp_str, "wb" /*, mode|0600 */);
	if (fddsk == NULL) {
		/* Can't open file -- send error response */
		if (DEBUG_LEVEL(0)) {
		    printf(
			"Cannot open temp file %s (%s) for writing, errno=%d\n",
				temp, dstnam, errno);
		}
		logit("-> REQUEST", "FAILED -- TEMP FILE");
		if (yesno('S', 0, 4))
			return FAIL;
		return SUCCESS;
	}
	setvbuf(fddsk, NULL, _IOFBF, 4096);
	
	/* FIXME: Are the above permissions right?? */
	/* FIXME: Should we create directories for the file? */
	if (yesno('S',1, 0))    /* Say yes */
		return 1;

	return receive_file(fddsk, temp, dstnam, srcnam);
}

/*
 * Master wants to Recieve a file from us -- we send it.
 * Return 1 to abort the call, 0 to continue.
 */
host_receive_file(msg)
char  *msg;
{
	FILE *fddsk;     /* Disk file descriptor */
	char tmp_str[256];
	int x;
	char	cmnd[10];		/* Command character */

	logit("<- REQUESTED", msg);

	sscanf((char *)msg,"%s %s %s",cmnd,srcnam,dstnam);
	strcpy (temp, munge_filename(srcnam));
	sprintf(tmp_str, "%s:%s", Spool, temp);
	fddsk = fopen(tmp_str, "rb");		/* Try to open the file */
	if (fddsk == NULL) {
		/* File didn't open, sigh. */
		if (DEBUG_LEVEL(0)) {
		    printf("Cannot open file %s (%s) for reading, errno=%d\n",
				temp, srcnam, errno);
		}
		logit("<- DENIED", "CAN'T OPEN");
		if (yesno('R', 0, 2))
			return 1;
		return 0;
	}
	setvbuf(fddsk, NULL, _IOFBF, 4096);
	

	if (yesno('R',1, 0))    /* Say yes */
		return 1;

	x = send_file(fddsk);
	switch (x) {
	default:
		return x;

	case COPYFAIL:
		/* We don't care if the copy failed, since the master
		   asked for the file and knows the result. */
		return SUCCESS;
	}
}

/*
 * We, as master, want to Send a file.
 *
 * Return FAIL, SUCCESS, or COPYFAIL.
 * SUCCESS is returned either if the file was not found locally (local
 * error, and the queued transfer should be flushed) or if it was moved
 * successfully.  COPYFAIL indicates that the queued transfer should be
 * left queued, and later retried.  FIXME, there are several failure points
 * in the transaction (see Protocol.doc) and we need finer control here.
 */
int
local_send_file(workstr)
	char *workstr;
{
	char buf[MAXMSGLEN];	/* Used for both xmit and receive */
	FILE *fddsk;     /* Disk file descriptor */
	char tmp_str[256];
	int res, status;	/* Result and file removal status */

	/* WHY are temp and srcnam switched?  FIXME!  And no notify? */
	sprintf(buf,"S %s %s %s %s %s 0%o %s",
		temp, dstnam, who, flags, srcnam, mode, who);

	logit("<- REQUEST", buf);

	if (strchr(flags, 'c')) {
		strcpy(temp, munge_filename(srcnam));
	} else {
		strcpy(temp, munge_filename(temp));
	}
	sprintf(tmp_str, "%s:%s", Spool, temp);
	fddsk = fopen(tmp_str, "rb");
	if (fddsk == NULL) {
		/* FIXME -- handle queued request for nonexistent file */
		if (DEBUG_LEVEL(0))
			printf("Can't open file %s (%s), errno=%d\n",
				temp, srcnam, errno);
		logit("<- NOT FOUND", temp);
		return COPYFAIL;	/* FIXME caller won't deal with this */
	}
	setvbuf(fddsk, NULL, _IOFBF, 4096);
	
	/* Tell the other side we want to send this file */
	if (wrmsg('S', buf+1, fdtty) != SUCCESS) {
		DEBUG(0, "problem sending request\n", 0);
		return FAIL;
	}

	/* See what they have to say about it */
	if ((long)rdmsg(buf, fdtty) != (long)SUCCESS)	
		return FAIL;
	if ((buf[0] != 'S') || (buf[1] != 'Y')) {
		logit("<- REQUEST DENIED", buf);
		return COPYFAIL;
	}
	res = send_file(fddsk);	/* FAIL, SUCCESS, or COPYFAIL */

	/* Delete the source file if it was just a copy */
	if (res != SUCCESS)
		return res;
	if (strchr(flags, 'c'))		/* If copied direct from source */
		return res;		/* ...just return. */
	status = remove(tmp_str);		/* Delete uucp's copy of the file */
	if (status != 0) {
		logit("<- CAN'T REMOVE SENT FILE", temp);
		DEBUG(0, "Can't remove, errno %d\n", errno);
	} else {
		DEBUG(4, "Removed sent file %s\n", temp);
	}
	return res;
}

/*
 * We want to Receive a file -- so we ask for it.
 * Return 1 to abort the call, 0 to continue.
 */
int
local_receive_file()
{
	char buf[MAXMSGLEN];
	FILE *fddsk;			/* Disk file pointer */
	char tmp_str[256];

	/* FIXME, test dest file access before we ask for it. */

	sprintf(buf,"R %s %s %s %s %s 0%o %s",
		srcnam, dstnam, who, flags, temp, mode, who);

	strcpy (dstnam, munge_filename(dstnam));  /* Translate to local name */
	strcpy (temp, temp_filename(dstnam));     /* Create a handy temp file */

	/* FIXME: deal with file modes now that we fopen. */
	/* FIXME: Are the above permissions right?? */
	/* FIXME: Should we create directories for the file? */
	sprintf(tmp_str, "%s:%s", Spool, temp);
	fddsk = fopen(tmp_str, "wb" /*, mode|060 */);

	if (fddsk == NULL) {
		/* Can't open temp file -- send error response */
		if (DEBUG_LEVEL(0)) {
		    printf(
			"Cannot open temp file %s (%s) for writing, errno=%d\n",
				temp, dstnam, errno);
		}
		logit("-> REQUEST", "FAILED -- TEMPFILE");
		return FAIL;
	}
	setvbuf(fddsk, NULL, _IOFBF, 4096);
	
	logit("-> REQUEST", buf);
	if (wrmsg('R', buf+1, fdtty) != SUCCESS) {
		printf("uucico: problem sending request\n");
		return FAIL;
	}

	/* See what the other side has to say about it */
	if ((long)rdmsg(buf, fdtty) != (long)SUCCESS)
		return FAIL;
	if ((buf[0] != 'R') || (buf[1] != 'Y')) {
		logit("-> REQUEST DENIED", buf);
		return SUCCESS;	/* FIXME, should do something more here */
	}

	return receive_file(fddsk, temp, dstnam, srcnam);
	/* FIXME - We should deal with files that didn't get there */
}

/* general file receive routine */
int
receive_file(fddsk, temp, dstnam, srcnam)
	FILE *fddsk;			/* Disk file pointer */
	char	*temp, *dstnam, *srcnam;
{
	int status;
	char tmp_str1[256];
	char tmp_str2[256];
	char err_str[30];
	int error = 0;			/* No errors so far */

	if (rddata(fdtty, fddsk) != SUCCESS) error++;
	status = fclose(fddsk);		/* Make sure the data got here */
	FlushVol(NULL, 0);
	if (status != 0) {
		error++;
		DEBUG(0, "fclose errno=%d\n", errno);
	}

	/* Move the file from its temp location to its real loc */
	/* FIXME:  This needs to be able to copy the file, if 
	   a simple rename does not suffice. */
	/* FIXME:  should create directories if necessary, e.g. D.
	   or subdirs of /usr/spool/uucppublic. */
	/* FIXME:  should use source name if target is a directory e.g. ~/ */
	sprintf(tmp_str1, "%s:%s", Spool, temp);
	sprintf(tmp_str2, "%s:%s", Spool, dstnam);
	remove(tmp_str2);
	status = rename(tmp_str1, tmp_str2);
	if (status != 0) {
		error++;
		if (DEBUG_LEVEL(0)) {
			logit("-> Cannot rename file", temp);
			logit("-> to", dstnam);
			sprintf(err_str, "%d", errno);
			logit("-> Error Code", err_str);
		}
	}

	logit("-> COPY", error? "FAILED": "SUCCEEDED");
	if (yesno('C', error == 0, 5))	/* Send yes or no */
		return FAIL;
	return SUCCESS;
}

/*
 * general file send routine
 * Return SUCCESS, FAIL, or COPYFAIL.
 */
int
send_file(fddsk)
	FILE *fddsk;     /* Disk file pointer */
{
	char ansbuf[MAXMSGLEN];

	if (wrdata(fddsk, fdtty) != SUCCESS)
		return FAIL;
	(void) fclose(fddsk);
	FlushVol(NULL, 0);
	/* Await the "CY" or "CNddd" packet, and toss it. */
	while (1) {
		if ((long)rdmsg(ansbuf, fdtty) != (long)SUCCESS)	
			return FAIL;
		if (ansbuf[0] != 'C') {
			DEBUG(0,"\nDidn't get 'CY' or 'CN', got %s\n",
				ansbuf);
			/* and loop looking for C message */
		} else if (ansbuf[1] == 'Y') {
			logit("<- REQUESTED", ansbuf);
			return SUCCESS;
		} else {
			logit("<- COPY FAILED", ansbuf);
			return COPYFAIL;
		}
	}
}

