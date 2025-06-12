/*
*  Background procedures for rcp and ftp
*
*/
#include "stdio.h"
#include "memory.h"
#include "fcntl.h"

#include "whatami.h"
#include "hostform.h"

#define HTELNET 23
#define HRSHD 514
#define HFTP 21
#define BUFFERS 8000
#define PATHLEN 256

#ifdef PC

#define RCPSEGSIZE 1024
#define EOLCHAR 10

#ifdef MSC
#define O_RAW O_BINARY
#endif

#else

#define MACBINARY

#ifdef MACBINARY
#include "MacBinary.h"
int
	MacBinary=0;
MBFile
	*MBopen(),
	*mbfp=NULL;
extern int
	defaultv;						/* Default Volume from Maclook/Menu */
#endif

#define RCPSEGSIZE 512
#define O_RAW O_RDONLY
#define EOLCHAR 13
#endif


int32 atol(),lseek();

char *firstname(),*nextname(),*nextfile;

static  int
	ftpenable=0,				/* is file transfer enabled? */
	rcpenable=0,				/* is rcp enabled? */
	ftpdata=-1,					/* port for ftp data connection */
	fnum=-1,					/* port number for incoming ftp */
	rsnum=-1,					/* port number for incoming rshell */
	rserr=-1;					/* port number for rshd() stderr */

#ifdef PC
static unsigned char xs[BUFFERS+10],	/* buffer space for file transfer */
		pathname[PATHLEN],			/* space to keep path names */
		newfile[PATHLEN],			/* current file being received */
		myuser[17],					/* user name on my machine */
		hisuser[17],				/* user name on his machine */
		waitchar;					/* character waiting for from net */
#else
static  unsigned char *xs=0L,	/* buffer space for file transfer */
		*pathname=0L,			/* space to keep path names */
		*newfile=0L,			/* current file being received */
		myuser[17],					/* user name on my machine */
		hisuser[17],				/* user name on his machine */
		waitchar;					/* character waiting for from net */
#endif PC

static int 
	curstate = -1,			/* state machine for background processes */
	retstate = 200,			/* to emulate a subroutine call */
	ftpstate = 0,			/* state of the ftp data transfer */
	isdir=0,				/* flag for rcp target pathname */
	waitpos=0,				/* marker for gathering strings from net */
	cnt=0,					/* number of characters from last netread() */
	fh=0,					/* file handle when transfer file is open */
	ftpfh=0,				/* file handle for ftp data */
	rc=0,					/* telnet flag */
	xp=0,					/* general pointer */
	towrite=0,				/* file transfer pointer */
	len=0;					/* file transfer length */

static long int
	filelen=0L;				/* length of current file for transfer */

static char mungbuf[1024],crfound=0;



extern char Sptypes[NPORTS];			/* flags for port #'s */

#define PFTP 1
#define PRCP 2
#define PDATA 3

#ifdef PC

#define ga()  while (!netwrite(rsnum,"",1)) netsleep(0)

/************************************************************************/
/*  unsetrshd
*   remove the acceptance of rshd calls (rcp)
*/
unsetrshd()
	{
	netclose(rsnum);
	rsnum = -1;
	rcpenable = 0;
}

/************************************************************************/

setrshd()
{
	int i;
/*
*  set up to receive a rsh call connection 
*/
	if (rsnum >= 0)
		return(0);
	curstate = 199;					/* waiting for connection */
	i = netsegsize(RCPSEGSIZE);
	rsnum = netlisten(HRSHD);
	netsegsize(i);
	if (rsnum >= 0)
		Sptypes[rsnum] = PRCP;

	rcpenable = 1;
}


