/*			dcpxfer.c

			Revised edition of dcp

			Stuart Lynne May/87

			Copyright (c) Richard H. Lamb 1985, 1986, 1987
			Changes Copyright (c) Stuart Lynne 1987
			Portions Copyright � David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/
/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* file send routines */
#include "dcp.h"
#include <unix.h>
#include <ctype.h>

#include "dcpxfer.proto.h"
static int getfile(void);

static unsigned char	rpacket[MAXPACK+4];
static unsigned char	spacket[MAXPACK+4];
static long				xtime, tsize;
static char				id[20];
static struct tm		*now;
static int				reportRfileError;
/**/
/***************SEND PROTOCOL***************************/
/*
 *  s d a t a
 *
 *  Send File Data
 */
sdata(void)
{
int response;

	while( TRUE ) {

		if (response = ( *sendpkt) ( spacket, size, 0 ))
			return (0 );     /* send data */
		if ((size=bufill((char *)spacket)) == 0)  /* Get data from file */
			return( 'Z' ); 						/* If EOF set state to that */
		tsize += (long)size;
	}
}


/*
 *  b u f i l l
 *
 *  Get a bufferful of data from the file that's being sent.
 *  Only control-quoting is done; 8-bit & repeat count prefixes are
 *  not handled.
 */
bufill(char *buffer)
{
	return( read(fp, buffer, pktsize) );/* Handle partial buffer */
}


/*
 *  s b r e a k
 *
 *  Send Break (EOT)
 */
sbreak(void)
{
	int	len, i;
	sprintf((char *)spacket, "H");
	if ((*sendmsg)(spacket, FALSE))
		return(0);
	if ((*getmsg)(spacket, &len))
		return(0);
	printmsg( 2, "Switch modes" );
	if (spacket[1] == 'N')
		return('G');
	return('Y');
}


/**/
/*
 *  s e o f
 *
 *  Send End-Of-File.
 */
seof(void)
{
	int	len, i;
	long int ticks, bytes;
/* send end-of-file indication, and perhaps receive a lower-layer ACK/NAK */
	switch ((*eofpkt)()) {
	 case 'R':			/* retry */
	 	printmsg(0, "Remote system asks that the file be resent");
		lseek(fp, 0L, SEEK_SET);
		size = bufill((char *)spacket);
		tsize = (long)size;
		(*filepkt)();				/* warmstart file-transfer protocol */
	 	return 'D';					/* stay in data phase */
	 case 'N':
	 	return(0);					/* cannot send file */
	 case 'Y':
	 	break;						/* sent, proceed */
	 case 'Q':
	 	printmsg(0, "Remote gave up... try again later");
		xfer_problems ++;
	 	return 'A';
	}
	if((*getmsg)(rpacket, &len)) {
		printmsg(0, "Never got yea/nay response at end-of-file");
		xfer_problems ++;
		return(0); /* didn't rec CY or CN packet */
	}
	if(strncmp((char *)rpacket, "CY", 2)) {
		printmsg(0, "Remote had trouble with the file");
		xfer_problems ++;
		return(0); /* cant send file */
	}
#ifdef	THINK_C
	xtime = TickCount() - xtime;
#endif
	close(fp);
	fp = (-1);
 	importpath( hostfile, fromfile );
	UNLINK(hostfile);
	printmsg( 1, "Transfer of %s (%s) completed.", fromfile, hostfile);
#ifdef	THINK_C
	ticks = xtime ? xtime : 1;
	bytes = remote_stats.bsent - remote_stats.bstart;
	printmsg( 0, "Copy succeeded, %ld chars/sec", (bytes * 60) / ticks);
	time(&theTime);
	now = localtime(&theTime);
	fprintf( syslog, "%s!%s (%d/%d-%02d:%02d:%02d) -> %ld / %0.2f secs\n",
					 nodename, id,
					 now->tm_mday, (now->tm_mon+1),
					 now->tm_hour, now->tm_min, now->tm_sec, tsize, xtime/60.0 );
#else
	printmsg( 0, "Copy succeeded");
#endif
	return('F');                    /* go get the next file to send */
}


