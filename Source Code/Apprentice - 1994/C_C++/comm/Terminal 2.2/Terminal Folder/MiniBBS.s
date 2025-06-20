/*
	Terminal 2.0
	"MiniBBS.s"

This script sets up a small BBS. To try it out connect another computer
directly to your Macintosh (an ImageWriter II cable can be used to connect
two Macintosh computers, or the same Macintosh computer between the modem
and the printer port) and set the variable "modem" to 0.

In real operation you would set the variable "modem" to 1, configure your
modem to auto-answer and wait for others to call you. Configure the modem
so that a drop of the DTR line is recognized as a command to hang up. This
way to hang up is more reliable than sending the "+++" escape sequence,
followed by the command "ATH0", to the modem.

Note: the script works fine with my modems. My old modem had problems to
recognize if the other end had hung up. If the other end hanged up the
carrier detect stayed on, my modem did not hang up and continually sent me
a lot of garbage. If the X/Y-Modem file transfer detects errors it would
wait for the line to clear before giving up. But because the modem
continued to send garbage there was no way to get out of the file transfer.
This has been corrected since "Terminal 1.9" and now the X/Y-Modem file
transfer will not wait endlessly for the line to clear but for one minute
and then gives up. This makes unattended operation more reliable, as it
should be for a BBS!

When the BBS receives the string "RING\r" it waits for ten seconds to
receive the string "CONNECT\r", "CONNECT 1200\r" or "CONNECT 2400\r" (this
indicates that the modem has answered an incoming call at 300, 1200 or 2400
Baud) and then it prompts for a password after adapting its speed. Once the
correct password is given the BSS will prompt for commands. The password is
not echoed to the caller, nor is it echoed to the screen. The password to
use is hard-coded into the script. There is a retry limit of 3 on the
password, then the BBS will hang up. The BBS is command line oriented. You
enter commands on a line using a command name followed by a parameter if
needed, then press "Return".

There are some timeout values hard-coded into the script. If waiting for
commands the BBS will timeout and log you out when no command is received
after some time. If you make 10 consecutive command errors you are logged
out. This measure was necessary, because my (old) modem did send me a
continous stream of garbage when the other side hanged up (see note above)
and this blocked the BBS. There is also a login timeout. If you have been
logged in for some time you are logged out automatically. While promptimg
for a new command the state of the DCD (data carrier detect) is checked. If
carrier is lost the BBS logs out.

The upload and download commands will not use MacBinary format, but it
should be easy to change the script so that they do. Any file uploaded is
stored on the disk as received. It may be a file from another computer
system, not necessarily a Macintosh. If you want to make Macintosh files
available to your subscribers you must first convert them to MacBinary
format using the "Make MacBinary..." menu item of "Terminal" before storing
them in the down- and upload folder. An upload cannot use the name of an
existing file. There is no way to delete files. The folder on the disk
where all file operations take place is the folder that can be set up using
the "Binary file transfer" menu option. There is no way to switch to
another folder thru BBS commands.

This script is a good example of the "Terminal" script language. It shows
most constructs available in this subset of the C language and most of the
built-in intrinsic functions. You should especially look for and understand
the recognition of commands, where an array of function pointers is used,
some pointer arithmetic is done, and functions are called indirectly thru
pointers.

Note that the possibility to cancel scripts depends on the goodwill of the
script itself. It should test the return codes of the intrinsic functions
it calls and recognize the cancel return code (2) and then exit gracefully.
*/

/* ----- Configuration ------------------------------------------------- */

int modem = 1;				/* 0: direct connect, 1: auto-answer modem */
int carrier = 1;			/* 0: ignore DCD, 1: check DCD */
int LIMIT0 = 60*60;			/* Timeout for password (ticks) */
int LIMIT1 = 3*60*60;		/* Timeout for command prompt (ticks) */
int LIMIT2 = 7200;			/* Timeout for login session (seconds) */
char PASSWORD[] =			/* Login password */
	"LET ME IN";

/* ----- Global data --------------------------------------------------- */

char Version[] = "2.0";

int TIMEOUT = 1;
int CANCEL = 2;		/* Script canceled by menu command */
int ABORT = 3;		/* Abort by (2 consecutive) control-X characters */
int ILLEGAL = 4;	/* Wrong password */
int NOCARRIER = 5;	/* No data carrier (DCD off) */
int QUIT = 6;