/************************************************************************/
/*  rshell
*   take an incoming rshell request and service it.  Designed to handle
*   rcp primarily.
*/
rshd(code)
	int code;
	{
	int i,j;

	if (!rcpenable)
		return(0);

	switch (curstate) {
		case 199:					/* wait to get started */
			if (code != CONOPEN)
				break;

			curstate = 0;
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */

			break;
			
/*
* in effect, this is a subroutine that captures network traffic while
* waiting for a specific character to be received
*/
		case 50:
			while (0 < (cnt = netread(rsnum,&xs[waitpos],1))) {
				if (xs[waitpos] == waitchar) {
					curstate = retstate;
					netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */
					break;
				}
				else 
					waitpos += cnt;
			}
			netpush(rsnum);
			break;

		case 51:				/* for recursion, passes straight through */
			break;

		case 0:					/* waiting for first string */
			retstate = 1;
			curstate = 50;
			waitchar = 0;
			waitpos = 0;
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */
			break;

		case 1:					/* we have received stderr port number */
			i = atoi(xs);		/* port number */
			curstate = 51;
#ifdef notneeded
/*
*  caution, netrespond calls netsleep()
*  which will call this routine
*  careful with the synchronicity!
*/
			if (i)		/* zero means, don't bother */
				rserr = netrespond(i,rsnum,1);	/* respond to rsh */
			else
#else
			if (i) {
				cnt = -1;		/* abort it all, we don't take rsh */
				break;
			}
			else
#endif
				rserr = -1;

			retstate = 2; curstate = 50;
			waitpos = 0; waitchar = 0;
			break;

		case 2:				/* get user name, my machine */
			strncpy(myuser,xs,16);

			retstate = 3; curstate = 50;
			waitpos = 0; waitchar = 0;
			break;

		case 3: 			/* get user name, his machine */
			strncpy(hisuser,xs,16);
/*			ftransinfo(hisuser); */

			retstate = 4; curstate = 50;
			waitchar = 0; waitpos = 0;

			break;

		case 4:
/*			ftransinfo(xs);*/
/*
* ACK receipt of command line
*/
			if (rserr >= 0)
				netwrite(rserr,&xp,1);		/* send null byte */
			else {
				ga();			/* send NULL on main connection */
			}

			if (!strncmp(xs,"rcp ",4)) {
/*
*  rcp will be using wildcards, target must be a directory
*/
				if (!strncmp(&xs[4],"-d -t",5)) {
					strncpy(pathname,&xs[10],PATHLEN);
					if (direxist(pathname)) {
/*						ftransinfo("no directory by that name ");*/
						netwrite(rsnum,"\001 No dir found ",16);
						netpush(rsnum);
						cnt = -1;
						break;
					}

					isdir = 1;
					retstate = 20; curstate = 50;
					waitchar = '\012'; waitpos = 0;

					ga();		/* ready for them to start */
					break;
				}
/*
* target could be a directory or a complete file spec
*/
				if (!strncmp(&xs[4],"-t",2)) {
					strncpy(pathname,&xs[7],PATHLEN);
					if (!direxist(pathname)) 
						isdir = 1;
					else
						isdir = 0;

					retstate = 20 ; curstate = 50;
					waitchar = '\012'; waitpos = 0;

					ga();			/* ready for rcp to start */
					break;
				}
/*
*  rcp is requesting me to transfer file(s) (or giving directory name)
*/
				if (!strncmp(&xs[4],"-f",2)) {
					strncpy(pathname,&xs[7],PATHLEN);

/*
*  direxist returns whether the path spec refers to a directory, and if
*  it does, prepares it as a prefix.  Therefore, if it is a dir, we append
*  a '*' to it to wildcard all members of the directory.
*  Firstname() takes a file spec (with wildcards) and returns a pointer
*  to a prepared ACTUAL file name.  nextname() returns successive ACTUAL
*  filenames based on firstname().
*/
					if (!direxist(pathname)) {
						i = strlen(pathname);
						pathname[i] = '*';		/* all members of directory*/
						pathname[++i] = '\0';
					}
					nextfile = firstname(pathname);

					if (nextfile == NULL) {
/*						ftransinfo(" file or directory not found ");*/
						netwrite(rsnum,"\001 File not found ",18);
						netpush(rsnum);
						cnt = -1;
					}
					else {
						/* wait for other side to be ready */
						retstate = 30;	curstate = 50;
						waitchar = 0; waitpos = 0;
					}
					break;
				}
			}

			break;

		case 20:
			xs[waitpos] = '\0';		/* add terminator */

/*
*  get working values from command line just received
*  open file for receive
*/
			if (xs[0] != 'C' || xs[5] != ' ') {
/*				ftransinfo(" Cannot parse filename line "); */
				netwrite(rsnum,"\001 Problem with file name ",26);
				cnt = -1;
				break;
			}

			filelen = atol(&xs[6]);

			for (i = 6; xs[i] != ' '; i++) 
				if (!xs[i]) {
/*					ftransinfo(" premature EOL ");*/
					netwrite(rsnum,"\001 Problem with file name ",26);
					cnt = -1;
					break;
				}

			strcpy(newfile,pathname);		/* path spec for file */

			if (isdir)						/* add file name for wildcards */
				strcat(newfile,&xs[++i]);

			if (0 > (fh = creat(newfile,O_RAW))) {
				netwrite(rsnum,"\001 Cannot open file for write ",29);
				cnt = -1;
				break;
			}
			netputevent(USERCLASS,RCPBEGIN,-1);
			ga();							/* start sending the file to me */
			xp = len = 0;
			curstate = 21;					/* receive file, fall through */
			break;

		case 21:
			do {
			/* wait until xs is full before writing to disk */
				if (len <= 0) {
					if (xp) {
						write(fh,xs,xp);
						xp = 0;
					}
					if (filelen > (long)BUFFERS)
						len = BUFFERS;
					else
						len = (int)filelen;
				}

				cnt = netread(rsnum,&xs[xp],len);

				filelen -= (long)cnt;
				len -= cnt;
				xp += cnt;

/*				printf(" %ld %d %d %d ",filelen,len,xp,cnt);
				n_row(); n_puts(""); */

				if (filelen <= 0L || cnt < 0) {
					write(fh,xs,xp);		/* write last block */
					close(fh);
					fh = 0;
					
					/* wait for NULL byte at end after closing file */
					curstate = 50;  retstate = 22;
					waitchar = 0;   waitpos = 0;
					break;
				}

			} while (cnt > 0);
			break;

		case 22:
			/* cause next sequence of bytes to be saved as next filename
				to transfer     */
			ga();			/* tell other side, I am ready */
			waitchar = '\012'; waitpos = 0;
			curstate = 50; retstate = 20;
			break;

/*
*  transfer file(s) to the sun via rcp
*/
		case 30:
			if (0 > (fh = open(nextfile,O_RAW))) {
				netwrite(rsnum,"\001 File not found ",19);
/*				ftransinfo("Cannot open file to transfer: ");
				ftransinfo(nextfile); */
				cnt = -1;
				break;
			}
			netputevent(USERCLASS,RCPBEGIN,-1);
			filelen = lseek(fh,0L,(short)2);	/* how long is file? */
			lseek(fh,0L,0);				/* back to beginning */

			for (i=0,j=-1; nextfile[i] ; i++)
				if (nextfile[i] == '\\')
					j = i;

			sprintf(xs,"C0755 %lu %s\012",filelen,&nextfile[j+1]);
			netwrite(rsnum,xs,strlen(xs));	/* send info to other side */

/*			ftransinfo(xs);					 check it */

			retstate = 31; curstate = 50;
			waitchar = 0;  waitpos = 0;

			towrite = xp = 0;
			break;

		case 31:
/*
*   we are in the process of sending the file 
*/
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */

			if (towrite <= xp) {
				towrite = read(fh,xs,BUFFERS);
				xp = 0;
				filelen -= (long)towrite;
			}
			i = netwrite(rsnum,&xs[xp],towrite-xp);
			if (i > 0)
				xp += i;

/*			printf(" %d %d %d %ld\012",i,xp,towrite,filelen);
			n_row();
*/
/*
*  done if:  the file is all read from disk and all sent
*  or other side has ruined connection
*/
			if ((filelen <= 0L && xp >= towrite) || netest(rsnum)) {
				close(fh);
				fh = 0;
				nextfile = nextname();		/* case of wildcards */
				ga(); 
				netputuev(SCLASS,RCPACT,rsnum);
				if (nextfile == NULL)
					retstate = 32;
				else
					retstate = 30;
				curstate = 50;
				waitchar = 0;	waitpos = 0;
			}
			break;
		case 32:
			cnt = -1;
			break;
		case 5:
			break;
		default:
			break;

	}

/*
*  after reading from connection, if the connection is closed,
*  reset up shop.
*/
	if (cnt < 0) {
		if (fh > 0) {
			close(fh);
			fh = 0;
		}
		curstate = 5;
		cnt = 0;
		netclose(rsnum);
		rsnum = -1;
		netputevent(USERCLASS,RCPEND,-1);

		setrshd();					/* reset for next transfer */
	}


}