/**/
/*
 *  s f i l e
 *
 *  Send File Header.
 */
sfile(void)
{
	int	i, len, resp;
	char * cp;
    char tmpfilename[FILENAME_MAX];
	if (fp == -1) {/* If not already open, */
next:	printmsg( 3, "looking for next file..." );
		if (getfile()) { /* get next file from current work*/
			if (fw != (FILE *)NULL) {
				fclose(fw);
				fw = (FILE *)NULL;
			}
			UNLINK( cfile );/* close and delete completed workfile */
			if (Main_State == Cancel_Call) {
				return ('H'); /* semi-polite disconnect */
			}
			return('B'); /* end sending session */
		}

 		importpath( hostfile, fromfile );
 
      if (*tofile == 'S')  {
         printmsg(3, "Opening %s (%s) for sending.", fromfile, hostfile);
		fp = OPEN (hostfile, 0);/* open the file to be sent */
         if (fp == -1)
         {
         	if (errno == ENOENT) {
         		printmsg(0, "sfile: File %s (%s) does not exist, skipped", fromfile, hostfile);
         		return 'F';
         	} else {
           		printmsg(0, "sfile: Cannot open file %s (%s), error %d", fromfile, hostfile, errno);
				xfer_problems ++;
            	return 'A';
            }
         } /* if */
      } /* if */
/*--------------------------------------------------------------------*/
/*    If the target is a directory, automatically put a file into     */
/*    it rather than on it!                                           */
/*--------------------------------------------------------------------*/
      else {
#ifdef STAT
         struct  stat    statbuf;

         if ((hostfile[strlen(hostfile) - 1] == '/') ||
             ((stat(hostfile , &statbuf) == 0) &&
              (statbuf.st_mode & S_IFDIR)))
         {
            char *slash = strrchr( rmtfname, '/');
            if ( slash == NULL )
               slash = rmtfname;
            else
               slash ++ ;
            printmsg(3,"sfile: Destination \"%s\" is directory, appending filename \"%s\"", hostfile, slash);
            if (hostfile[strlen(hostfile) - 1] != '/')
               strcat(hostfile, "/");
            strcat( hostfile, slash );
         } /* if */
#endif
      } /* else */
   } /* if */
   else
      return 'A'; /* Something's already open.  We're in trouble! */

	if (*tofile == 'S') {
		printmsg( 1, "Sending %s (%s) to %s", fromfile, hostfile, rmtfname);
		printmsg( 0, "Send %s -> %s", fromfile, rmtfname);
	} else {
		printmsg( 1, "Receiving %s (%s) from %s", fromfile, hostfile, rmtfname);
		printmsg( 0, "Recv %s <- %s", fromfile, rmtfname);
	}
	xtime = TickCount();
	remote_stats.bstart = remote_stats.bsent;
   if ((*sendmsg)((char *) tofile, FALSE))
           return 0;      /* send 'S fromfile tofile user - tofile 0666' */

   if ((*getmsg)((char *) spacket, &len))
      return 0;

   if (spacket[1] != 'Y') {
   		if (spacket[0] == 'R') {
	   		switch (spacket[2]) {
	   			case '2':
	   				printmsg(0, "Remote is not willing to send that file");
	   				break;
	   			case '6':
	   				printmsg(0, "Remote says the file is too big to send");
	   				break;
	   			default:
	   				printmsg(0, "Remote won't send file, didn't say why");
	   				break;
	   		}
	   		goto next;
	   	} else {
			xfer_problems ++;
	   		switch (spacket[2]) {
	   			case '2':
	   				printmsg(0, "Cannot send; remote reports trouble with work files");
	   				break;
	   			case '4':
	   				printmsg(0, "Cannot send; remote has denied permission");
	   				break;
	   			default:
	   				printmsg(0, "Cannot send; remote didn't say why");
	   				break;
	   		}
    		return 'A';    /* If other side says no, then quit */
	   	}
   }

   if (*tofile == 'S') {
		size = bufill((char *)spacket);
		tsize = (long)size;
	}
   else  {
			/* Try to open a new file */
		if ( strncmp( hostfile, "~/", 2 ) == SAME ) {
			sprintf( tmpfilename, "%s%s", pubdir, hostfile+1);
			fp = CREAT (tmpfilename, 0775, 'b');
		} else {
			fp = CREAT( hostfile, 0775, 'b' );
		}
		if (fp == -1) { 
			printmsg( 0, "cannot create %s", tofile ); /* Give up if can't */
			return('A');
		}
   }

	(*filepkt)(); /* warmstart file-transfer protocol */
	return(char)(*tofile == 'S' ? 'D' : 'R');
                              /* Is this how to do it ? */

}