char M_timeout[]	= "\r*** Timeout\r";
char M_error[]		= "*** Error\r";
char M_prompt[]		= "\r> ";
char M_fileErr[] 	= "\r*** File error %i\r";
char M_catalog[]	= " BYTES    LAST MODIFIED    NAME\r";
char M_file[]		= "%6i  %s  %s\r";
char M_abort[]		= "Press control-X twice to abort\r";

char *Days[] = {			/* Weekday names */
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *Month[] = {			/* Month names */
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int IGNORE = -1;

char Line[256];				/* Command line */
char *Mark;					/* Running pointer into command line */
int Timer;					/* Login timer */
int Errors;					/* Error counter */

/* ----- Verify file name ---------------------------------------------- */

/* In order to make sure nobody can access any folder other than the down-
and upload folder, we must check for ':' in file names. Imagine the
following commands, that some hacker might send:
	TYPE ::Terminal Folder:miniBBS.s
	DOWNLOAD HD:Confidential:Very confidential file */

checkname (char *name)
{
	char c;

	if (*name == '.' || *name == 0)
		return -37;		/* Illegal file name */
	while (c = *name++)
		if (c == ':')
			return -37;	/* Illegal file name */
	return 0;			/* File name ok */
}

/* ----- Convert date and time to string ------------------------------- */

sec2str (int sec, char *s)
{
	int year, month, day, hour, minute, second, weekday;

	date(sec, &year, &month, &day, &hour, &minute, &second, &weekday);
	format(s, "%02i-%s-%04i %02i:%02i:%02i %s",
		day, Month[month-1], year, hour, minute, second, Days[weekday-1]);
}

/* ----- Type date and time -------------------------------------------- */

clock (int sec)
{
	char s[30];

	sec2str(sec, s);
	type("%s\r", s);
}

/* ----- Type modem intialisation string ------------------------------- */

/* This is for my Abaton InterFax 24/96 modem:
	E0		Commands are not echoed to the screen
	X1		OK, ERROR, CONNECT speed, RING, NO CARRIER responses
	S2=255	Modem does not recognize escape command
	S0=2	Auto-answer, wait for 2 RINGs
	S7=60	Wait time for carrier is 60 seconds
	S10=14	If carrier lost for more than 1.4 seconds hang up
	S9=12	Carrier must be present for 1.2 seconds to be recognized
	&C1		DCD shows that a carrier is present only when one is
	!C1		The DCD is output on pin 5 (CTS), pin 8 not used */

setmodem ()
{
	type("ATE0X1S2=255S0=2S7=60S10=14S9=12");
	if (carrier)
		type("&C1!C1\r");
	else
		type("&C0!C0\r");
}

/* ----- Check for data carrier detect --------------------------------- */

dcd ()
{
	if (carrier)
		return getcts();	/* DCD has been moved to pin 5 (CTS) */
	else
		return 1;
}

/* ----- Main function ------------------------------------------------- */

main ()
{
	int flg = 0, e = 0, i;

	display("MiniBBS %s activated (%i bytes free)\r", Version, stack());
	setup(
		3,		/* 2400 baud */
		1,		/* 8 data */
		0,		/* no parity */
		0,		/* 1 stop */
		IGNORE,	/* port: no change */
		IGNORE,	/* DTR: no change */
		0);		/* Handshake: none */
	terminal(
		0,		/* Local echo off */
		0,		/* Remote echo off */
		0,		/* AutoLF off */
		1);		/* Save & display on */
	protocol(
		0,		/* no MacBinary */
		0,		/* no QuickB */
		0,		/* no ZModem */
		0);		/* no Zmodem autoreceive */
	xyparms(
		1,		/* crc */
		1,		/* 1K */
		0,		/* no batch */
		600);	/* timeout = 10 sec */
	text(
		"",		/* No prompt string */
		0,		/* No line delay */
		0);		/* No character delay */
	macrol("MiniBBS.m");	/* Load macros file */

	while (!e && flg != CANCEL) {
		lecho(0);	/* Local echo off */
		recho(0);	/* Remote echo off */

		if (modem) {
			setdtr(0);	/* Negate DTR: modem hangs up */
			pause(60);
			setdtr(1);	/* Assert DTR: now back in command mode */

			/* Try (up to 5 times) to reset and setup modem */

			for (i = 0; i < 5; ++i) {
				type("ATZ\r");							/* Reset modem */
				if (!(e = prompt("OK\r", 120))) {
					pause(30);
					setmodem();
					if (!(e = prompt("OK\r", 120))) {
						lecho(1);						/* Local echo on */
						break;
					}
				}
			}
			if (e)
				break;	/* Could not setup modem */

			/* Wait until modem says "RING" (or script is canceled) */

			if (!(flg = prompt("RING\r", 0))) {	/* No timeout */

				/* See what speed (ignore additional "RING"s) */

				i = IGNORE;
				while (i == IGNORE) {
					char *s;
					if (flg = nextline(Line, 600))
						break;
					s = Strip(Line);
					if (!strcmp(s, "CONNECT 2400"))
						i = 3;	/* 2400 Baud */
					else if (!strcmp(s, "CONNECT 1200"))
						i = 2;	/* 1200 Baud */
					else if (!strcmp(s, "CONNECT"))
						i = 0;	/* 300 Baud */
				}
				if (!flg) {

					/* Set speed and enter command loop */

					setup(i, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
					pause(300);	/* Give modem a chance */
					flg = loop();
				}
			}

		} else {		/* Direct connection (wait for CR) */
			if (!(flg = prompt("\r", 0)))
				flg = loop();
		}
	}
	if (modem) {
		setdtr(0);	/* Negate DTR: modem hangs up */
		pause(60);
	}
	setdtr(1);	/* Assert DTR: now back in command mode */
	if (e == TIMEOUT)
		display("Modem problem\r");
	display("End of MiniBBS mode\r");
	return 1;	/* Restore previous settings */
}

/* ----- Strip leading control characters (like LFs) ------------------- */

Strip(char *line)
{
	while (*line && (*line < 0x20 || *line > 0x7E))
		++line;
	return line;
}

/* ----- Login session ------------------------------------------------- */

loop ()
{
	int flg, retry;
	char s[30];

	/* Prompt for password (3 retries), wait for it, check it */

	retry = 3;
	while(retry--) {
		sec2str(time(), s);
		type("\r%s  Password [%s]: ", s, PASSWORD);
		save(0);							/* Save & display off */
		flg = nextline(Line, LIMIT0);
		save(1);							/* Save & display on */
		if (flg == CANCEL)
			return flg;
		if (!dcd())
			return NOCARRIER;
		if (flg == TIMEOUT)
			type(M_timeout);
		if (flg == 0) {
			if (!strcmp(Strip(Line), PASSWORD))
				break;						/* Correct password */
			flg = ILLEGAL;					/* Wrong password */
		}
	}
	if (flg) {
		type("\rBye bye...\r");
		return flg;							/* Wrong password */
	}

	/* Password was correct, now logged in */

	macrox(0, 0);							/* Type date & time and ... */
	clock(Timer = time());					/* 	start login timer */
	recho(1);								/* Remote echo on */

	/* Prompt and wait for commands (with timeout) */

	Errors = 0;
	flg = 0;
	while (!flg) {
		type(M_prompt);
		if ((flg = nextline(Line, LIMIT1)) == CANCEL)
			return flg;						/* Cancel */
		if (!dcd())
			return NOCARRIER;
		if (!flg) {
			flg = command();				/* Execute command */
			if ((time() - Timer) > LIMIT2)
				flg = TIMEOUT;				/* Set timeout */
		}
		if (Errors > 10)
			flg = TIMEOUT;
	}

	/* Log out (command has set "flg" to non-zero value) */

	recho(0);						/* Remote echo off */
	if (flg == TIMEOUT)
		type(M_timeout);
	macrox(1, 0);
	datetime();
	return flg;						/* Timeout, cancel or quit */
}

/* ----- Execute command line ------------------------------------------ */

command ()
{
	char name[256];			/* Command name extracted from command line */
	char **p = Commands;	/* Pointer into command names array */

	Mark = Strip(Line);		/* Reset command line pointer */
	if (getnext(name)) {	/* Get first word from command line */
		while (*p) {		/* Search thru valid command names */
			if (!strcmp(name, *p)) {
				Errors = 0;
				return (Pointers[p - Commands])();	/* Execute command */
			}
			++p;
		}
	}
	type(M_error);			/* Command not recognized */
	++Errors;
	return 0;
}

/* ----- Get next word from command line ------------------------------- */

getnext (char *s)
{
	int n;

	n = 0;
	while (*Mark == ' ' && n < 255) {	/* Skip leading spaces */
		++Mark;
		++n;
	}
	n = 0;
	while (*Mark && *Mark != ' ' && n < 255) {	/* Copy word */
		*s++ = *Mark++;
		++n;
	}
	*s = '\0';		/* \0 as end of string */
	if (*Mark)
		++Mark;		/* Skip blank, now Mark -> name */
	return n;		/* Return string length */
}

/* ----- Help command -------------------------------------------------- */

help ()
{
	char **p = Helps, **q = Commands;

	while (*p && *q)
		type("%-5s %s\r", *q++, *p++);
	return 0;		/* Don't quit */
}

/* ----- Info command -------------------------------------------------- */

info ()
{
	type("MiniBBS version %s\r", Version);
	macrox(2, 0);
	return 0;		/* Dont't quit */
}

/* ----- Quit command -------------------------------------------------- */

quit ()
{
	return QUIT;	/* Now quit */
}

/* ----- Type date & time and login time ------------------------------- */

datetime ()
{
	int t = time();

	clock(t);
	t = t - Timer;
	type("Login time is %i'%02i\" of %i'%02i\" maximum\r",
		t / 60, t % 60, LIMIT2 / 60, LIMIT2 % 60);
	return 0;		/* Don't quit */
}

/* ----- Type file directory ------------------------------------------- */

directory ()
{
	int i;
	char name[32];
	int data, rsrc, creat, modif, ftype;
	char md[32];

	if (*Mark) {	/* File name specified */
		if (!(i = checkname(Mark)))
			i = catalog(0, Mark, &ftype, &data, &rsrc, &creat, &modif);
		if (i)
			type(M_fileErr, i);
		else {
			type(M_catalog);
			sec2str(modif, md);
			md[17] = '\0';	/* Truncate string */
			type(M_file, data, md, Mark);
		}
	} else {		/* No file name specified */
		type(M_catalog);
		for (i = 1; ; ++i) {
			if (catalog(i, name, &ftype, &data, &rsrc, &creat, &modif))
				break;
			sec2str(modif, md);
			md[17] = '\0';	/* Truncate string */
			if (type(M_file, data, md, name)) {
				++i;
				break;
			}
		}
		type("%i files listed\r", i - 1);
	}
	return 0;		/* Don't quit */
}

/* ----- Type TEXT file ------------------------------------------------ */

typefile ()
{
	int err;

	if (!(err = checkname(Mark))) {
		type(M_abort);
		err = send(Mark);
	}
	if (err)
		type(M_fileErr, err);
	return 0;		/* Don't quit */
}

/* ----- Type macro text ----------------------------------------------- */

typemacro ()
{
	int i;
	char name[50];

	if (*Mark >= '0' && *Mark <= '9') {
		type(M_abort);
		macrox(val(Mark), 0);
	} else
		for (i = 0; i < 10; ++i) {
			macrox(i, 2, name);
			if (*name)
				type("[%i] %s\r", i, name);
		}
	return 0;		/* Don't quit */
}

/* ----- Download file (X-Modem) --------------------------------------- */

downloadfile ()
{
	int err;
	int ftype, data, rsrc, creat, modif;

	if (!(err = checkname(Mark))) {
		/* First see if file exists */
		if (!(err = catalog(0,Mark,&ftype,&data,&rsrc,&creat,&modif))) {
			type("Please start download. %s", M_abort);
			err = upload(Mark, 0, 0);
		}
	}
	if (err)
		type(M_fileErr, err);
	return 0;		/* Don't quit */
}

/* ----- Upload file (X-Modem) ----------------------------------------- */

uploadfile ()
{
	int err;
	int ftype, data, rsrc, creat, modif;

	if (!(err = checkname(Mark))) {
		if (!(err = catalog(0,Mark,&ftype,&data,&rsrc,&creat,&modif)))
			type("*** File <%s> already exists\r", Mark);
		else {
			type("Please start upload. %s", M_abort);
			err = download(Mark, 0, 0);
		}
	}
	if (err)
		type(M_fileErr, err);
	return 0;		/* Don't quit */
}

/* ----- Command info -------------------------------------------------- */

char *Commands[] = {		/* Command keywords */
	"?",
	"DIR",
	"DOWN",
	"INFO",
	"MACRO",
	"QUIT",
	"TIME",
	"TYPE",
	"UP",
	0
};

char *Helps[] = {			/* Help text for each command */
	"         Display this help text",
	"[name]   Directory of file[s]",
	" name    Download file (X-Modem CRC 1K)",
	"         Display system information",
	"[number] Type macro text / list macros",
	"         Logout of the system",
	"         Show date and time",
	" name    Type text file",
	" name    Upload file (X-Modem CRC 1K)",
	0
};

int Pointers[] = {			/* Pointers to command functions */
	help,
	directory,
	downloadfile,
	info,
	typemacro,
	quit,
	datetime,
	typefile,
	uploadfile,
	0
};