#endif

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/************************************************************************/
/*  ftp section
*   This should be extracted from rcp so that it compiles cleanly
*/

#define CRESP(A)  netpush(fnum);netwrite(fnum, messs[(A)], strlen(messs[(A)]))

#define FASCII 0
#define FIMAGE O_RAW
#define FAMODE 0
#define FIMODE 1
#define FMMODE 2			/* Mac Binary, when ready */

static int rfstate,
	portnum[8],
	ftpfilemode=FASCII,			/* how to open ze file */
	ftptmode=FAMODE;			/* how to transfer ze file on net */

static uint16 fdport;

setftp()
	{
/*
*  set up to receive a telnet connection for ftp commands
*/
	rfstate = 0;
	ftpstate = 0;
	fnum = netlisten(HFTP);
	ftpenable = 1;

	if (fnum >= 0)				/* signal that events should be caught */
		Sptypes[fnum] = PFTP;

	strcpy(myuser,"unknown");	/* set unknown user name */
}

unsetftp()
	{
	rfstate = 0;
	ftpstate = 0;
	netclose(fnum);
	fnum = -1;
	ftpenable = 0;
}

/***********************************************************************/
/*
*  resident ftp server -- enables initiation of ftp without a username
*  and password, as long as this telnet is active at the same time
*  Now checks for the need of passwords.
*/

