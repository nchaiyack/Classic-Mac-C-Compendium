/*
 * Contains system routines to get and put bytes, change tty modes, etc
 * Sys V version.  UNTESTED!!!!!!
 */

#include "xmodem.h"

/*
 *
 *	Get a byte from the specified file.  Buffer the read so we don't
 *	have to use a system call for each character.
 *
 */
getbyte(fildes, ch)				/* Buffered disk read */
int fildes;
char *ch;

	{
	static char buf[BUFSIZ];	/* Remember buffer */
	static char *bufp = buf;	/* Remember where we are in buffer */
	
	if (nbchr == 0)			/* Buffer exausted; read some more */
		{
		if ((nbchr = read(fildes, buf, BUFSIZ)) < 0)
			error("File Read Error", TRUE);
		bufp = buf;		/* Set pointer to start of array */
		}
	if (--nbchr >= 0)
		{
		*ch = *bufp++;
		return(0);
		}
	else
		{
		return(EOF);
		}
	}

/* Count the number of newlines in a file so we know the REAL file size */

long
countnl(fd)
int fd;
{
	char buf[BUFSIZ];
	char *bufp;
	long nltot = 0;
	int numchar;
	long lseek();

	while (numchar = read(fd, buf, BUFSIZ))		/* cycle through file */
		for (bufp=buf; numchar--; bufp++)
			if (*bufp == '\n')
				nltot++;

	(void) lseek (fd, 0l, 0);			/* rewind file */
	if (DEBUG)
		fprintf(LOGFP, "DEBUG: countnl--%ld newlines counted\n", nltot);
	return (nltot);
}

/*   CRC-16 constant array...
     from Usenet contribution by Mark G. Mendel, Network Systems Corp.
     (ihnp4!umn-cs!hyper!mark)
*/

/* crctab as calculated by initcrctab() */
unsigned short crctab[1<<B] = { 
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
    };

/* get a byte from data stream -- timeout if "seconds" elapses */

int timedout;
 
int
readbyte(seconds)
int seconds;
{
	int force_it();
	char c;
	signal(SIGALRM, force_it);

	timedout = 0;
	alarm(seconds);
	read(0, &c, 1);
	alarm(0);
	if (timedout)
		{
		if (DEBUG)
			fprintf(LOGFP, "DEBUG: readbyte TIMEOUT after %d seconds\n", seconds);
		return(TIMEOUT);
		}
	if (DEBUG)
		fprintf(LOGFP, "DEBUG: readbyte %02xh\n", c & 0xff);
	return(c & 0xff);
}

int
force_it()
{
	timedout++;
	return;
}


/* flush input stream */

flushin()
	{
/* No good way to do this without select */
/* Perhaps....but we waste 1 second with every call!
	while (readbyte(1) != TIMEOUT)
		;
*/
	}

/* 
 get a buffer (length bufsize) from data stream -- timeout if "seconds" elapses.
 Read bunches of characters to save system overhead;
 Further process data while kernel is reading stream (calculating "checksum").
 Try to nap long enough so kernel collects 100 characters or so until we wake up
 unless TOOBUSY is set.
*/


int
readbuf(bufsize, seconds, tmode, amode, recvsectcnt, checksum, bufctr)

int bufsize,	/* number of chars to be read */
seconds, 	/* timeout period for each read */
tmode, 		/* transmission mode: TRUE if text */
amode, 		/* transmission mode: TRUE if apple macintosh */
*checksum, 	/* pointer to checksum value */
*bufctr;	/* length of actual data string in buffer */
long recvsectcnt;	/* running sector count (128 byte sectors) */

{
	int force_it();
	int numread;		/* number of chars read */
	int left;		/* number of chars left to read */
	int recfin = 0;		/* flag that EOF read */
	char inbuf[BBUFSIZ];	/* buffer for incoming packet */
	register unsigned char c;	/* character being processed */
	register unsigned short chksm;	/* working copy of checksum */
	register int bfctr;	/* working copy of bufctr */
	int j;			/* loop index */
	char *sectdisp();

	signal(SIGALRM, force_it);
	chksm = 0;
	bfctr = 0;

	for (left = bufsize; left > 0;) {

		/* read however many chars are waiting */
		timedout = 0;
		alarm(seconds);
		numread = read(0, inbuf, left);
		alarm(0);
		if (timedout)
			return(TIMEOUT);
		left -= numread;

		if (DEBUG)
			fprintf(LOGFP, "DEBUG: readbuf--read %d characters\n", numread);

		/* now process part of packet we just read */

		for (j =  0; j < numread; j++) 
			{  
				buff[bfctr] = c = inbuf[j] & 0xff;
				fileread++;

				if (MOREDEBUG)
					fprintf(LOGFP, "DEBUG: character read %02xh\n", c & 0xff);

				if (CRCMODE)  /* CRC */
					chksm = (chksm<<B) ^ crctab[(chksm>>(W-B)) ^ c];

				else        /* checksum */
		       			chksm = ((chksm+c) & 0xff);

				if (CHECKLENGTH && fileread > filelength)	/* past EOF ? */
					continue;

				if (tmode) 		/* text mode processing */
					{
					buff[bfctr] &= 0x7f;	/* nuke bit 8 */
					if (c == CR || c == 0)	/* skip CRs and nulls */
						continue;
					else if (c == CTRLZ)	/* CP/M EOF char */
						{  
						recfin = TRUE;
		       				continue;
		       				}
		       			else if (!recfin)	/* don't increment if past EOF */
						bfctr++;
					}
				else if (amode) 	/* Apple macintosh text mode processing */
					{
					buff[bfctr] &= 0x7f;	/* nuke bit 8 */
					if (c == 0)		/* skip nulls */
						continue;
					else if (c == CR)	/* translate CR to LF */
						buff[bfctr] = LF;
					else if (c == CTRLZ)	/* CP/M EOF char */
						{  
						recfin = TRUE;
		       				continue;
		       				}
		       			if (!recfin)	/* don't increment if past EOF */
						bfctr++;
					}
				else			/* binary */
					bfctr++;

		     	}	

		/* go to sleep to save uneeded system calls while kernel
		   is reading data from serial line, fudge constant from 10 to
		   9 to avoid sleeping too long
		*/
		if (left && !TOOBUSY)
			sleep ((left<SLEEPNUM ? left:SLEEPNUM) * 9/ttyspeed);
	}

	if (CHECKLENGTH && fileread >= filelength)
		logitarg("File end from YMODEM length found in sector %s\n",
		  sectdisp(recvsectcnt,bufsize,1));
	*checksum = chksm;
	*bufctr = bfctr;
	return(0);
}