/**/
/*
 *  s i n i t
 *
 *  Send Initiate: send this host's parameters and get other side's back.
 */
sinit(void)
{
	if ((*openpk)(TRUE))
		return('X');
	return('B');
}

/*
   g e t f i l e

   Reads the next line from the presently open call file
   (*workfile) and determines from this the next file to be sent
   (*fromfile).  If there are no more, TRUE is returned.
*/
static int getfile(void)
{
   char line[BUFSIZ];
   char fname[FILENAME_MAX], tname[FILENAME_MAX], dname[FILENAME_MAX];
   char type[5], who[20], flgs[16];
   int i;

   if (fgets(line, BUFSIZ, fw) == (char *)NULL)
      return TRUE;

   i = strlen(line) - 1;
   if (line[i] == '\n')            /* remove new_line from card */
      line[i] = '\0';

   sscanf(line, "%s %s %s %s %s %s", type, fname, tname, who, flgs, dname);

   strcpy(tofile, line);
   if (*type == 'R')  {
      del_file_flag = FALSE;        /* we aren't about to delete anything! */
      strcpy(fromfile, tname);      /* NOTE - strange use of the name 'fromfile' */
      strcpy( rmtfname, fname );    /* Save for sfile() magic  */
   }
   else if (strcmp(dname, "D.0") == 0)  {
           del_file_flag = FALSE;
           strcpy(fromfile, fname);
           strcpy(rmtfname, tname);
   }
   else  {
           del_file_flag = TRUE;
           strcpy(fromfile, dname); /* correct name according to Nutshell */
           strcpy(rmtfname, tname);
   }

   printmsg(3, "getfile: fromfile=%s, tofile=%s.", fromfile, tofile);

   return FALSE;

} /*getfile*/


/**/
/*********************** MISC SUB SUB PROTOCOL *************************/
/*
**
**schkdir
** scan the dir
*/
schkdir(void)
{
	char	c;
	c = scandir();
	if (c == 'Q') {
		return('Y');
	}
	if (c == 'S') {
		sprintf((char *)rpacket, "HN");
		if ((*sendmsg)(rpacket, FALSE))
			return(0);
	}
	return('B');
}


/**/
/*
 *
 *      endp() end protocol
 *
*/
endp(int polite)
{
	if (polite) {
		sprintf((char *)rpacket, "HY");
		(*sendmsg)(rpacket, TRUE); /* dont wait for ACK */
		zzz(2);			/* allow peer a moment to hear msg before hangup occurs */
	} else {
		printmsg(0, "Closing connection (cancelled by user)");
	}
	(*closepk)();
	printmsg( 0, "Conversation complete");
	return('P');
}


/**/
/***********************RECIEVE PROTOCOL**********************/
/*
 *  r d a t a
 *
 *  Receive Data
 */