static char *messs[] = {
#ifdef PC
						"220 PC Resident FTP server, ready \015\012",
#else
						"220 Macintosh Resident FTP server, ready \015\012",
#endif
						"451 Error in processing list command \015\012",
						"221 Goodbye \015\012",						/*2*/
						"200 This space intentionally left blank <   > \015\012",
						"150 Opening connection \015\012",
						"226 Transfer complete \015\012",			/*5*/
						"200 Type set to A, ASCII transfer mode \015\012",
						"200 Type set to I, binary transfer mode \015\012",
						"500 Command not understood \015\012",		/*8*/
						"200 Okay \015\012",
						"230 User logged in \015\012",
						"550 File not found \015\012",				/*11*/
						"501 Directory not present or syntax error\015\012",
						"250 Chdir okay\015\012",
						"257 \"",
						"\" is the current directory \015\012",		/*15*/
						"501 File not found \015\012",
						"504 Parameter not accepted, not implemented\015\012",
						"200 Stru F, file structure\015\012",
						"200 Mode S, stream mode\015\012",		/*19*/
						"202 Allocate and Account not required for this server\015\012",
						"501 Cannot open file to write, check for valid name\015\012",
						"530 USER and PASS required to activate me\015\012",
						"331 Password required\015\012",      /*23 */
						"530 Login failed\015\012",
						"200 MacBinary Mode enabled\015\012",
						"200 MacBinary Mode disabled\015\012",  /*26 */
						"552 Disk write error, probably disk full\015\012",
						"214-NCSA Telnet FTP server, supported commands:\015\012",
						"    USER  PORT  RETR  ALLO  PASS  STOR  CWD  XCWD  XPWD  LIST  NLST\015\012",
#ifdef MAC
						"    HELP  QUIT  MODE  TYPE  STRU  ACCT  NOOP  MACB\015\012",  /*30*/
						"    MACB is MacBinary and must be done with TYPE I\015\012",
#else
						"    HELP  QUIT  MODE  TYPE  STRU  ACCT  NOOP\015\012",
						"    A Macintosh version of NCSA Telnet is also available.\015\012",
#endif
						"214 Direct comments and bugs to telbug@ncsa.uiuc.edu\015\012",
						"200 Type set to I, binary transfer mode [MACBINARY ENABLED]\015\012",				/* 33 */
						"200 Type set to I, binary transfer mode [macbinary disabled]\015\012",
				""};

