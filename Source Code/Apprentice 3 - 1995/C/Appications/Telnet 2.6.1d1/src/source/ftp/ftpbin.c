/*
*  User FTP
*  6/8/87
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*/

#ifdef MPW
#pragma segment FTPClient
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "TelnetHeader.h"
#include "wind.h"
#include "netevent.proto.h"
#include "InternalEvents.h"
#include "VSkeys.h"

#include "network.proto.h"
#include "mydnr.proto.h"
#include "bkgr.proto.h"
#include "maclook.proto.h"

#include "vsdata.h"
#include "vsinterf.proto.h"

#include "menuseg.proto.h"
#include "rsmac.proto.h"
#include "macbinary.h"
#include "macutil.proto.h"
#include "binsubs.proto.h"
#include "ftppi.h"										   /* list of commands, help strings */
#include "ftpbin.proto.h"
#include "parse.proto.h"
#include "maclook.proto.h"
#include "netevent.proto.h"
#include "debug.h"
#include "Connections.proto.h"

//#define	DEBUG_FTPCLIENT
#ifdef	DEBUG_FTPCLIENT
#define	ftpclient_debug_print(x) putln(x)
#else
#define ftpclient_debug_print(x)
#endif


#define FASCII 0
#define FIMAGE 1										   /* This is different than the definition
														      in bkgr.c */
#define FAMODE 0
#define FIMODE 1
//#define FMMODE 2										   /* Mac Binary, when ready */

#define HFTP  21

#define SUCCESS	   2
#define ERROR	  -1
#define NONE	  -2
#define ABORT	  -3
#define INCOMPLETE -4
#define AMBIGUOUS -5
#define HAVEDATA   4

#define BUFFERS 8000									   /* size of buffer */
#define PATHLEN 256
#define READSIZE 128									   /* how much to read */

#define HELPRESID 23229

long
        filelen = 0,									   /* file transfer length */
        start = 0L,										   /* timing var */
        towrite = 0,									   /* file transfer pointer */
        translen = 0,									   /* length of file transfered */
        transtime;

short
        xp = 0,											   /* general pointer */
        ftpfh,											   /* file handle for ftp */
        fcnt = 0,										   /* counter for ftpd */
        ftpfilemode = 0,								   /* file open mode for transfer */
        hash = 0,										   /* hash mark printing */
        sendport = 1,									   /* to send ports or not */
        verbose = 1,									   /* informative messages */
        bell = 0,										   /* sound bell */
        autologin = 1,									   /* login on connect */
        prompt = 1,										   /* check on multiple commands */
        glob = 1,										   /* expand wildcards */
        slashflip = 1,									   /* change \ to / */
        fromtty = 1;									   /* default input from tty */

char	Template[256];
CInfoPBRec	ClientDirectoryState;

static short ftpdata;
static char bellchar = {7};
static char backspace = {8};
static char poundsign = {'#'};

FILE*	fromfp = NULL;						/* file pointer for input */
MBFile*	ftp_mbfp;

unsigned char
       *captlist = 0L, *newfile = 0L, *transfile = 0L,	   /* name of file being sent/received */
       *xs = 0L;										   /* response string */
unsigned short curftpprt = 0;								   /* port to use */
char    response_data[20][4];							   /* BYU 2.4.12 - first 4 characters of
														      each response */
unsigned char response_count[20];						   /* BYU 2.4.12 - non-zero indicates
														      waiting for response */

extern short scrn;
extern WindRec *screens;

#define RCPSEGSIZE 512
#define O_RAW O_RDONLY
#define EOLCHAR 13
#define captlistsize 2001L

void	FTPClientunload(void) {}

/* putstring: display string using vt100 emulation routines				*/
void	putstring (short ftpscrn, char *string)
{
	for (; *string; string++)
		parse (&screens[ftpscrn], (unsigned char *) string, 1);
}

char   *stpblkp (char *mystring)
{
	/* Return pointer to first non blank in string */
	short	mycnt;
	char	*ptr;

	mycnt = 0;
	ptr = mystring;
	while (((*ptr == ' ') || (*ptr == '\t')) && (mycnt < 256)) {
		ptr++;
		mycnt++;
	}
	return (ptr);
}

char   *stptok (char *mystring, char *myword, short mylen, char *myterms)
{
	/* Return pointer to first non blank in string */
	short	done, i;
	char	*p, *q;

	p = stpblkp (mystring);								   /* skip leading blanks */
	i = 0;
	done = 0;
	while (*(p + i) && (i <= mylen) && (done == 0)) {
		q = myterms;
		while ((*q) && (*q != *(p + i)))
			q++;
		if (!(*q)) {
			*(myword + i) = *(p + i);
			i++;
		}
		else
			done = 1;
	}
	*(myword + i) = '\0';
	q = p + i;
	return (q);
}

short ftpopen(Str255 name)
{
	OSErr	myerr;
	short	myrefnum;
	
	myerr = HOpen (TelInfo->FTPClientVRefNum, TelInfo->FTPClientDirID, name, fsRdWrPerm, &myrefnum);
	return (myrefnum);
}

/* End of Macintosh Routines created at BYU, 1988 */

/*	ftpgets - read a line from the keyboard 
	returns ABORT if aborted, non-zero on success */

char	ftpgets (char *s, short lim, short echo)
//char   *s;									/* where to put the line */
//short   lim, echo;							/* max chars to read, echo? */
{
	int			count, i;
	unsigned	char c;
	char		*save;
	Boolean		gotOne;										   /* Did we get an event */
	EventRecord	myEvent;								   /* Event Record for this loop */

	count = 0;											   /* none read */
	save = s;											   /* beginning of line */

	while (1) {

		gotOne = WaitNextEvent (everyEvent, &myEvent, gApplicationPrefs->TimeSlice, 0L);

		if ((myEvent.what == keyDown) || (myEvent.what == autoKey))
			c = myEvent.message & charCodeMask;
		else {
			Stask ();									   /* BYU 2.4.15 */
			DoNetEvents ();								   /* process event queue */
			c = 0;
		}

		switch (c) {									   /* allow certain editing chars */
		case 8:										   /* backspace */
			if (count) {
				if (echo) {
					parse (&screens[scrn], &c, 1);
					parse (&screens[scrn], (unsigned char *) " ", 1);
					parse (&screens[scrn], &c, 1);
				}
				count--;								   /* one less character */
				s--;									   /* move pointer backward */
			}
			break;
		case 13:										   /* carriage return, = ok */
			if (echo)
				putstring (scrn, "\015\012");			   /* newline */
			*s = '\0';									   /* terminate the string */
			return (c);									   /* return ok */
			break;
		case 21:										   /* kill line */
			for (i = 0; i < s - save; i++) {			   /* length of line */
				if (echo) {								   /* erase */
					parse (&screens[scrn], (unsigned char *) &backspace, 1);
					parse (&screens[scrn], (unsigned char *) " ", 1);
					parse (&screens[scrn], (unsigned char *) &backspace, 1);
				}
			}
			s = save;									   /* reset line */
			break;
		case 0:										   /* do nothing */
			break;
		default:										   /* not special char */
			if (c > 31 && c < 127) {					   /* printable */
				if (echo)
					parse (&screens[scrn], &c, 1);		   /* display */
				*s++ = c;								   /* add to string */
				count++;								   /* length of string */
			}
			else										   /* acts as eol */
				return (c);								   /* value of special char */

			if (count == lim) {							   /* to length limit */
				*s = '\0';								   /* terminate */
				return (c);
			}
			break;
		}

	}
}

/************************************************************************/
/* captcon capture everything from a connection into the capture list return -1 on closed
   connection, else 0, 1 if paused 
/*********************************************************************** */

