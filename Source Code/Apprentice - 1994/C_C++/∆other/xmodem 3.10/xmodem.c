/*
 *  XMODEM -- Implements the Christensen XMODEM protocol, 
 *            for packetized file up/downloading.    
 *
 *	See the README file for some notes on SYS V adaptations.
 *	The program has been successfully run on VAXes (4.3BSD) and SUN-3/4s
 *	(SunOS 3.x) against MEX-PC and ZCOMM/DSZ.
 *
 *  See the README and update.doc files for history and change notes.
 *
 *  Please send bug fixes, additions and comments to:
 *	grandi@noao.edu
 */

#include "xmodem.h"
#include "version.h"

main(argc, argv)
int argc;
char **argv;
{
	char *getenv();
	FILE *fopen();
	char *unix_cpm();
	char *strcpy();
	char *strcat();
	char *prtype();
	
	char *fname = filename;		/* convenient place to stash file names */
	char *logfile = "xmodem.log";	/* Name of LOG File */
	
	char *stamptime();		/* for timestamp */

	char *defname = "xmodem.in";	/* default file name if none given */

	struct stat filestatbuf;	/* file status info */

	int index;
	char flag;
	long expsect;
	int fd;

	/* initialize option flags */

	XMITTYPE = 't';		/* assume text transfer */
	DEBUG = FALSE;		/* don't keep debugging info in log */
	MOREDEBUG = FALSE;	/* don't keep even more debugging info in log */
	RECVFLAG = FALSE;	/* not receive */
	SENDFLAG = FALSE;	/* not send either */
	BATCH = FALSE;		/* nor batch */
	CRCMODE = FALSE;	/* use checksums for now */
	DELFLAG = FALSE;	/* don't delete old log file */
	LOGFLAG = TRUE;		/* keep log */
	LONGPACK = FALSE; 	/* do not use long packets on transmit */
	MDM7BAT = FALSE;	/* no MODEM7 batch mode */
	YMDMBAT = FALSE;	/* no YMODEM batch mode */
	TOOBUSY = FALSE;	/* not too busy for sleeping in packet read */
	TIPFLAG = FALSE;	/* no special logging on stderr */
	DELAYFLAG = FALSE;	/* don't delay startup for a while */
	NOEOT = FALSE;		/* don't suppress EOT verification */
	CANCAN = FALSE;		/* don't allow CAN-CAN aborts in mid-transfer */
	YMODEMG = FALSE;	/* no YMODEM-G */

	fprintf(stderr, "XMODEM Version %s", VERSION);
	fprintf(stderr, " -- UNIX-Microcomputer File Transfer Facility\n");

	if (argc == 1)
		{
		help();
		exit(-1);
		}

	index = 0;		/* set index for flag loop */

	stopsig();		/* suppress keyboard stop signal */

	while ((flag = argv[1][index++]) != '\0')
	    switch (flag) {
		case '-' : break;
		case 'X' :
		case 'x' : if (DEBUG) {
				MOREDEBUG = TRUE;
			   }
			   else {
		 		DEBUG = TRUE;  /* turn on debugging log */
			   }
			   break;
		case 'C' :
		case 'c' : CRCMODE = TRUE; /* enable CRC on receive */
			   break;
		case 'D' :
		case 'd' : DELFLAG = TRUE;  /* delete log file */
			   break;
		case 'L' :
		case 'l' : LOGFLAG = FALSE;  /* turn off log  */
			   break;
		case 'm' :
		case 'M' : MDM7BAT = TRUE;  /* turn on MODEM7 batch protocol */
			   BATCH   = TRUE;
			   break;
		case 'y' :
		case 'Y' : YMDMBAT = TRUE;  /* turn on YMODEM batch protocol */
			   BATCH   = TRUE;
			   break;
		case 'k' :
		case 'K' : LONGPACK = TRUE;  /* use 1K packets on transmit */
			   break;
		case 't' :
		case 'T' : TOOBUSY = TRUE;  /* turn off sleeping */
			   break;
		case 'p' :
		case 'P' : TIPFLAG = TRUE;  /* turn on special handling for SunOS tip */
			   break;
		case 'w' :
		case 'W' : DELAYFLAG = TRUE;  /* delay startup */
			   break;
		case 'e' :
		case 'E' : NOEOT = TRUE;  /* turn off EOT verification */
			   break;
		case 'n' :
		case 'N' : CANCAN = TRUE;  /* allow mid-transfer CAN-CAN */
			   break;
		case 'g' :
		case 'G' : YMODEMG = TRUE;  /* YMODEM-G mode */
			   CANCAN = TRUE;
			   CRCMODE = TRUE;
			   YMDMBAT = TRUE;
			   break;
		case 'R' :
		case 'r' : RECVFLAG = TRUE;  /* receive file */
			   XMITTYPE = gettype(argv[1][index++]);  /* get t/b */
			   break;
		case 'S' :
		case 's' : SENDFLAG = TRUE;  /* send file */
			   XMITTYPE = gettype(argv[1][index++]);
			   break;
		default  : fprintf(stderr, "Invalid Flag %c ignored\n", flag);
			   break;
	   }

	if (DEBUG)
		LOGFLAG = TRUE;

	if (LOGFLAG)
	   { 
	     if ((fname = getenv("HOME")) == 0)	/* Get HOME variable */
		error("XMODEM Fatal Error- Can't get Environment!", FALSE);
	     fname = strcat(fname, "/");
	     fname = strcat(fname, logfile);
	     if (!DELFLAG)
		LOGFP = fopen(fname, "a");  /* append to LOG file */
	     else
		LOGFP = fopen(fname, "w");  /* new LOG file */
	     if (!LOGFP)
		error("XMODEM Fatal Error- Can't Open Log File", FALSE);

	     fprintf(LOGFP,"\n++++++++  %s", stamptime());
	     fprintf(LOGFP,"XMODEM Version %s\n", VERSION);
	     fprintf(LOGFP,"Command line: %s %s", argv[0], argv[1]);
	     for (index=2; index<argc; ++index)
		fprintf(LOGFP, " %s", argv[index]);
	     fprintf(LOGFP, "\n");
	   }

	getspeed();		/* get tty-speed for time estimates */

	if (RECVFLAG && SENDFLAG)
		error("XMODEM Fatal Error- Both Send and Receive Functions Specified", FALSE);

	if (MDM7BAT && (YMDMBAT || YMODEMG))
		error("XMODEM Fatal Error - Both YMODEM and MODEM7 Batch Protocols Specified", FALSE);

	if (!RECVFLAG && !SENDFLAG)
		error("XMODEM Fatal Error - Either Send or Receive Function must be chosen!",FALSE);
	
	if (SENDFLAG && argc==2)
		error("XMODEM Fatal Error - No file specified to send",FALSE);

	if (RECVFLAG && argc==2)
		{
		/* assume we really want CRC-16 in batch, unless we specify MODEM7 mode, unless we explicitly set CRCMODE */ 
		if (!CRCMODE)
			CRCMODE = MDM7BAT ? FALSE : TRUE;
		fprintf(stderr, "Ready for BATCH RECEIVE");
		fprintf(stderr, " in %s mode\n", prtype(XMITTYPE));
		if (!TIPFLAG)
			fprintf(stderr, "Send several Control-X characters to cancel\n");
		logit("Batch Receive Started");
		logitarg(" in %s mode\n", prtype(XMITTYPE));
		strcpy(fname, defname);
		}

	if (RECVFLAG && argc>2)
		{
		if(open(argv[2], 0) != -1)  /* check for overwriting */
			{
			logit("Warning -- Target File Exists and is Being Overwritten\n");
			fprintf(stderr, "Warning -- Target File Exists and is Being Overwritten\n");
			}
		fprintf(stderr, "Ready to RECEIVE File %s", argv[2]);
		fprintf(stderr, " in %s mode\n", prtype(XMITTYPE));
		if (!TIPFLAG)
			fprintf(stderr, "Send several Control-X characters to cancel\n");
		logitarg("Receiving in %s mode\n", prtype(XMITTYPE));
		strcpy(fname,argv[2]);
		}

	if (RECVFLAG)
		{  
		if (DELAYFLAG)		/* delay if -w requested */
			sleep(TIPDELAY);
		setmodes();		/* set tty modes for transfer */

		while(rfile(fname) != FALSE);  /* receive files */

		flushin();
		restoremodes(FALSE);	/* restore normal tty modes */

		sleep(2);		/* give other side time to return to terminal mode */
		exit(0);
		}

	if (SENDFLAG && BATCH) 
		{
		if (YMDMBAT)
			{
			ytotleft = 0l;
			yfilesleft = 0;
			for (index=2; index<argc; index++) {
				if (stat(argv[index], &filestatbuf) == 0) {
					yfilesleft++;
					ytotleft += filestatbuf.st_size;
					if (XMITTYPE == 't') {
						if((fd=open(argv[index],0)) >= 0) {
							ytotleft += countnl(fd);
							close(fd);
							}
						}
					}
				}
			if (DEBUG)
				fprintf(LOGFP, "DEBUG YMODEM file count: %d, %ld bytes\n", yfilesleft, ytotleft);

			fprintf(stderr, "Ready to YMODEM BATCH SEND");
			fprintf(stderr, " in %s mode\n", prtype(XMITTYPE));
			logit("YMODEM Batch Send Started");
			logitarg(" in %s mode\n", prtype(XMITTYPE));
			}
		else if (MDM7BAT)
			{
			fprintf(stderr, "Ready to MODEM7 BATCH SEND");
			fprintf(stderr, " in %s mode\n", prtype(XMITTYPE));
			logit("MODEM7 Batch Send Started");
			logitarg(" in %s mode\n", prtype(XMITTYPE));
			}
		if (!TIPFLAG)
			fprintf(stderr, "Send several Control-X characters to cancel\n");

		if (DELAYFLAG)		/* delay if -w requested */
			sleep(TIPDELAY);
		setmodes();
		for (index=2; index<argc; index++) {
			if (stat(argv[index], &filestatbuf) < 0) {
				logitarg("\nFile %s not found\n", argv[index]);
				tlogitarg("\nFile %s not found\n", argv[index]);
				continue;
			}
			sfile(argv[index]);
		}
		sfile("");
		flushin();
		restoremodes(FALSE);

		logit("Batch Send Complete\n");
		tlogit("Batch Send Complete\n");
		sleep(2);
		exit (0);
		}

	if (SENDFLAG && !BATCH) 
		{
		if (stat(argv[2], &filestatbuf) < 0)
			error("Can't find requested file", FALSE);
		expsect = (filestatbuf.st_size/128)+1;
			
		fprintf(stderr, "File %s Ready to SEND", argv[2]);
		fprintf(stderr, " in %s mode\n", prtype(XMITTYPE));
		fprintf(stderr, "Estimated File Size %ldK, %ld Sectors, %ld Bytes\n",
	    	  (filestatbuf.st_size/1024)+1, expsect,
	  	  filestatbuf.st_size);
		projtime(expsect, stdout);
		if (!TIPFLAG)
			fprintf(stderr, "Send several Control-X characters to cancel\n");
		logitarg("Sending in %s mode\n", prtype(XMITTYPE));

		setmodes();
		sfile(argv[2]);
		flushin();
		restoremodes(FALSE);

		sleep(2);
		exit(0);
		}
}