rftpd(code)
	int code;
	{
	int i;

	if (!ftpenable)
		return(0);

	netpush(fnum);

	switch (rfstate) {
		case 0:
			if (code != CONOPEN) 
				break;
			ftpfilemode = FASCII;
			ftptmode = FAMODE;
			netputevent(USERCLASS,FTPCOPEN,-1);
#ifndef PC
			if (!xs) xs=(char *)NewPtr(BUFFERS+10);
			if (!pathname) pathname=(char *)NewPtr(PATHLEN);
			if (!newfile) newfile=(char *)NewPtr(PATHLEN);
#endif PC
			rfstate = 1;				/* drop through */
		case 1:
			CRESP(0);
			netgetftp(portnum,fnum);	/* get default ftp information */
			for (i=0; i<4; i++)			/* copy IP number */
				hisuser[i] = portnum[i];
			fdport = portnum[6]*256+portnum[7];

			waitpos = 0; waitchar = '\012';
			rfstate = 50;       		/* note skips over */
			if (Sneedpass()) 
				retstate = 3;				/* check pass */
			else
				retstate = 5;				/* who needs one ? */
			break;
		case 3:				/* check for passwords */
		case 4:
			waitpos = 0;  waitchar = '\012';
			rfstate = 50;  
			if (!strncmp("USER",xs,4)) {
				if (strlen(xs) < 6)				/* make sure blank name stays blank */
					xs[5] = myuser[0] = 0;
				strncpy(myuser,&xs[5],16);		/* keep user name */
				netputevent(USERCLASS,FTPUSER,-1);
				CRESP(23);
				retstate = 4;		/* wait for password */
				break;
			}
			if (!strncmp("PASS",xs,4)) {
				if (Scheckpass(myuser,&xs[5])) {
					netputevent(USERCLASS,FTPPWOK,-1);
					CRESP(10);
					retstate = 5;
				}
				else {
					netputevent(USERCLASS,FTPPWNO,-1);
					CRESP(24);
					retstate = 3;
				}
				break;
			}
			if (!strncmp("QUIT",xs,4)) {
				CRESP(2);
				cnt = -1;
			}
			else {
				CRESP(22);
			}
			retstate = 3;			/* must have password first */
			break;				
				
/*
*  interpret commands that are received from the other side
*/
			
		case 5:
#ifdef PC
			for (i=4; i< strlen(xs); i++)
				if (xs[i] == '/')		/* flip slashes */
					xs[i] = '\\';
#endif

/*
*  set to a safe state to handle recursion
*  wait for another command line from client
*  
*/
			rfstate = 50; retstate = 5;
			waitchar = '\012';  waitpos = 0;

			if (!strncmp(xs,"LIST",4) || !strncmp(xs,"NLST",4)) {
				if ((strlen(xs) < 6) || xs[5] == '.')
					strcpy(xs,"LIST *");

				nextfile = firstname(&xs[5]);	/* find first name */
				if (nextfile == NULL) {
					CRESP(16);
				}
				else {
					ftpgo();			/* open the connection */
					fdport = portnum[6]*256+portnum[7]; /* reset to def */
					if (ftpdata >= 0)
						Sptypes[ftpdata] = PDATA;
					ftpstate = 40;		/* ready to transmit */
					CRESP(4);
					netputevent(USERCLASS,FTPLIST,-1);
				}
			}
			else if (!strncmp(xs,"CWD",3)) {
				if (chgdir(&xs[4])) {			/* failed */
					CRESP(12);
				}
				else {						/* success */
					CRESP(13);
				}
			}
			else if (!strncmp(xs,"STOR",4)) {

#ifdef MACBINARY
				if ((mbfp = MBopen(&xs[5],defaultv,MB_WRITE +
					(((!MacBinary) || (ftptmode == FAMODE)) ? MB_DISABLE : 0
					))) == 0L) {
					CRESP(21);
					break;
				}
				else
					ftpfh = 12;
#else

				if (0 > (ftpfh = creat(&xs[5],ftpfilemode))) {
					CRESP(21);
					break;
				}
#endif

				ftpstate = 0;

				strncpy(newfile,&xs[5],PATHLEN-1);

				ftpgo();				/* open connection */
				fdport = portnum[6]*256+portnum[7]; /* reset to def */
				if (ftpdata >= 0)
					Sptypes[ftpdata] = PDATA;

				CRESP(4);

				ftpstate = 30;		/* ready for data */
			}

			else if (!strncmp(xs,"RETR",4)) {

#ifdef MACBINARY
			if ((mbfp = MBopen( &xs[5], defaultv, MB_READ + ((!MacBinary) ||
				(ftptmode == FAMODE)) ? MB_DISABLE : 0)) == 0L) {
				CRESP(11);
				break;
			}
			ftpfh = 12;
#else
				if (0 > (ftpfh = open(&xs[5],ftpfilemode))) {
					CRESP(11);
					break;
				}
#endif
				strncpy(newfile,&xs[5],PATHLEN-1);

				ftpgo();				/* open connection */
				fdport = portnum[6]*256+portnum[7]; /* reset to def */

				ftpstate = 20;		/* ready for data */
				if (ftpdata >= 0)
					Sptypes[ftpdata] = PDATA;

				CRESP(4);
			}
			else if (!strncmp(xs,"TYPE",4)) {
				if (toupper(xs[5]) == 'I') {
					ftpfilemode = FIMAGE;
					ftptmode = FIMODE;
#ifdef MACBINARY
				if (MacBinary) {
					CRESP( 33);						/* Binary on, MACB ON */
				}
				else {
					CRESP( 34);						/* Binary on, MACB off */
#else
					CRESP(7);
#endif MACBINARY
				}
				}
				else if (toupper(xs[5]) == 'A') {
					ftpfilemode = FASCII;
					ftptmode = FAMODE;
					CRESP(6);
				}
				else {
					CRESP(17);
				}

			}
#ifdef MACBINARY
			else if (!strncmp(xs,"MACB",4)) {
				if (toupper(xs[5]) == 'E') {
					MacBinary = 1;
					CRESP(25);
				}
				else {
					MacBinary = 0;
					CRESP(26);
				}
				DisplayMacBinary();			/* post an event ? */
			}
#endif
			else if (!strncmp(xs,"PORT",4)) {
/*
* get the requested port number from the command given
*/
				sscanf(&xs[5],"%d,%d,%d,%d,%d,%d",&portnum[0],&portnum[1],
				&portnum[2],&portnum[3],&portnum[4],&portnum[5]);
				fdport = portnum[4]*256+portnum[5];
				CRESP(3);
			}
			else if (!strncmp(xs,"QUIT",4)) {
				CRESP(2);
				rfstate = 60;
				netputuev(CONCLASS,CONDATA,fnum);	/* post back to me */
			}
			else if (!strncmp(xs,"XPWD",4) || !strncmp(xs,"PWD",3)) {
				CRESP(14);						/* start reply */
				dopwd(xs,1000);					/* get directory */
				netwrite(fnum,xs,strlen(xs));	/* write dir name */
				CRESP(15);						/* finish reply */
			}
			else if (!strncmp(xs,"USER",4)) {
				if (strlen(xs) < 6)				/* make sure blank name stays blank */
					xs[5] = myuser[0] = 0;
				strncpy(myuser,&xs[5],16);		/* keep user name */
				netputevent(USERCLASS,FTPUSER,-1);
				/* confirm log in without password */
				CRESP(10);
			}
			else if (!strncmp(xs,"STRU",4)) {	/* only one stru allowed */
				if (xs[5] == 'F') {
					CRESP(18); }
				else {
					CRESP(17); }
			}
			else if (!strncmp(xs,"MODE",4)) {	/* only one mode allowed */
				if (xs[5] == 'S') {
					CRESP(19); }
				else {
					CRESP(17); }
			}
			else if (!strncmp(xs,"ALLO",4) || !strncmp(xs,"ACCT",4)) {
				CRESP(20); }
			else if (!strncmp(xs,"HELP",4)) {
				for (i=28; i<33; i++) {
					CRESP(i); }
			}
			else if (!strncmp(xs,"NOOP",4)) {
				CRESP(9); }
			else {			/* command not understood */
				CRESP(8); 
			}

			break;

/*
*  subroutine to wait for a particular character
*/
		case 50:
			while (0 < (cnt = netread(fnum,&xs[waitpos],1))) {
				if (xs[waitpos] == waitchar) {
					rfstate = retstate;

					while (xs[waitpos] < 33)		/* find end of string */
						waitpos--;
					xs[++waitpos] = '\0';			/* put in terminator */

					for (i=0; i<4; i++)				/* want upper case */
						xs[i] = toupper(xs[i]);

					break;
				}
				else
					waitpos += cnt;

			}
			break;

		case 60:					/* wait for message to get through */
									/* or connection is broken */
/*			printf("                  %d,%d",netpush(fnum),netest(fnum));*/
			if (!netpush(fnum) || netest(fnum))
				cnt = -1;
			else
				netputuev(CONCLASS,CONDATA,fnum);	/* post back to me */
			break;

		default:
			break;

	}

	if (cnt < 0) {
#ifdef MACBINARY
		if (mbfp) {
			MBclose( mbfp );
			mbfp = NULL;
		}
#else
		if (ftpfh > 0) {
			ftpfh = 0;
			close(ftpfh);
		}
#endif
		if (ftpdata > 0) {
			netclose(ftpdata);
			netputevent(USERCLASS,FTPEND,-1);
		}
		rfstate = 100;
		ftpstate = 0;
		cnt = 0;
		netclose(fnum);
		netputevent(USERCLASS,FTPCLOSE,-1);
		fnum = -1;
		ftpdata = -1;
		setftp();				/* reset it */
	}

}