short captcon(short cnum, short mywindow)
{
	short			capturesize, cnt = 0;
	unsigned	char s[80];								   /* temporary string */

	/* if (fromtty && n_scrlck()) return(TRUE);	/* if paused, nothing to do */
	capturesize = screens[mywindow].capturesize;
	do {
		cnt = netread (cnum, &s[0], 64);				   /* get some from queue */
		if ((cnt + capturesize) < captlistsize) {
			BlockMove (&s[0], &captlist[capturesize], (long) cnt);
			capturesize += cnt;
		}

		/* demux all packets */
	} while ((cnt > 0) && ((cnt + capturesize) < captlistsize));

	captlist[capturesize] = 0;
	screens[mywindow].capturesize = capturesize;

	return (cnt);										   /* 0 normally, -1 if connection closed */
}

/************************************************************************/
/* telnet filter telnet options on incoming data
/*********************************************************************** */

short	telnet (int port, int cnt, unsigned char *s)
{
	register int i, mylen, myport;
	char    printline[256];								   /* line to display */

	myport = WindByPort (port);
	for (i = 0; i < cnt; i++) {							   /* put on screen */
		if (*(s + i) & 128) {							   /* if over ASCII 128 */
			sprintf (printline, " %d ", *(s + i));		   /* show as number */
			mylen = strlen (printline);
			parse (&screens[myport], (unsigned char *) &printline[0], mylen);
		}
		else
			parse (&screens[myport], (s + i), 1);
	}
	return (TRUE);
}

/************************************************************************/
/* 	dumpcon take everything from a connection and send it to the screen
	return -1 on closed connection, else 0, 1 if paused
/*********************************************************************** */

int dumpcon (int cnum, int mywindow)
{														   /* BYU 2.4.12 */
	short   i1, i2, cnt = 0, result = 0;				   /* BYU 2.4.12 */
	unsigned char s1[64], s2[128];						   /* temporary strings */

	/* if (fromtty && n_scrlck()) return(TRUE);		/* if paused, nothing to do */

	do {
		cnt = netread (cnum, &s1[0], 64);				   /* get some from queue */

		/* Search the incoming data for status codes and newlines */
		i1 = 0;
		i2 = 0;
		while (i1 < cnt) {

			/* Check for 3 digit codes beginning with "5" at the beginning of each line. */
			if (s1[i1] == '\012' || s1[i1] == '\015') {	   /* BYU 2.4.12 */
				response_count[mywindow] = 0;			   /* BYU 2.4.12 */
			}
			else if (response_count[mywindow] < 4) {	   /* BYU 2.4.12 */
				response_data[mywindow][response_count[mywindow]++] = s1[i1];	/* BYU 2.4.12 */
				if (response_count[mywindow] == 4) {	   /* BYU 2.4.12 */
					if (response_data[mywindow][0] == '5' &&	/* BYU 2.4.12 */
						response_data[mywindow][1] >= '0' &&	/* BYU 2.4.12 */
						response_data[mywindow][1] <= '9' &&	/* BYU 2.4.12 */
						response_data[mywindow][2] == '0' &&	/* BYU 2.4.12 */
						response_data[mywindow][2] <= '9' &&	/* BYU 2.4.12 */
						response_data[mywindow][3] == ' ') /* BYU 2.4.12 */
						result = -1;					   /* BYU 2.4.12 - Abort indicated by other
														      host */
				}
			}

			/* If a "newline" is found then force a "carriage return" immediately. */
			if ('\012' == (s2[i2++] = s1[i1++]))
				s2[i2++] = '\015';
		}

		telnet (cnum, i2, &s2[0]);						   /* display on screen, etc. */

		/* demux all packets */
	} while (cnt > 0);

	return (result);									   /* BYU 2.4.12 - 0 normally, -1 if
														      connection closed */
}

/************************************************************************/
/* 	getword: remove a word from a string.  Things within quotes are assumed
	to be one word. return TRUE on success, FALSE on end of string
/*********************************************************************** */

short getword (char *string, char *word)
{
	char   *p, *q;
	int     i = 0;

	p = stpblkp (string);								   /* skip leading blanks */
	if (!(*p)) {										   /* no words in string */
		word[0] = '\0';
		return (FALSE);
	}
	if (*p == '!') {									   /* ! is a word */
		word[0] = *p;
		word[1] = '\0';
		strcpy (string, ++p);
		return (TRUE);
	}
	if (*p == '\"') {									   /* word delimited by quotes */
		while (p[++i] && p[i] != '\"')
			word[i - 1] = p[i];
		word[i - 1] = '\0';
		if (!p[i])
			ftpclient_debug_print ("Missing \". Assumed at end of string.");
		else
			i++;
		q = p + i;
	}
	else
		q = stptok (p, word, 50, " \t\015\012");		   /* get word, max len 50 */
	p = stpblkp (q);									   /* remove trailing blanks */
	strcpy (string, p);									   /* remove extracted stuff */
	return (TRUE);
}

/************************************************************************/
/* lowercase: convert a string to lowercase
/*********************************************************************** */

short lowercase(char *word)
{
	short     i;

	for (i = 0; word[i] = tolower (word[i]); i++) ;
	return (TRUE);
}

/************************************************************************/
/*	finduniq: find name that is a unique prefix of one of the entries in a list.
	Return position of the entry, NONE if none, AMBIGUOUS if more than one.
/*********************************************************************** */

int     finduniq (unsigned char *name, int listsize)
{
	int     i, j = NONE, len;
	unsigned char *p, cmdstring[256];

	len = strlen ((char *) name);
	for (i = 0; i < listsize; i++) {
		GetIndString (cmdstring, FTPCOMMANDS, i + 1);
		p = (unsigned char *) p2cstr (cmdstring);
		if (!strncmp ((char *) name, (char *) cmdstring, len)) {	/* prefix */
			if (len == strlen ((char *) cmdstring))
				return (i + 1);							   /* exact match */
			if (j != NONE)
				j = AMBIGUOUS;							   /* more than one match */
			else
				j = i + 1;								   /* note prefix found */
		}
	}
	return (j);											   /* prefix */
}

/************************************************************************/
/* 	checkoredir: check for output redirection.  If the command contains a >,
	assume a filename follows and extract it.  Remove the redirection from
	the original command. Also change \ to / return TRUE if redirection
	specified, FALSE otherwise
/*********************************************************************** */
Boolean checkoredir(char *command, char *filename, short slashflip)
{
	short	i;

	filename[0] = '\0';
	for (i = 0; (command[i] != '>'); i++) {				   /* process command part */
		if (slashflip && command[i] == '\\')
			command[i] = '/';
		if (!command[i])
			return (FALSE);								   /* no redirection */
	}
	getword (&command[i + 1], filename);				   /* get redirected filename */
	command[i] = '\0';
	return (TRUE);
}

/************************************************************************/
/* getnname: get next name from captured list names delimited by newlines - <CR> or <LF>
/*********************************************************************** */
Boolean getnname(char *string, char *word)
{
	char   *s;

	s = string;
	while ((*string == '\012') || (*string == '\015'))
		string++;										   /* skip initial newlines */
	if (!(*string))
		return (FALSE);									   /* end of captlist */
	while ((*string != '\012') && (*string != '\015') && (*string))
		*(word++) = *(string++);
	while ((*string == '\012') || (*string == '\015'))
		string++;										   /* skip trailing newline */
	*word = '\0';
	strcpy (s, string);
	return (TRUE);
}