rdata(void)
{
	int	len, wrote;
	long int ticks, bytes;
	int resp;
	resp = (*getpkt)(rpacket, &len);
	if (resp == 'R') { /* Retransmission required */
		return resp;
	}
	if (resp != 0)
		return(0);
	if (len > 0) {
		wrote = write(fp, (char *)rpacket, len);/* Write the data to the file */
		tsize += (long)len;
		if (len != wrote && !reportRfileError) {
			printmsg( 0, "Write error, tried %d, wrote %d", len, wrote);
			reportRfileError = 1;
		}
		return('D');/* Remain in data state */
	}
	if (len < 0) {
		wrote = write(fp, (char *)rpacket, -len);/* Write the final data to the file */
		tsize -= (long)len;
		if (-len != wrote && !reportRfileError) {
			printmsg( 0, "Write error, tried %d, wrote %d", -len, wrote);
			reportRfileError = 1;
		}
	}
#ifdef	THINK_C
	xtime = TickCount() - xtime;
#endif
	close(fp);
	fp = -1;
	if ((*sendresp)(OK)) {
		return (0);
	}
	if (reportRfileError) {
		strcpy((char *)spacket, "CN5");
	} else {
		strcpy((char *)spacket, "CY");
	}
	if ((*sendmsg)(spacket, FALSE)) {
		return (0);
	}
	ticks = xtime ? xtime : 1;
	bytes = remote_stats.breceived - remote_stats.bstart;
	if (reportRfileError) {
		printmsg( 0, "Copy failed");
		xfer_problems ++;
	} else {
		printmsg( 0, "Copy succeeded, %ld chars/sec", (bytes * 60) / ticks);
	}
	time(&theTime);
	now = localtime(&theTime);
	fprintf( syslog, "%s!%s (%d/%d-%02d:%02d:%02d) <- %ld / %0.2f secs\n",
					 rmtname, id,
					 now->tm_mday, (now->tm_mon+1),
					 now->tm_hour, now->tm_min, now->tm_sec, tsize, xtime/60.0 );
	return('F');
}


/*
   r f i l e

   Receive File Header
*/