/***********************************************************************/
/* ftpgo
*  open the FTP data connection to the remote host
*/
ftpgo()
	{
	int savest;
	struct machinfo *m;

	xs[0] = portnum[0];
	xs[1] = portnum[1];
	xs[2] = portnum[2];
	xs[3] = portnum[3];

	netfromport(20);	 /* ftp data port */

	if (NULL == (m = Slookip(xs))) {		/* use default entry */
		if (NULL == (m = Shostlook("default")))
			return(0);
		savest = m->mstat;
		m->mstat = HAVEIP;
		movebytes(m->hostip,xs,4);
		ftpdata = Snetopen(m,fdport);
		m->mstat = savest;
		movebytes(m->hostip,"\0\0\0\0",4);
		return(0);
	}

	ftpdata = Snetopen(m,fdport);

}

/*********************************************************************/
/*
*  FTP receive and send file functions
*/
static int fcnt=0;

ftpd(code,curcon)
	int code,curcon;
	{
	int i;


	if (curcon != ftpdata)		/* wrong event, was for someone else */
		return(0);

	switch (ftpstate) {
		default:
			break;

		case 40:				/* list file names in current dir */

			if (code == CONFAIL)	/* something went wrong */
				fcnt = -1;
			if (code != CONOPEN) 	/* waiting for connection to open */
				break;
			
			ftpstate = 41;

/*
*  send the "nextfile" string and then see if there is another file
*  name to send
*/
		case 41:
			netputuev(SCLASS,FTPACT,ftpdata);
			netpush(ftpdata);
			i = strlen(nextfile);
			if (i != netwrite(ftpdata,nextfile,i)) {
				CRESP(1);
				fcnt = -1;
				break;
			}
			netwrite(ftpdata,"\015\012",2);
			if (NULL == (nextfile = nextname())) {	/* normal end */
				ftpstate = 22;   			/* push data through */
			}
			break;
			
		case 30:
			if (code == CONFAIL)	/* something went wrong */
				fcnt = -1;
			if (code != CONOPEN)	/* waiting for connection to open */
				break;
			ftpstate = 31;
			crfound = 0;
			len = xp = 0;
			filelen = 0L;
			netputevent(USERCLASS,FTPBEGIN,-2);
			
		case 31:
/*
* file has already been opened, take everything from the connection
* and place into the open file: ftpfh
*/
			do {
			/* wait until xs is full before writing to disk */
				if (len <= 2000) {

					if (xp) {
#ifdef MACBINARY
						if (0 > MBwrite(mbfp, xs, xp)) {
							netclose(ftpdata);
							fcnt = -1;
							CRESP(27);
							MBclose( mbfp);			/* Close on Disk Full Error */
							mbfp=NULL;
							break;
						}
#else
						if (0 > write(ftpfh,xs,xp)) { /* disk full err */
							netclose(ftpdata);
							fcnt = -1;
							CRESP(27);
							break;
						}
#endif
						xp = 0;
					}
					len = BUFFERS;		/* expected or desired len to go */
				}

				if (ftptmode == FAMODE)
					fcnt = Sfread(ftpdata,&xs[xp],len);
				else
					fcnt = netread(ftpdata,&xs[xp],len);

				if (fcnt >= 0) {
					len -= fcnt;
					xp += fcnt;
					filelen += fcnt;
				}

				if (fcnt < 0) {
#ifdef MACBINARY
					if (0 > MBwrite( mbfp, xs, xp)) {
						CRESP(27);
						MBclose( mbfp);			/* Close file on error */
						break;
					}
					MBclose( mbfp );
#else
					if (0 > write(ftpfh,xs,xp)) { /* disk full check */
						CRESP(27);
						break;
					}
					close(ftpfh);
#endif
					ftpfh = 0;
					CRESP(5);
				}

			} while (fcnt > 0);
			break;

		case 20:

			if (code == CONFAIL)	/* something went wrong */
				fcnt = -1;
			if (code != CONOPEN)	/* waiting for connection to open */
				break;
			ftpstate = 21;
#ifdef MACBINARY
			filelen = MBsize( mbfp );
#else
			filelen = lseek(ftpfh,0L,2);		/* how long is file? */
			lseek(ftpfh,0L,0);					/* back to beginning */
#endif
			towrite = 0;
			xp = 0;
			netputevent(USERCLASS,FTPBEGIN,-1);

		case 21:
/*
*  transfer file(s) to the other host via ftp request
*  file is already open = ftpfh
*/
			netputuev(SCLASS,FTPACT,ftpdata);
		
			if (towrite <= xp) {

				i = BUFFERS;
#ifdef MACBINARY
				towrite = MBread( mbfp, xs, i);
#else
				towrite = read(ftpfh,xs,i);
#endif
				xp = 0;
			}

			if (towrite <= 0 || netest(ftpdata)) {		/* we are done */
				ftpstate = 22;
				break;
			}

			if (ftptmode == FAMODE)
				i = Sfwrite(ftpdata,&xs[xp],towrite-xp);
			else
				i = netwrite(ftpdata,&xs[xp],towrite-xp);


/*			printf(" %d %d %d \012",i,xp,towrite);
			n_row(); */

			if (i > 0) {
				xp += i;
				filelen -= i;
				if (filelen < 0L)
					filelen = 0L;
			}

			break;

		case 22:		/* wait for data to be accepted */
			netputuev(SCLASS,FTPACT,ftpdata);

			fcnt = netpush(ftpdata);		/* will go negative on err */
			if (!fcnt || netest(ftpdata))
				fcnt = -1;
			if (fcnt < 0) {
				CRESP(5);
			}
			break;

		case 0:
			break;

	}  /* end of switch */

/*
*  after reading from connection, if the connection is closed,
*  reset up shop.
*/
	if (fcnt < 0) {
#ifdef MACBINARY
		if (mbfp) {
			MBclose( mbfp );
			mbfp = NULL;
		}
#else
		if (ftpfh > 0) {
			close(ftpfh);
			ftpfh = 0;
		}
#endif
		ftpstate = 0;
		fcnt = 0;
		if (ftpdata >= 0) {
			netclose(ftpdata);
			netputevent(USERCLASS,FTPEND,-1);
			ftpdata = -1;
		}
	}

}