/********************************************************************/
/* 	FTP PI Protocol interpreter for user interface commands Will permit any command to
	be abbreviated uniquely. Recognizes commands, translates them to the protocol
	commands to be sent to the other server, and uses userftpd, the daemon, to do data
	transfers.
/*********************************************************************** */
short ftppi (char *command)
{
	Boolean needanswer;
	int     cmdno, i, cmd_status;
	char    cmdname[20], word[PATHLEN], line[MAXFTP], answer[20], ofilename[PATHLEN];
	unsigned char destname[50];							   /* who to connect to */
	char    printline[256];								   /* line to display */
	Str255	scratchPstring;
	
	if (captlist == 0) {
		captlist = (unsigned char *) NewPtrClear ((long) captlistsize);
		/* newfile = (unsigned char *) NewPtrClear( (long) PATHLEN);		/* Pointer, no allocation
		   necessary */
		transfile = (unsigned char *) NewPtrClear ((long) PATHLEN);
		xs = (unsigned char *) NewPtrClear ((long) (BUFFERS + 10));
	}

	// Should check for not enough memory here!
	
	/* Indicates waiting for response *//* BYU 2.4.12 */
	response_count[scrn] = 0;							   /* BYU 2.4.12 */

	/* get command number */
	if (!getword (command, cmdname))
		return (FALSE);									   /* get command name */

	/* removes first word from command */
	lowercase (cmdname);
	cmdno = finduniq ((unsigned char *) cmdname, NCMDS);   /* search cmdlist for prefix */
	if (cmdno == AMBIGUOUS) {							   /* not unique abbreviation */
		putstring (scrn, "?Ambiguous command\015\012");
		return (FALSE);
	}
	if (cmdno == NONE) {								   /* not a prefix of any command */
		putstring (scrn, "?Invalid command\015\012");
		return (FALSE);
	}

	/* change \ to / and check if command output redirected */
	if (cmdno != BANG) {								   /* don't alter shell escape */
		if (cmdno != LLS)								   /* do not flip slashes for LLS */
			checkoredir(command, ofilename, slashflip);   /* check redirection, flip \ */
		else
			checkoredir(command, ofilename, FALSE);	   /* check redirection */
	}

	/* process commands */
	switch (cmdno) {
	case QMARK:
	case HELP:
		if (!command[0]) {								   /* no argument */
			putstring (scrn, "Commands may be abbreviated:\015\012");

			/* display command list */
			printline[0] = '\0';
			for (i = 2; i < NCMDS; i++) {
				GetIndString (scratchPstring, FTPCOMMANDS, i + 1);
				PtoCstr(scratchPstring);
				sprintf (word, "%-16s", (char *)scratchPstring);			   /* get word from list */
				strcat (printline, word);				   /* add to line */
				if ((i - 2) % 5 == 4) {					   /* display line */
					printline[79] = '\0';
					putstring (scrn, printline);
					putstring (scrn, "\015\012");
					printline[0] = '\0';
				}
			}
			if (i % 5 != 4) {
				putstring (scrn, printline);			   /* last line */
				putstring (scrn, "\015\012");
			}
			return (TRUE);
		}

		/* help for specific commands */
		else {
			while (getword (command, word)) {			   /* loop for all args */
				i = finduniq ((unsigned char *) word, NCMDS);	/* which command? */
				if (i == AMBIGUOUS)						   /* non-unique command name */
					sprintf (printline, "?Ambiguous help command %s", word);
				else if (i == NONE)						   /* no such command */
					sprintf (printline, "?Invalid help command %s", word);
				else {									   /* display help string */
					GetIndString (scratchPstring, HELPRESID, i);
					PtoCstr(scratchPstring);
				}
				putstring (scrn, (char *)scratchPstring);
				putstring (scrn, "\015\012");
			}
			return (TRUE);
		}
		break;
	case BANG:											   /* shell escape */
		return (TRUE);
	case BELL:
		if (getword (command, word)) {					   /* scan arg */
			lowercase (word);
			if (!strcmp (word, "off"))
				bell = FALSE;
			else if (!strcmp (word, "on"))
				bell = TRUE;
			else
				bell = !bell;
		}
		else
			bell = !bell;
		if (bell)
			ftpclient_debug_print ("Bell mode on.");
		else
			ftpclient_debug_print ("Bell mode off.");
		return (TRUE);
	case BYE:
	case QUIT:
		cmd_status = ftpdo (scrn, "QUIT", ofilename);
		netclose (screens[scrn].port);
		destroyport (scrn);
		return (TRUE);
	case DEBUG:										   /* turn on/off debugging, optional level */
		if (TelInfo->debug) {
			TelInfo->debug = 0;
			ftpclient_debug_print ("Debug off");
		}
		else {
			TelInfo->debug = 1;
			ftpclient_debug_print ("Debug on");
		}
		return (TRUE);
	case GLOB:											   /* wildcard expansion */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				glob = FALSE;
			else if (!strcmp (word, "on"))
				glob = TRUE;
			else
				glob = !glob;
		}
		else
			glob = !glob;
		if (glob)
			ftpclient_debug_print ("Globbing on.");
		else
			ftpclient_debug_print ("Globbing off.");
		return (TRUE);
	case HASH:											   /* hash mark printing */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				hash = FALSE;
			else if (!strcmp (word, "on"))
				hash = TRUE;
			else
				hash = !hash;
		}
		else
			hash = !hash;
		if (hash)
			ftpclient_debug_print ("Hash mark printing on (1024 bytes/hash mark).");
		else
			ftpclient_debug_print ("Hash printing off.");
		return (TRUE);
	case INTERACTIVE:									   /* prompting on multiple transfers */
		prompt = TRUE;
		putstring (scrn, "Interactive mode on.\015\012");
		return (TRUE);
	case LCD:											   /* change local directory */
		if (*(stpblkp (command))) {						   /* BYU 2.4.17 - CD */
			getword (command, word);					   /* BYU 2.4.17 */
			if (ChangeDirectory(&(TelInfo->FTPClientDirID), &(TelInfo->FTPClientVRefNum), word))							   /* BYU 2.4.17 */
				putstring (scrn, "Unable to change directory\015\012");
		}
		PathNameFromDirID(TelInfo->FTPClientDirID, TelInfo->FTPClientVRefNum, scratchPstring);
		PtoCstr(scratchPstring);
		sprintf (printline, "Local directory now %s\015\012", (char *)scratchPstring);
		putstring (scrn, printline);
		return (TRUE);
	case LDIR:											   /* local DIR */
	case LLS:
		putstring (scrn, "Directory ");
		if (!(*(stpblkp (command)))) {					   /* null response ok */
			PathNameFromDirID(TelInfo->FTPClientDirID, TelInfo->FTPClientVRefNum, scratchPstring);
			PtoCstr(scratchPstring);
			putstring (scrn, (char *)scratchPstring);
			sprintf (line, "*");
		}
		else {
			putstring(scrn, command);
			sprintf (line, "%s", command);
		}

		putstring (scrn, "\015\012");
		strcpy(Template, &line[0]);
		if (NULL == (newfile = (unsigned char *) firstname (Template, TelInfo->FTPClientDirID, TelInfo->FTPClientVRefNum, &ClientDirectoryState))) {
			putstring (scrn, "No files found\015\012");
		}
		else {
			putstring (scrn, (char *) newfile);
			putstring (scrn, "\015\012");
			while (NULL != (newfile = (unsigned char *) nextname (Template, &ClientDirectoryState))) {
				putstring (scrn, (char *) newfile);
				putstring (scrn, "\015\012");
			}
		}
		return (TRUE);
	case NONINTERACTIVE:								   /* turn off interactive prompting */
		prompt = FALSE;
		ftpclient_debug_print ("Interactive mode off.");
		return (TRUE);
	case OPEN:											   /* open connection to host */
		PresentOpenConnectionDialog();						/* Open a connection */
		return (TRUE);
	case PROMPT:										   /* interactive prompting */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				prompt = FALSE;
			else if (!strcmp (word, "on"))
				prompt = TRUE;
			else
				prompt = !prompt;
		}
		else
			prompt = !prompt;
		if (prompt)
			ftpclient_debug_print ("Interactive mode on.");
		else
			ftpclient_debug_print ("Interactive mode off.");
		return (TRUE);
	case SENDPORT:										   /* send PORT commands for each transfer */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				sendport = FALSE;
			else if (!strcmp (word, "on"))
				sendport = TRUE;
			else
				sendport = !sendport;
		}
		else
			sendport = !sendport;
		if (sendport)
			ftpclient_debug_print ("Use of PORT cmds on.");
		else
			ftpclient_debug_print ("Use of PORT cmds off.");
		return (TRUE);
	case SLASHFLIP:									   /* change \ to / */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				slashflip = FALSE;
			else if (!strcmp (word, "on"))
				slashflip = TRUE;
			else
				slashflip = !slashflip;
		}
		else
			slashflip = !slashflip;
		if (slashflip)
			ftpclient_debug_print ("Slash translation on.");
		else
			ftpclient_debug_print ("Slash translation off.");
		return (TRUE);
	case STATUS:										   /* display status info */
		sprintf (printline, "Connected to %s", destname);
		ftpclient_debug_print (printline);
		if (ftpfilemode == FASCII)
			ftpclient_debug_print ("Transfer mode is ascii.");
		else
			ftpclient_debug_print ("Transfer mode is binary.");
		if (bell)
			ftpclient_debug_print ("Bell on.");
		else
			ftpclient_debug_print ("Bell off.");
		if (TelInfo->debug) {
			ftpclient_debug_print ("Debug on");
		}
		else
			ftpclient_debug_print ("Debug off");
		if (glob)
			ftpclient_debug_print ("Filename globbing on.");
		else
			ftpclient_debug_print ("Filename globbing off.");
		if (hash)
			ftpclient_debug_print ("Hash-mark printing on.");
		else
			ftpclient_debug_print ("Hash-mark printing off.");
		if (prompt)
			ftpclient_debug_print ("Interactive prompting on.");
		else
			ftpclient_debug_print ("Interactive prompting off.");
		if (sendport)
			ftpclient_debug_print ("Sending of port commands on.");
		else
			ftpclient_debug_print ("Sending of PORT cmds off.");
		if (slashflip)
			ftpclient_debug_print ("Flipping \\ to / on.");
		else
			ftpclient_debug_print ("Flipping \\ to / off.");
		if (verbose)
			ftpclient_debug_print ("Verbose mode on.");
		else
			ftpclient_debug_print ("Verbose mode off.");

		ftpclient_debug_print ("\012Remote status:");
		cmd_status = ftpdo (scrn, "STAT", ofilename);

		return (TRUE);
	case VERBOSE:										   /* display informative messages */
		if (getword (command, word)) {
			lowercase (word);
			if (!strcmp (word, "off"))
				verbose = FALSE;
			else if (!strcmp (word, "on"))
				verbose = TRUE;
			else
				verbose = !verbose;
		}
		else
			verbose = !verbose;
		if (verbose)
			ftpclient_debug_print ("Verbose mode on.");
		else
			ftpclient_debug_print ("Verbose mode off.");
		return (TRUE);
	default:											   /* The other commands valid only if
														      connected (we always are here) */
		switch (cmdno) {
		case ACCT:										   /* get file in binary mode */
			while (!(*(stpblkp (command)))) {
				putstring (scrn, "Acct: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "ACCT %s", command);
			cmd_status = ftpdo (scrn, line, ofilename);	   /* get file */
			return (TRUE);
		case ASCII:									   /* transfer mode */
			cmd_status = ftpdo (scrn, "TYPE A", ofilename);
			return (TRUE);
		case MACB:										   /* transfer mode */
			if (TelInfo->MacBinary) {
				TelInfo->MacBinary = 0;
				cmd_status = ftpdo (scrn, "MACB D", ofilename);
			}
			else {
				TelInfo->MacBinary = 1;
				cmd_status = ftpdo (scrn, "MACB E", ofilename);
			}
			return (TRUE);
		case BGET:										   /* get file in binary mode */
			i = ftpfilemode;							   /* save current mode */
			if (i == FASCII)
				cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			while (!(*(stpblkp (command)))) {
				putstring (scrn, "File: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "RETR %s", command);
			cmd_status = ftpdo (scrn, line, ofilename);	   /* get file */
			if (i == FASCII)
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);	/* restore mode */
			return (TRUE);
		case BINARY:									   /* binary mode */
			cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			return (TRUE);
		case BPUT:										   /* put file in binary mode */
			i = ftpfilemode;
			if (i == FASCII)
				cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			while (!(*(stpblkp (command)))) {			   /* if no arg */
				putstring (scrn, "File: ");				   /* get from user */
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "STOR %s", command);
			cmd_status = ftpdo (scrn, line, ofilename);
			if (i == FASCII)
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
			return (TRUE);
		case CD:										   /* change remote directory */
			while (!(*(stpblkp (command)))) {			   /* if no arg, get from user */
				putstring (scrn, "To: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			getword (command, word);
			if (!strcmp (word, "..")) {					   /* special case */
				cmd_status = ftpdo (scrn, "CDUP", ofilename);
				if (cmd_status != ERROR)
					return (TRUE);						   /* if CDUP understood */
#if 0
				ftpclient_debug_print ("Trying again...");
				cmd_status = ftpdo (scrn, "XCUP", ofilename);	/* try alternative */
				if (cmd_status != ERROR)
					return (TRUE);
				ftpclient_debug_print ("Trying again...");				   /* else try usual CD */
#endif
			}
			sprintf (line, "CWD %s", word);				   /* try CWD */
			cmd_status = ftpdo (scrn, line, ofilename);
			if (cmd_status != ERROR)
				return (TRUE);
#if 0
			ftpclient_debug_print ("Trying again...");
			sprintf (line, "XCWD %s", word);			   /* try XCWD */
			cmd_status = ftpdo (scrn, line, ofilename);
#endif
			return (TRUE);
		case CLOSE:									   /* drop connection */
			cmd_status = ftpdo (scrn, "QUIT", ofilename);
			netclose (screens[scrn].port);
			destroyport (scrn);
			return (TRUE);
		case DEL:
		case RM:
			getword (command, word);
			while (!word[0]) {							   /* get arg from user */
				putstring (scrn, "File: ");
				if (ftpgets (word, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			if (prompt) {								   /* check interactively */
				sprintf (printline, "Delete %s? ", word);
				putstring (scrn, printline);
				ftpgets (answer, 20, 1);
				if (tolower (*(stpblkp (answer))) != 'y')
					return (TRUE);
			}
			sprintf (line, "DELE %s", word);
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case DIR:										   /* get list of remote files */
			if (ftpfilemode == FIMAGE) {
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
				if (getword (command, word)) {
					sprintf (line, "LIST %s", word);
				}
				else
					sprintf (line, "LIST");
				strcpy (screens[scrn].ftpbuf, line);
				screens[scrn].clientflags = TYPE_I_ON_CLOSE;
				screens[scrn].ftpstate = 3;				   /* Do "LIST" when "TYPE" is finished */
				screens[scrn].ftpnext = 1;
			}
			else {
				if (getword (command, word)) {			   /* Do DIR */
					sprintf (line, "LIST %s", word);
					cmd_status = ftpdo (scrn, line, ofilename);
				}
				else
					cmd_status = ftpdo (scrn, "LIST", ofilename);
			}
			return (TRUE);
		case GET:
		case RECV:										   /* get remote file */
			while (!(*(stpblkp (command)))) {			   /* if no arg */
				putstring (scrn, "File: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			sprintf (line, "RETR %s", command);
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case LS:										   /* get remote file list - short */
			if (ftpfilemode == FIMAGE) {
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
				if (getword (command, word)) {
					sprintf (line, "NLST %s", word);
				}
				else
					sprintf (line, "NLST");
				strcpy (screens[scrn].ftpbuf, line);
				screens[scrn].clientflags = TYPE_I_ON_CLOSE;
				screens[scrn].ftpstate = 3;				   /* Do "NLST" when "TYPE" is finished */
				screens[scrn].ftpnext = 1;
			}
			else {
				if (getword (command, word)) {
					sprintf (line, "NLST %s", word);
					cmd_status = ftpdo (scrn, line, ofilename);
				}
				else
					cmd_status = ftpdo (scrn, "NLST", ofilename);
			}
			return (TRUE);
		case MDELETE:
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Files: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			while (getword (command, word)) {			   /* for each arg */
				if (glob) {								   /* wildcard expansion */
					sprintf (line, "NLST %s", word);
					screens[scrn].clientflags |= CAPTURE_DATA;
					cmd_status = ftpdo (scrn, line, ofilename);	/* put exapnsion in captlist */
				}
				else
					strcpy ((char *) captlist, word);	   /* captlist has name(s) now */
				while (getnname ((char *) captlist, word)) {	/* for each name */
					if (prompt) {						   /* check */
						sprintf (printline, "mdelete %s? ", word);
						putstring (scrn, printline);
						if (ftpgets (answer, 20, 1) == ABORT) {	/* abort */
							command[0] = '\0';			   /* no more processing */
							break;						   /* quit immediately */
						}
						if (tolower (*(stpblkp (answer))) != 'y')
							continue;
					}
					sprintf (line, "DELE %s", word);	   /* delete */
					cmd_status = ftpdo (scrn, line, ofilename);
				}
			}
			return (TRUE);
		case MDIR:										   /* remote multiple DIR */
			i = ftpfilemode;							   /* save mode */
			if (i == FIMAGE)
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Directories: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			while (getword (command, word)) {			   /* for each arg */
				if (glob) {								   /* expand wildcards */
					sprintf (line, "NLST %s", word);
					screens[scrn].clientflags |= CAPTURE_DATA;
					cmd_status = ftpdo (scrn, line, ofilename);
				}
				else
					strcpy ((char *) captlist, word);
				while (getnname ((char *) captlist, word)) {	/* for each name */
					if (prompt) {						   /* check */
						sprintf (printline, "mdir %s? ", word);
						putstring (scrn, printline);
						if (ftpgets (answer, 20, 1) == ABORT) {	/* abort */
							command[0] = '\0';			   /* no more processing */
							break;						   /* quit immediately */
						}
						if (tolower (*(stpblkp (answer))) != 'y')
							continue;
					}
					sprintf (line, "LIST %s", word);	   /* DIR */
					cmd_status = ftpdo (scrn, line, ofilename);
				}
			}
			if (i == FIMAGE)
				cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			return (TRUE);
		case MGET:										   /* get multiple files */
			getword (command, line);
			while (!line[0]) {							   /* no arg */
				putstring (scrn, "Files: ");
				if (ftpgets (line, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			if (getword (line, word)) {
				sprintf (command, "NLST %s", word);
				screens[scrn].clientflags |= CAPTURE_DATA;
				cmd_status = ftpdo (scrn, command, ofilename);

				screens[scrn].clientflags |= MGET_STATE;
			}
			return (TRUE);
		case MKDIR:									   /* create directory */
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Directory: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			sprintf (line, "XMKD %s", command);			   /* try XMKD */
			cmd_status = ftpdo (scrn, line, ofilename);
			if (cmd_status != ERROR)
				return (TRUE);
			ftpclient_debug_print ("Trying again...");
			sprintf (line, "MKD %s", command);			   /* else try MKD */
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case MLS:
			i = ftpfilemode;
			if (i == FIMAGE)
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Directories: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			while (getword (command, word)) {			   /* for each arg */
				if (glob) {								   /* exapnd wildcards */
					sprintf (line, "NLST %s", word);
					screens[scrn].clientflags |= CAPTURE_DATA;
					cmd_status = ftpdo (scrn, line, ofilename);
				}
				else
					strcpy ((char *) captlist, word);
				while (getnname ((char *) captlist, word)) {	/* for each name */
					if (prompt) {						   /* check */
						sprintf (printline, "mls %s? ", word);
						putstring (scrn, printline);
						if (ftpgets (answer, 20, 1) == ABORT) {	/* abort */
							command[0] = '\0';			   /* no more processing */
							break;						   /* quit immediately */
						}
						if (tolower (*(stpblkp (answer))) != 'y')
							continue;
					}
					sprintf (line, "NLST %s", word);	   /* DIR */
					cmd_status = ftpdo (scrn, line, ofilename);
				}
			}
			if (i == FIMAGE)
				cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			return (TRUE);
		case MODE:										   /* set stream mode */
			getword (command, word);
			lowercase (word);
			if (strncmp (word, "stream", strlen (word)))
				ftpclient_debug_print ("We only support stream mode, sorry.");
			else
				ftpclient_debug_print ("Mode is stream.");
			return (TRUE);
		case MPUT:										   /* put multiple files */
			getword (command, (char *) captlist);
			while (!captlist[0]) {						   /* no arg */
				putstring (scrn, "Files: ");
				if (ftpgets ((char *) captlist, 100, 1) == ABORT)
					return (FALSE);						   /* abort */
			}
			if (getword ((char *) captlist, (char *) transfile)) {	/* for each arg */
				newfile = NULL;
				needanswer = TRUE;
				while (needanswer) {
					needanswer = FALSE;
					if (glob) {							   /* local wildcard expansion */
						if (newfile == NULL) {			   /* if no expansions yet */
							strcpy(Template, (char *)transfile);
							newfile = (unsigned char *) firstname (Template, TelInfo->FTPClientDirID, TelInfo->FTPClientVRefNum, &ClientDirectoryState);	/* get first name */
							if (newfile == NULL) {		   /* if no expansions */
								sprintf (printline, "No match for %s", transfile);
								ftpclient_debug_print (printline);
								if (getword ((char *) captlist, (char *) transfile)) {
									needanswer = TRUE;
								}
							}
						}
						else {							   /* not first name */
							newfile = (unsigned char *) nextname (Template, &ClientDirectoryState);	/* get next name */
							if (newfile == NULL) {		   /* if no names, next arg */
								if (getword ((char *) captlist, (char *) transfile)) {
									needanswer = TRUE;
								}
							}
						}
					}
					else
						newfile = transfile;			   /* no expansion */
					if (newfile != NULL) {
#if 1													   /* When prompting is fixed switch to the
														      code below */
						sprintf (command, "STOR \"%s\"", newfile);	/* name may have spl chars */
						cmd_status = ftpdo (scrn, command, ofilename);
						screens[scrn].clientflags |= MPUT_STATE;
#else
						if (prompt) {					   /* check */
							sprintf (printline, "mput %s? ", newfile);
							putstring (scrn, printline);
							if (ftpgets (answer, 20, 1) == ABORT) {	/* abort */
								command[0] = '\0';		   /* no more processing */
								break;
							}
							else if (tolower (*(stpblkp (answer))) == 'y') {
								sprintf (command, "STOR \"%s\"", newfile);	/* name may have spl
																			   chars */
								cmd_status = ftpdo (scrn, command, ofilename);
								screens[scrn].clientflags |= MPUT_STATE;
							}
						}
						else {
							sprintf (command, "STOR \"%s\"", newfile);	/* name may have spl chars */
							cmd_status = ftpdo (scrn, command, ofilename);
							screens[scrn].clientflags |= MPUT_STATE;
						}
#endif
					}
				}
			}
			return (TRUE);
		case PUT:
		case SEND:										   /* put file */
			while (!(*(stpblkp (command)))) {			   /* no args */
				putstring (scrn, "File: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "STOR %s", command);			   /* put file */
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case PWD:
#if 0
			cmd_status = ftpdo (scrn, "XPWD", ofilename);  /* try XPWD */
			if (i != ERROR)
				return (TRUE);
			ftpclient_debug_print ("Trying again...");
#endif
			cmd_status = ftpdo (scrn, "PWD", ofilename);   /* else try PWD */
			return (TRUE);
		case QUOTE:
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Command: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			cmd_status = ftpdo (scrn, command, ofilename); /* send command */
			return (TRUE);
		case REMOTEHELP:								   /* get help */
			if (*(stpblkp (command))) {					   /* for specific command */
				sprintf (line, "HELP %s", command);
				cmd_status = ftpdo (scrn, line, ofilename);
			}
			else
				cmd_status = ftpdo (scrn, "HELP", ofilename);	/* generic help */
			return (TRUE);
		case RENAME:									   /* rename remote file */
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "From: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			getword (command, word);
			sprintf (line, "RNFR %s", word);
			cmd_status = ftpdo (scrn, line, ofilename);	   /* send rename from name */
			while (!(*(stpblkp (command)))) {			   /* no second arg */
				putstring (scrn, "To: ");
				if (ftpgets (command, 100, 1) == ABORT) {
					cmd_status = ftpdo (scrn, "ABOR", ofilename);
					return (FALSE);
				}
			}
			sprintf (line, "RNTO %s", command);			   /* send rename to name */
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case RMDIR:									   /* remove remote dir */
			while (!(*(stpblkp (command)))) {			   /* no arg */
				putstring (scrn, "Directory: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "XRMD %s", command);			   /* try XRMD */
			cmd_status = ftpdo (scrn, line, ofilename);
			if (cmd_status != ERROR)
				return (TRUE);
			ftpclient_debug_print ("Trying again...");
			sprintf (line, "RMD %s", command);			   /* try RMD */
			cmd_status = ftpdo (scrn, line, ofilename);
			return (TRUE);
		case STRUCT:									   /* set structure type - only file */
			getword (command, word);
			lowercase (word);
			if (strncmp (word, "file", strlen (word)))
				ftpclient_debug_print ("We only support file structure, sorry.");
			else
				ftpclient_debug_print ("Structure is file.");
			return (TRUE);
		case TYPE:										   /* set transfer type */
			if (!getword (command, word)) {				   /* no arg, just show */
				if (ftpfilemode == FASCII)
					ftpclient_debug_print ("Transfer type is ascii.");
				else
					ftpclient_debug_print ("Transfer type is binary.");
			}
			lowercase (word);
			if (!strncmp (word, "ascii", strlen (word)))
				cmd_status = ftpdo (scrn, "TYPE A", ofilename);
			else if (!strncmp (word, "binary", strlen (word)) ||
					 !strncmp (word, "image", strlen (word)))
				cmd_status = ftpdo (scrn, "TYPE I", ofilename);
			else {
				sprintf (printline, "Unrecognized type: %s", word);
				ftpclient_debug_print (printline);
			}
			return (TRUE);
		case USER:										   /* login to remote machine */
			if (!(*(stpblkp (command)))) {				   /* null response to prompt ok */
				putstring (scrn, "Username: ");
				if (ftpgets (command, 100, 1) == ABORT)
					return (FALSE);
			}
			sprintf (line, "USER %s", command);			   /* username */
			if ((cmd_status = ftpdo (scrn, line, ofilename)) == TRUE)
				return (TRUE);
#if 0
			putstring (scrn, "Password: ");
#endif
			if (ftpgets (word, PATHLEN, 0) == ABORT)
				return (FALSE);							   /* no echoing */
			sprintf (line, "PASS %s", word);			   /* password */
			if ((cmd_status = ftpdo (scrn, line, ofilename)) == INCOMPLETE) {	/* if account needed */
				do {
					putstring (scrn, "Account: ");
					if (ftpgets (command, 100, 1) == ABORT)
						return (FALSE);
				} while (!(*(stpblkp (command))));
				sprintf (line, "ACCT %s", command);
				cmd_status = ftpdo (scrn, line, ofilename);
			}
			return (TRUE);
		default:										   /* unknown command */
			putstring (scrn, "?Invalid command\015\012");
#if 0
			sprintf (printline, "***Unknown command no: %d", cmdno);
			ftpclient_debug_print (printline);
#endif
			break;
		}
	}

}

/************************************************************************/
/* 	ftpport return a new port number so that we don't try to re-use ports before the
	mandatory TCP timeout period.  (lifetime of a packet) use a time-based initial port
	selection scheme.
/*********************************************************************** */

short ftpport(int ftpscrn)
{
	unsigned int i;
	unsigned char hostnum[5];
	char    sendline[60];							/* for port command */

	if (!sendport)									/* default port */
		return (HFTP - 1);
		
	if (curftpprt < 40000) {						/* restart cycle */
		i = (unsigned int) TickCount ();
		curftpprt = 40000 + (i & 0x3fff);
	}

	i = curftpprt--;								/* get port, update for next time */

	netgetip (hostnum);								/* get my ip number */

	ftpdata = netlisten (i);
	screens[ftpscrn].ftpport = ftpdata;
	SetPortType(ftpdata, UDATA);

	sprintf (sendline, "PORT %d,%d,%d,%d,%d,%d\015\012", hostnum[0], hostnum[1], hostnum[2],
			 hostnum[3], i / 256, i & 255);				   /* full port number */

	netpush (screens[ftpscrn].port);				/* empty command connection */
	netwrite (screens[ftpscrn].port, sendline, strlen (sendline));	/* send PORT command */

	return (i);										/* port number */
}

/************************************************************************/
/* 	ftpdo Do whatever command is sent from the user interface using userftpd,
	the background file handler
/*********************************************************************** */
int     ftpdo (int ftpscrn, char *s, char *ofile)
{
	int     i;
	char    name[MAXFTP], name2[MAXFTP];
	char    commandbuf[MAXFTP];
	HFileParam iop;

	strcpy (commandbuf, s);
	for (i = 0; i < 4; i++) {
		commandbuf[i] = toupper (commandbuf[i]);		   /* command to upper case */
	}

	if (!strncmp (commandbuf, "STOR", 4)) {				   /* put file */
		getword (&commandbuf[5], name);					   /* first arg - local file */
		if (!commandbuf[5])
			strcpy (&commandbuf[5], name);				   /* if only one argument */
		else {
			getword (&commandbuf[5], name2);			   /* second arg - removes quotes etc. */
			strcpy (&commandbuf[5], name2);				   /* copy back into command */
		}

		ftp_mbfp = MBopen (&name[0], TelInfo->FTPClientVRefNum, TelInfo->FTPClientDirID, 
				MB_READ + 	/* BYU 2.4.18 */
				( ((!TelInfo->MacBinary) || (ftpfilemode == FAMODE)) ? MB_DISABLE : 0) +
				((ftpfilemode == FAMODE) ? MB_ISASCII : 0)
				);
		if (ftp_mbfp == 0L) {
			putstring (ftpscrn, "Could not open file\015\012");
			return (-1);
		}
		ftpfh = 12;

		c2pstr (&name[0]);
		GetFileInfo (TelInfo->FTPClientVRefNum, TelInfo->FTPClientDirID, (StringPtr)&name[0], &iop);
		filelen = iop.ioFlLgLen;
		if (TelInfo->MacBinary && (ftpfilemode == FIMODE))
			filelen += iop.ioFlRLgLen;

		ftpport (ftpscrn);								   /* open data connection */
		screens[ftpscrn].ftpstate = 2;
		screens[ftpscrn].ftpnext = 20;
		strcpy (screens[ftpscrn].ftpbuf, commandbuf);
		strcat (screens[ftpscrn].ftpbuf, "\015\012");
		return (1);

	}
	else if (!strncmp (commandbuf, "RETR", 4)) {		   /* get file */
		getword (&commandbuf[5], name);					   /* remote file */
		if (commandbuf[5]) {							   /* two args present */
			getword (&commandbuf[5], name2);			   /* local file */

			if ((ftp_mbfp = MBopen (&name2[0], TelInfo->FTPClientVRefNum, TelInfo->FTPClientDirID, MB_WRITE +	/* BYU 2.4.18 */
							  (((!TelInfo->MacBinary) || (ftpfilemode == FAMODE)) ? MB_DISABLE : 0) +
							  ((ftpfilemode == FAMODE) ? MB_ISASCII : 0)
						)) == 0L) {
				putstring (ftpscrn, "Cannot open file to receive\015\012");
				return (-1);
			}
			else
				ftpfh = 12;
		}
		else {
			if ((ftp_mbfp = MBopen (&name[0], TelInfo->FTPClientVRefNum, TelInfo->FTPClientDirID, MB_WRITE +	/* BYU 2.4.18 */
							  (((!TelInfo->MacBinary) || (ftpfilemode == FAMODE)) ? MB_DISABLE : 0)
							  + ((ftpfilemode == FAMODE) ? MB_ISASCII : 0)
					)) == 0L) {
				putstring (ftpscrn, "Cannot open file to receive\015\012");
				return (-1);
			}
			else
				ftpfh = 12;
		}

		strcpy (&commandbuf[5], name);					   /* Put remote name back into command */

		ftpport (ftpscrn);								   /* open data connection */
		screens[ftpscrn].ftpstate = 2;
		screens[ftpscrn].ftpnext = 30;
		strcpy (screens[ftpscrn].ftpbuf, commandbuf);
		strcat (screens[ftpscrn].ftpbuf, "\015\012");
		return (1);
	}
	else if (!strncmp (commandbuf, "LIST", 4) || !strncmp (commandbuf, "NLST", 4)) {

		if ((screens[ftpscrn].clientflags & CAPTURE_DATA) != 0)
			screens[ftpscrn].capturesize = 0;			   /* Size of captured data */

		ftpport (ftpscrn);								   /* data connection */
		screens[ftpscrn].ftpstate = 2;
		screens[ftpscrn].ftpnext = 40;
		strcpy (screens[ftpscrn].ftpbuf, commandbuf);
		strcat (screens[ftpscrn].ftpbuf, "\015\012");
		return (1);

	}
	else if (!strncmp (commandbuf, "TYPE", 4)) {
		if (toupper (commandbuf[5]) == 'I')
			ftpfilemode = FIMAGE;						   /* Remember mode */
		else if (toupper (commandbuf[5]) == 'A')
			ftpfilemode = FASCII;

	}

	netpush (screens[ftpscrn].port);

	strcat (commandbuf, "\015\012");					   /* BYU 2.4.11 */
	netwrite (screens[ftpscrn].port, commandbuf, strlen (commandbuf));	/* BYU 2.4.11 - send
																		   command */
	c2pstr(ofile);
	
	if (((screens[ftpscrn].clientflags & CAPTURE_DATA) == 0) && ofile[0]) {	/* command redirected */
		if ((screens[ftpscrn].ftpstate != 20) && (screens[ftpscrn].ftpstate != 30)) {	/* not get or put */
			if (0 > (ftpfh = ftpopen ((StringPtr)ofile)))
				ftpclient_debug_print (" Cannot open output file.");
			else if (ftpdata > -1) {
				screens[ftpscrn].ftpstate = 30;			   /* act as get, since data goes into file */
			}
			else {
				/* Close(ftpfh);		/* BYU LSC */
				ftpfh = 0;
			}
		}
	}

	return (FALSE);
}

/************************************************************************/
/* userftpd FTP receive and send file functions
/*********************************************************************** */

short userftpd (int code, int myport)
{
	Boolean needanswer;
	long    mytime;
	int     cmd_status, connection_status, ftpstate, i, mycode, myftpdata, mytelport, mywindow;
	char    command[MAXFTP], printline[MAXFTP], word[MAXFTP];

	mycode = code;
	connection_status = 0;								   /* BYU 2.4.12 - assume good connection
														      status */
	ftpstate = 1;

	mywindow = WindByPort (myport);
	if (mywindow >= 0) {
		if (myport == screens[mywindow].port) {
			ftpstate = screens[mywindow].ftpstate;
			myftpdata = -1;
		}
		else {
			ftpstate = screens[mywindow].ftpstate;
			myftpdata = screens[mywindow].ftpport;
		}
	}

	if (mywindow < 0)
		return (0);

	mytelport = screens[mywindow].port;

	switch (ftpstate) {
	default:											   /* unknown */
		break;

	case 1:
		if ((mycode == CONDATA) && (myport == mytelport))
			connection_status = dumpcon (myport, mywindow);
		break;

		/* Wait for response from "PORT" command, then send ftp command */
	case 2:
		if (mycode == CONDATA) {
			connection_status = dumpcon (myport, mywindow);
			netpush (screens[mywindow].port);
			netwrite (screens[mywindow].port, screens[mywindow].ftpbuf, strlen (screens[mywindow].ftpbuf));	/* send command */
			screens[mywindow].ftpstate = screens[mywindow].ftpnext;
		}
		break;

		/* Wait for response from "TYPE" command, then send ftp command */
	case 3:
		if (mycode == CONDATA) {
			connection_status = dumpcon (myport, mywindow);
			cmd_status = ftpdo (mywindow, screens[mywindow].ftpbuf, "");
		}
		break;

	case 5:
		/* Get the next file of an MGET. */
		if ((screens[mywindow].clientflags & CAPTURE_DATA) != 0) {
			screens[mywindow].clientflags &= ~CAPTURE_DATA;
		}

		/* After capturing the data, initiate the file transfer(s) */
		if ((screens[mywindow].clientflags & MGET_STATE) != 0) {
			if (screens[mywindow].capturesize > 0) {
				screens[mywindow].ftpstate = 6;
				netputuev (CONCLASS, CONDATA, mytelport,0);  /* Stay alive to transfer file */
			}
			else {
				screens[mywindow].clientflags &= ~MGET_STATE;
				screens[mywindow].ftpstate = 1;
			}
		}
		break;

		/* Receive one MGET file. */
	case 6:
		connection_status = dumpcon (myport, mywindow);
		if (getnname ((char *) captlist, (char *) word)) { /* for each name */
#if 0
			if (prompt) {								   /* check */
				sprintf (printline, "mget %s? ", word);
				putstring (mywindow, printline);
				if (ftpgets (answer, 20, 1) == ABORT) {	   /* abort */
					command[0] = '\0';					   /* no more processing */
					break;								   /* quit immediately */
				}
				if (tolower (*(stpblkp (answer))) != 'y')
					continue;
			}
			else {
#endif
				sprintf (printline, "Receiving %s\015\013", word);
				putstring (mywindow, printline);
#if 0
			}
#endif
			sprintf (command, "RETR \"%s\"", word);
			if ((cmd_status = ftpdo (mywindow, command, "")) < 0) {
				screens[mywindow].clientflags &= ~MGET_STATE;
				screens[mywindow].ftpstate = 1;
			}
		}
		else {
			screens[mywindow].clientflags &= ~MGET_STATE;
			screens[mywindow].ftpstate = 1;
		}
		break;

		/* Send a file to the remote connection */
	case 20:

		if (mycode == CONFAIL)
			mycode = CONCLOSE;							   /* something went wrong */

		if (mycode == CONOPEN) {
			screens[mywindow].ftpstate = 21;
			transtime = TickCount ();
			translen = 0;
			towrite = 0;
			xp = 0;
			netputuev (CONCLASS, CONDATA, myport,0);
		}
		else if ((mycode == CONDATA) && (myport == mytelport)) {
			connection_status = dumpcon (myport, mywindow);
		}
		break;

	case 21:
		/* transfer file(s) to the other host via ftp request file is already open = ftpfh */
		if (mycode == CONDATA) {
			if (myport == mytelport) {
				connection_status = dumpcon (myport, mywindow);
			}
			else if (myport == myftpdata) {

				netputuev (CONCLASS, CONDATA, myport,0);	   /* Stay alive */

				if (towrite <= xp) {

					i = BUFFERS;
					towrite = MBread (ftp_mbfp, xs, i);
					xp = 0;
				}

				if ((towrite <= 0) || netest (myftpdata)) {/* we are done */
					if (ftp_mbfp->fd != 0)
						MBclose (ftp_mbfp);				   /* BYU - close input file */
					screens[mywindow].ftpstate = 22;
					break;
				}

				if (ftpfilemode == FAMODE)
					i = Sfwrite (myftpdata, &xs[xp], (int) towrite - xp);
				else
					i = netwrite (myftpdata, &xs[xp], (int) towrite - xp);

				if (i > 0) {
					xp += i;
					translen += i;
				}

				mytime = TickCount ();
				if ((transtime + 800) < mytime) {
					transtime = mytime;
					sprintf (printline, "%ld/%ld bytes sent.\015\013", translen, filelen);
					i = strlen (printline);
					parse (&screens[mywindow], (unsigned char *) &printline[0], i);
				}
			}
		}

		break;

	case 22:											   /* wait for data to be accepted */
		netputuev (CONCLASS, CONDATA, myport,0);			   /* Stay alive */

		fcnt = netpush (myftpdata);						   /* will go negative on err */
		if (!fcnt || netest (myftpdata)) {
			mycode = CONCLOSE;
			sprintf (printline, "%ld/%ld bytes sent.\015\013", translen, filelen);
			parse (&screens[mywindow], (unsigned char *) &printline[0], strlen (printline));
		}
		if ((mycode == CONDATA) && (myport == mytelport))
			connection_status = dumpcon (myport, mywindow);
		break;

		/* Get a file from the remote connection */
	case 30:
		if (mycode == CONFAIL)
			mycode = CONCLOSE;							   /* something went wrong */

		if (mycode == CONOPEN) {
			screens[mywindow].ftpstate = 31;
			filelen = xp = 0;
			transtime = TickCount ();
			translen = 0L;
		}
		else if ((mycode == CONDATA) && (myport == mytelport)) {
			connection_status = dumpcon (myport, mywindow);
		}
		break;

		/* file has already been opened, take everything from the connection and place into the
		   open file: ftpfh */
	case 31:
		if (mycode == CONDATA) {
			if (myport == mytelport) {
				connection_status = dumpcon (myport, mywindow);
			}
			else if (myport == myftpdata) {

				/* wait until xs is full before writing to disk */
				if (filelen <= 2000) {

					if (xp) {
						if (0 > MBwrite (ftp_mbfp, xs, xp))
							mycode = CONCLOSE;
						xp = 0;
					}
					filelen = BUFFERS;					   /* expected or desired len to go */
				}

				if (mycode == CONDATA) {
					if (ftpfilemode == FAMODE)
						fcnt = Sfread (myftpdata, (char *)&xs[xp], (long) filelen);
					else
						fcnt = netread (myftpdata, &xs[xp], (int) filelen);

					if (fcnt >= 0) {
						filelen -= fcnt;
						xp += fcnt;
						translen += fcnt;
					}

					mytime = TickCount ();
					if ((transtime + 800) < mytime) {
						transtime = mytime;
						if ((screens[mywindow].clientflags & MGET_STATE) == 0) {
							sprintf (printline, "%ld bytes received.\015\013", translen);
							i = strlen (printline);
							parse (&screens[mywindow], (unsigned char *) &printline[0], i);
						}
					}

					/* printf(" %d %d %d \012",filelen,xp,fcnt); n_row();  */
					if (fcnt < 0) {
						if (0 > MBwrite (ftp_mbfp, xs, xp)) {
							break;
						}
						if (ftp_mbfp->fd != 0) {
							MBclose (ftp_mbfp);
						}
						ftpfh = 0;
					}
					netputuev (CONCLASS, CONDATA, myport,0);
				}
			}
		}
		/* Ouput any remaining data before closing */
		if (mycode == CONCLOSE) {
			if (xp) {
				if (0 > MBwrite (ftp_mbfp, xs, xp))
					netclose (myftpdata);
			}
			if ((screens[mywindow].clientflags & MGET_STATE) == 0) {
				sprintf (printline, "%ld bytes received.\015\013", translen);
				i = strlen (printline);
				parse (&screens[mywindow], (unsigned char *) &printline[0], i);
			}
		}
		break;

		/* Waiting for "ftpdata" connection to open, display any communication from the telnet
		   port. */
	case 40:
		if (mycode == CONOPEN)
			screens[mywindow].ftpstate = 41;

		if ((myport == myftpdata) &&
			((screens[mywindow].clientflags & CAPTURE_DATA) != 0)) {
			captcon (myport, mywindow);
		}
		else {
			connection_status = dumpcon (myport, mywindow);
		}
		break;

		/* If the "ftpdata" port is open, then postpone reading and displaying any data from other
		   ports once data has been received.  Otherwise, read and display data from anywhere. */
	case 41:
		if (mycode == CONDATA) {
			if ((myport == myftpdata) &&
				((screens[mywindow].clientflags & CAPTURE_DATA) != 0)) {
				captcon (myport, mywindow);
			}
			else {
				connection_status = dumpcon (myport, mywindow);
			}
			if (myport == myftpdata)
				screens[mywindow].ftpstate = 42;
			netputuev (CONCLASS, CONDATA, myport,0);
		}
		else if (mycode == CONCLOSE) {
			screens[mywindow].ftpstate = 40;
		}
		break;

		/* Only data from "ftpdata" */
	case 42:
		if (mycode == CONDATA) {
			if (myport == myftpdata) {
				if ((screens[mywindow].clientflags & CAPTURE_DATA) != 0) {
					captcon (myport, mywindow);
				}
				else {
					connection_status = dumpcon (myport, mywindow);
				}
			}
			netputuev (CONCLASS, CONDATA, myport,0);		   /* Keep port monitoring alive */
		}
		else if (mycode == CONCLOSE) {
			screens[mywindow].ftpstate = 40;
		}
		break;

	}													   /* end of switch */

	/* After reading from connection, if the connection is closed, reset up shop. */
	if ((mycode == CONCLOSE) || (connection_status < 0)) { /* BYU 2.4.12 */
		if (connection_status < 0)						   /* BYU 2.4.12 */
			screens[mywindow].clientflags &= ~MPUT_STATE;
		if (ftpfh > 0) {
			/* Close(ftpfh);		/* BYU LSC */
			ftpfh = 0;
		}
		if (ftp_mbfp->fd != 0)
			MBclose (ftp_mbfp);
		screens[mywindow].ftpstate = 1;
		fcnt = 0;
		if (myftpdata == myport) {
			netclose (myftpdata);
			SetPortType(myftpdata, NO_TYPE);
			screens[mywindow].ftpport = -1;
		}
		if ((screens[mywindow].clientflags & TYPE_I_ON_CLOSE) != 0) {
			screens[mywindow].clientflags = 0;
			cmd_status = ftpdo (mywindow, "TYPE I", "");
		}

		/* Send the next file of an MPUT */
		if ((screens[mywindow].clientflags & MPUT_STATE) != 0) {
			needanswer = TRUE;
			while (needanswer) {
				needanswer = FALSE;
				if (glob) {								   /* local wildcard expansion */
					newfile = (unsigned char *) nextname (Template, &ClientDirectoryState);	/* get next name */
					if (newfile == NULL) {				   /* if no expansions yet */
						if (getword ((char *) captlist, (char *) transfile)) {
							strcpy(Template, (char *) transfile);
							newfile = (unsigned char *) firstname (Template, TelInfo->FTPClientDirID, TelInfo->FTPClientVRefNum, &ClientDirectoryState);	/* get first name */
							if (newfile == NULL) {		   /* if no expansions */
								sprintf (printline, "No match for %s", transfile);
								ftpclient_debug_print (printline);
							}
						}
					}
				}
				else if (getword ((char *) captlist, (char *) transfile)) {
					newfile = transfile;
				}
				else
					newfile = NULL;

				if (newfile == NULL) {
					screens[mywindow].clientflags &= ~MPUT_STATE;
					screens[mywindow].ftpstate = 1;
				}
				else {
#if 1													   /* When prompting is fixed switch to the
														      code below */
					sprintf (screens[mywindow].ftpbuf, "STOR \"%s\"", newfile);
					cmd_status = ftpdo (mywindow, screens[mywindow].ftpbuf, "");
#else
					if (prompt) {						   /* check */
						sprintf (printline, "mput %s? ", newfile);
						putstring (mywindow, printline);
						if (ftpgets (answer, 20, 1) == ABORT) {	/* abort */
							screens[mywindow].clientflags &= ~MPUT_STATE;
							screens[mywindow].ftpstate = 1;
						}
						else if (tolower (*(stpblkp (answer))) != 'y') {
							needanswer = TRUE;
						}
						else {
							sprintf (screens[mywindow].ftpbuf, "STOR \"%s\"", newfile);
							cmd_status = ftpdo (mywindow, screens[mywindow].ftpbuf, "");
						}
					}
					else {
						/* The Mac file may have special characters, so quote the whole thing */
						sprintf (screens[mywindow].ftpbuf, "STOR \"%s\"", newfile);
						cmd_status = ftpdo (mywindow, screens[mywindow].ftpbuf, "");
					}
#endif
				}
			}
		}

		if ((screens[mywindow].clientflags & MGET_STATE) != 0) {
			screens[mywindow].ftpstate = 5;				   /* BYU 3 */
			netputuev (CONCLASS, CONDATA, mytelport,0);	   /* BYU 3 - Stay alive to transfer file */
		}
	}
	return (TRUE);
}