int rfile(void)
{
   char tmpfilename[FILENAME_MAX];
   char buf[BUFSIZ];
   char *flds[10], *cp;
   int rx;                /* TRUE = Receive of file in progress  */
   int nondefault;        /* TRUE = non default directory target */
   int public;			  /* TRUE = destined for public folder   */
   int numflds;

   FILE *stream;

   printmsg(3, "rfile: entered");

   if (Main_State == Cancel_Call) {
      return ('H');
   }
   
   reportRfileError = 0;

   for (cp = buf; ; ) {
      int len;
      if ((*getmsg)((char *) rpacket, &len))
         return 0;
      memcpy(cp, (char *) rpacket, len);
      cp += len;
      if (cp[-1] == '\0')
         break;
   }

/*--------------------------------------------------------------------*/
/*        Return if the remote system has no more data for us         */
/*--------------------------------------------------------------------*/

   if ((buf[0] & 0x7f) == 'H')
      return 'C';    /* the other side (master) is done */

/*--------------------------------------------------------------------*/
/*                  Begin transforming the file name                  */
/*--------------------------------------------------------------------*/

   printmsg(3, "rfile: command \"%s\"", buf);

   numflds = getargs( buf, flds );

   rx = (*flds[0] == 'S');         /* flag we are receiving a file */

   if (rx)
      cp = flds[2];
   else
      cp = flds[1];

   nondefault = (strchr(cp , '/') != NULL) || (strchr(cp , '\\') != NULL);
   strcpy(tmpfilename,cp);

	if ( strncmp( cp, "~/", 2 ) == SAME ) {
		sprintf( tmpfilename, "%s%s", pubdir, cp+1);
		public = 1;
	} else {
		strcpy( tmpfilename, cp );
		public = 0;
	}
   printmsg(3, "rfile: destination \"%s\"", cp);

/*--------------------------------------------------------------------*/
/* If the name has a path and we don't allow it, reject the transfer  */
/*--------------------------------------------------------------------*/

   if (nondefault && ! public) 
   {
        if (rx) {
			printmsg(0,"File %s not destined for spool directory, rejected.", cp);
			sprintf((char *)rpacket, "SN2");
		} else {
			printmsg(0,"File %s not in spool directory, will not be sent", cp);
			sprintf((char *)rpacket, "RN2");
		}
		if ((*sendmsg)(rpacket, FALSE))
			return(0);
		return 'F'; /* Reject this one but keep going */
   }

/*--------------------------------------------------------------------*/
/*       Check if the name is a directory name (end with a '/')       */
/*--------------------------------------------------------------------*/

#ifdef STAT
   if (rx && ((cp[strlen(cp) - 1] == '/') ||
              ((stat(fromfile , &statbuf) == 0) &&
               (statbuf.st_mode & S_IFDIR))))
#else
   if (rx && ((cp[strlen(cp) - 1] == '/')))
#endif
   {
      printmsg(3, "rfile: destination is directory \"%s\"", flds[1]);
      if ((cp = strrchr(flds[1], '/')) == NULL)
         cp = flds[1];
      else
         cp++;
      printmsg(3, "rfile: directory only, adding \"%s\"", cp);

      if ( tmpfilename[ strlen( tmpfilename ) - 1 ] != '/')
         strcat(tmpfilename, "/");
      strcat(tmpfilename, cp);
   } /* if */

   if (rx)
      printmsg(3, "rfile: receive file \"%s\"", tmpfilename);

/*--------------------------------------------------------------------*/
/*          Let host munge filename as appropriate                    */
/*--------------------------------------------------------------------*/

#ifdef IMPORTREMOTE
   importpath(tofile, tmpfilename, rmtname);
#else
   importpath(tofile, tmpfilename);
#endif

   printmsg(3, "rfile: host file \"%s\"", tofile);

/*--------------------------------------------------------------------*/
/*       Don't allow files to be overlaid                             */
/*--------------------------------------------------------------------*/

   if (rx && nondefault)
   {
      stream = fopen(tofile, "r");  /* Determine if the file exists
                                       by opening it for input       */
      if ( stream != NULL )         /* Does it exist?                */
      {
		fclose(stream);
		printmsg(0, "rfile: Cannot receive \"%s\", \"%s\" exists",
                     flds[1], tofile);
		sprintf((char *)rpacket, "SN4");
		if ((*sendmsg)(rpacket, FALSE))
			return(0);
		return 'F'; /* Reject this one but keep going */
      }
   } /* if rx */

/*--------------------------------------------------------------------*/
/*            The filename is transformed, try to open it             */
/*--------------------------------------------------------------------*/

   if (rx)
   {
		fp = CREAT( tofile, 0775, 'b' );

		if (fp == -1){
			printmsg(0, "rfile: cannot open file %s (%s), error %d.",
                        tmpfilename, tofile, errno);
			sprintf((char *)rpacket, "SN4");
			if ((*sendmsg)(rpacket, FALSE))
				return(0);
			return 'F'; /* Reject this one but keep going */
		} /* if */

		printmsg( 0, "Rcve %s <- %s", tofile, flds[1] );
		remote_stats.bstart = remote_stats.breceived;
		sprintf((char *)rpacket, "SY");
		xtime = TickCount();
		tsize = 0L;
		strcpy(id, flds[3]);
		if ((*sendmsg)(rpacket, FALSE))
			return(0);
		(*filepkt)(); /* init for file transfer */
		return('D'); /* Switch to data state */
   } /* if rx */
   else
   {
      printmsg(3, "Opening %s (%s) for sending.", flds[1], tofile);
		fp = OPEN(tofile, 0);/* open the file to be sent */
		if (fp == -1) {/* If bad file pointer, give up */
         printmsg(0, "rfile: Cannot open file %s (%s), error %d.", flds[1], tofile, errno);
		sprintf((char *)rpacket, "RN2");
		if ((*sendmsg)(rpacket, FALSE))
			return(0);
		return 'F'; /* Reject this one but keep going */
      } /* if */

      strcpy( (char *) rpacket, "RY");
      if ((*sendmsg)((char *) rpacket, FALSE))
         return 0;
	  printmsg( 1, "Sending %s (%s) as %s", flds[1], hostfile, tofile);
	  printmsg( 0, "Send %s -> %s", flds[1], tofile);
	  size = bufill((char *)spacket);
	  tsize = (long)size;
	  (*filepkt)(); /* init for file transfer */
      return 'S';     /* Switch to send data state */
   } /* else */

} /*rfile*/


/**/
/*
 *  r i n i t
 *
 *  Receive Initialization
 */
rinit(void)
{
	if ((*openpk)(FALSE))
		return(0);
	return('F');
}