/***************************************************************************/
/*  Sfwrite
*   Write an EOL translated buffer into netwrite.
*   Returns the number of bytes which were processed from the incoming
*   buffer.  Uses its own 1024 byte buffer for the translation (with Sfread).
*/

Sfwrite(pnum,buf,nsrc)
	int pnum,nsrc;
	char *buf;
	{
	int i,ndone,nout,lim;
	char *p,*q;

	ndone = 0;

	while (ndone < nsrc) {

		if (0 > ( i = netroom(pnum)))
			return(-1);

		if (i < 1024)					/* not enough room to work with */
			return(ndone);
/*
*  process up to 512 source bytes for output (could produce 1K bytes out) 
*/
		if (nsrc - ndone > 512)
			lim = 512;
		else
			lim = nsrc-ndone;

		p = buf + ndone;				/* where to start this block */
		q = mungbuf;					/* where munged stuff goes */
		for (i=0; i < lim; i++) {
			if (*p == EOLCHAR) {
				*q++ = 13;
				*q++ = 10;
				p++;
			}
			else
				*q++ = *p++;
		}
		ndone += lim;					/* # of chars processed */
		nout = q-mungbuf;				/* # of chars new */

		if ( nout != netwrite(pnum,mungbuf,nout) )
			putln("error in Sfwrite");	/* send them on their way */

	}

	return(ndone);
}