/* send a byte to data stream */

sendbyte(data)
char data;
	{
	if (DEBUG)
		fprintf(LOGFP, "DEBUG: sendbyte %02xh\n", data & 0xff);

	if (write(1, &data, 1) != 1)  	/* write the byte (assume it goes NOW; no flushing needed) */
		error ("Write error on stream", TRUE);
	return;
	}

/* send a buffer to data stream */

writebuf(buffer, nbytes)
char *buffer;
int  nbytes;
	{
	if (DEBUG)
		fprintf(LOGFP, "DEBUG: writebuf (%d bytes)\n", nbytes);

	if (write(1, buffer, nbytes) != nbytes)		/* write the buffer (assume no TIOCFLUSH needed) */
		error ("Write error on stream", TRUE);
	return;
	}

/* set and restore tty modes for XMODEM transfers */

struct termio ttys;
struct stat statbuf;		/* for terminal message on/off control */

int wason;			/* holds status of tty read write/modes */
char *tty;			/* current tty name */


setmodes()
	{
	char *ttyname();
	struct termio ttysnew;

	extern onintr();

	sleep(2);			/* let the output appear */
	if (ioctl(0,TCGETA,&ttys)<0)  /* get tty params */
		error("Can't get TTY Parameters", TRUE);

	tty = ttyname(0);  /* identify current tty */
	
	if (ioctl(0,TCGETA,&ttysnew)<0)  /* get tty params */
		error("Can't get TTY Parameters", TRUE);
	ttysnew.c_cc[4] = 1;		/* VMIN */
	ttysnew.c_cc[5] = 0;		/* VTIME */
	ttysnew.c_iflag = 0;
	ttysnew.c_oflag = 0;
	ttysnew.c_lflag = 0;
	ttysnew.c_cflag &= ~CSIZE;
	ttysnew.c_cflag |= CS8;
	ttysnew.c_cflag &= ~PARENB;
	if (ioctl(0,TCSETA,&ttysnew)<0)  /* set new paramters */
		error("Can't set new TTY Parameters", TRUE);

	if (stat(tty, &statbuf) < 0)	/* get tty modes */ 
		{
		logit("Can't get your TTY Status\n");
		tlogit("Can't get your TTY Status\n");
		wason = FALSE;
		}
	else
		{
		if (statbuf.st_mode & 022)
			{
			if (chmod(tty, (int)statbuf.st_mode & ~022) < 0)
				{
				logit("Can't change TTY mode\n");
				tlogit("Can't change TTY mode\n");
				wason = FALSE;
				}
			else 
				wason = TRUE;
			}
		else 
			wason = FALSE;
		}


	/* set up signal catcher to restore tty state if we are KILLed */

	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, onintr);
	}

/* restore normal tty modes */

restoremodes(errcall)
int errcall;
	{
	if (wason)
		if (chmod(tty, (int)statbuf.st_mode | 022) < 0)
			error("Can't change TTY mode", FALSE);
	if (ioctl(0,TCSETA,&ttys) < 0)
		{ if (!errcall)
		   error("RESET - Can't restore normal TTY Params", FALSE);
		else
		     printf("RESET - Can't restore normal TTY Params\n");
		}
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, SIG_DFL);
	return;
	}




/* signal catcher */
onintr()
	{
	error("Kill signal; bailing out", TRUE);
	}

/* create string with a timestamp for log file */

char *stamptime()
{
	char *asctime();		/* stuff to get timestamp */
	struct tm *localtime(), *tp;
	long now;

	time(&now);
	tp = localtime(&now);
	return(asctime(tp));
}



/* get tty speed for time estimates */

getspeed()
	{
	static int speedtbl[] = {0, 50, 75, 110, 134, 150, 200, 300, 600,
	1200, 1800, 2400, 4800, 9600, 19200, 0};
	struct termio ttystemp;

	if (ioctl(0,TCGETA,&ttystemp) < 0)	/* get tty structure */
		error("Can't get TTY parameters", FALSE);
	if ((ttystemp.c_cflag & 017) >= 0 && (ttystemp.c_cflag & 017) <= 14)
		{
		ttyspeed = speedtbl[ttystemp.c_cflag & 017];
		logitarg ("Line speed = %d bits per second\n", ttyspeed);
		}
	else
		{
		ttyspeed = 1200;
		logit ("Can't determine line speed; assuming 1200 bps\n");
		}
	}


/* turn off keyboard stop signal so stray ^X don't put us in background */

stopsig()
	{
	}