/*
*  important note:  for Sfread, nwant must be 256 bytes LARGER than the amount
*  which will probably be read from the connection.
*  Sfread will stop anywhere from 0 to 256 bytes short of filling nwant
*  number of bytes.
*/
Sfread(pnum,buf,nwant)
	int pnum,nwant;
	char *buf;
	{
	int i,ndone,lim;
	char *p,*q;

	if (nwant < 1024)
		return(-1);

	ndone = 0;

	while (ndone < nwant - 1024) {

		if (0 >= (lim = netread(pnum,mungbuf,1024))) {
			if (ndone || !lim)			/* if this read is valid, but no data */
				return(ndone);
			else
				return(-1);				/* if connection is closed for good */
		}

		p = mungbuf;
		q = buf + ndone;

/*		printf("\012 lim=%d done=%d want=%d",lim,ndone,nwant);
		n_row();
*/
		for (i=0; i < lim; i++) {

			if (crfound) {
				if (*p == 10)
					*q++ = EOLCHAR;
				else if (*p == 0)
					*q++ = 13;			/* CR-NUL means CR */
				crfound = 0;
			}
			else if (*p == 13)
				crfound = 1;
			else 
				*q++ = *p;				/* copy the char */

			p++;
		}

		ndone = q-buf;					/* count chars ready */
	}

	return(ndone);
}

/***********************************************************************/
/* Sftpname and Sftpuser and Sftphost
*  record the name of the file being transferred, to use in the status
*  line updates
*/

Sftpname(s)
	char *s;
	{
	strcpy(s,newfile);
}

Sftpuser(user)
	char *user;
	{
	strcpy(user,myuser);			/* user name entered to log in */
}

Sftphost(host)
	char *host;
	{
	movebytes(host,hisuser,4);		/* IP address of remote host */
}

Sftpstat(byt)
	long *byt;
	{
	
	if (ftptmode != FAMODE && MacBinary)	/* MacBinary transfer */
		*byt = -1;
	else
		*byt = filelen;

}

#ifdef MACBINARY
MBstat()
	{
	return(0);
}
#endif
