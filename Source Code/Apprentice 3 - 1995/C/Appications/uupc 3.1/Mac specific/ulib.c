/*		ulib.c


		macintosh library
		

	Things to do in uu host

		serial I/O
		
		directory stuff
			opendir, readdir, closedir

		prolog and epilog

		system call

			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/

#include <stdio.h>
#include "dcp.h"
#ifdef	 THINK_C
#   include <unix.h>
#else
#	include <sgtty.h>
#endif

#include "ulib.proto.h"

/*
 *
 *      login (for slave in SLAVELOG mode)
 * Real dumb login handshake.
 *     waitforever = TRUE if an unlimited number of login attempts should be
 *                   permitted (slave mode), and FALSE if the login process
 *                   should terminate after a few attempts (auto mode).
 *     callup_done = TRUE if some form of inbound-call preparation has been done
 *                   (either via the HAYES dialer or via a DIR chat-script) and
 *                   we have some confidence that there's really a caller out there.
 *                   FALSE if we have no such confidence (usually because there was
 *                   no INBOUND entry in the Systems file, and we've simply opened
 *                   the serial port) and we want to require a carriage return to
 *                   elicit the login prompt.
*/

login(int waitforever, int callup_done) {
	char logmsg[132];
	int len;
	int retries;
	int passTries;
#ifdef SLAVELOG
	msgtime = 1;
	retries = 5;
	while (callup_done && --retries >= 0 && rmsg(logmsg, 0) != 0) {
		; /* flush any dross in the input queue */
	}
	retries = 10;
	do {
		msgtime = 2 * MSGTIME;
		if (!callup_done) {
			if (0 == rmsg(logmsg, 0)) goto next;
		}
		sprintf(logmsg, "\r\n\n%s uupc listener\r\n\nUsername: ", nodename);
		wmsg(logmsg, 0);
		if (0 == rmsg(logmsg, 0)) goto next;
		if (strstr(logmsg, "Username:") || strstr(logmsg, "Password:") || strstr(logmsg, "NO CARRIER")) {
			printmsg(0, "Modem hung up");
			retries = 0;
			goto next;
		}
		printmsg( 0, "Username = %s", logmsg );
		passTries = 2;
getPassword:
		wmsg("\r\nPassword: ", 0);
		if (0 == rmsg(logmsg, 0)) {
			if (--passTries > 0) goto getPassword;
			goto next;
		}
		printmsg( 14, "\r\nPassword = %s", logmsg );
		if(strcmp(logmsg, password) == EQUAL) 
			return 'I';
		wmsg("\r\nSorry, incorrect login\r\n", 0);
next: ;
	} while ((waitforever || --retries > 0) && Main_State == Call_Systems);
#endif SLAVELOG
	return('A');
}


char *inbuf;
char *outbuf;

swrite(char *data, int num) {

	int test;
	unsigned char * cp;

	if (debuglevel > 14)
		fputc( '{', stderr );
	if (debuglevel > 14) {
		test = num;
		cp = (unsigned char *)data;
		while (test--)
			fprintf( stderr, isprint(*cp)? "{%c}":"{%02x}", *cp++ );
	}
	test = (*currentConnection->Write)( data, num );
	if (debuglevel > 14)
		fputc( '}', stderr );
	return( test );

}

void makenull(int refno)
{
	lseek(refno, 0L, SEEK_SET);
	SetEOF(refno, 0L);
}

/* non-blocking read essential to "g" protocol */
/* see "dcpgpkt.c" for description */
/* This all changes in a mtask systems. Requests for */
/* I/O should get qued and an event flag given. Then the */
/* requesting process (e.g.gmachine()) waits for the event */
/* flag to fire processing either a read or a write. */
/* Could be implemented on VAX/VMS or DG but not MS-DOS */
sread (char *buf, int num, long int timeout) {
/*
	return( SIORead( buf, num, num, timeout*10 ) ); OBSOLETE
*/
	int count;
	int test;
	unsigned char * cp;

	if (debuglevel > 13)
		fputc( '[', stderr );
	printmsg( 15, "sread: num: %d  timeout: %d", num, timeout );
		
	count = (*currentConnection->Read) ( buf, num, num, timeout*10 );
	printmsg( 15, "sread: read: %d ", count );

	if (debuglevel > 13 && count > 0) {
		test = count;
		cp = (unsigned char *)buf;
		while (test--)
			fprintf( stderr, isprint(*cp)? "[%c]":"[%02x]", *cp++ );
	}
		
	if (debuglevel > 13)
		fputc( ']', stderr );
	return( count );	
	
}

/* check number of chars in input buffer */

int savail(void)
{
	return (*currentConnection->Avail)();
}



openline(char *name, char *baud, char *phone) {
	int oops;
	printmsg(3, "openline: name: \"%s\"  baud: \"%s\"", name, baud);
	if (strncmp(name, "CTB/", 4) == 0) {
		oops = CTBIOInit(name+4, baud, phone);
	} else {
		oops = SIOInit(name, baud, phone); /* Yes, call SIO directly, currentConnection not yet set up */
	}
	if (oops) {
		return oops;
	}
	inbuf = NewPtr(PORTBUFSIZ);
	if (inbuf) {
		(*currentConnection->InBuffer)(inbuf, PORTBUFSIZ);
	}
	outbuf = NewPtr(PORTBUFSIZ);
	if (outbuf) {
		(*currentConnection->OutBuffer)(outbuf, PORTBUFSIZ);
	}
	return(0);
}


closeline(void)
{
	hangup();
	if (currentConnection) {
		(*currentConnection->Close) ( 1 );
	}
	if (inbuf) {
		DisposPtr(inbuf);
		inbuf = NULL;
	}
	if (outbuf) {
		DisposPtr(outbuf);
		outbuf = NULL;
	}
}

flowcontrol(int software, int hardware)
{
	(*currentConnection->SetFlowCtl) (software, hardware);
}

nodot(int string)
{
}



notimp(int argc, char *argv[])
{
	/*debuglevelMsg("\Pcheck argc (08) and argv (0a) ");*/
	fprintf( stderr, "shell: %s not implemented\n", *argv );
}

/* 		shell


*/

char *getcwd();

shell(char *command, char *inname, char *outname) {

	char *argvec[50];
	int is_ok;

	int rnews();
	int macbin();
	
	int argcp;

	char **argvp;
	char args;
	char *openFlags;
	

	int	(*proto)();
	
	if (strlen(command) == 0) {
		printmsg(0, "Missing or null command in X. file, no action taken");
		return 1;
	}
	
	argcp = 0;

	argcp = getargs( command, argvec );

	argvp = argvec;
	args = argcp;

	if ( debuglevel > 5 ) {
		while ( args ) 
			fprintf( stderr, "arg: %d  %s\n", args--, *argvp++ );
		argvp = argvec;
		args = argcp;
	}
	/* */
	
	proto = notimp;
	openFlags = "r";

	if ( strcmp( *argvp, "rmail" ) == SAME )
		proto = rmail;

	else if ( strcmp( *argvp, "rnews" ) == SAME ) {
		openFlags = "rb";
		proto = rnews;
	}
	
	else if ( strcmp( *argvp, "macbin" ) == SAME ) {
		openFlags = "rb";
		proto = macbin;
	}

	is_ok = 1;
		
	if ( *inname != '\0' ) {
		/* fprintf( stderr, "reopening stdin as %s\n", inname );
		fprintf( stderr, "curdir: %s\n", getcwd(NULL, 0)); /* */
		mapMacCaseness(inname);
		if ( freopen( inname, openFlags, stdin ) == NULL ) {
			fprintf( stderr, "Couldn't open %s, %d\n", inname, errno );
			is_ok = 0;
		}
	}
	if (is_ok) {
		is_ok = (*proto)( argcp, argvp );
	}

	if (freopen( ".console", "r", stdin ) == NULL) {;
		fprintf( stderr, "Couldn't reopen console, %d\n", errno );
	}
	return is_ok;
}


/*
	macbin

	copy a macbinary image from stdin into a macintosh file

		128 bytes of header information
		(n+128)/128*128 bytes of data fork data
		(n+128)/128*128 bytes of resource fork data

	The name is derived from the header information

	The directory is given as an arguement


	on remote system use:

		uux macfile.mb > remotemac!macbin /usr/sl


*/

#undef TRUE

#ifndef	THINK_C
# include <types.h>
# include <pb.h>
#endif

/* #include <stdio.h> */

typedef struct {
	/*
	char				blknum;
	char				mblknum;
	*/
	char				dummy;

	char				version;

	char				fName[64];
	FInfo				finderInfo;			/* check - starts on odd byte boundary */
	char				protected;
	char				ckZero1;
	long				dfLength;
	long				rfLength;
	long				crDat;
	long				mdDat;
	char				fill1[27];
	char				computer;
	char				os;
	} MacBinHeader;



int macbin(int argc, char *argv[])
{
#ifdef TESTETEST
	char	filename[132];
	char	buf[BUFSIZ];
	MacBinHeader mbh;
	long	count, size;
	int		i;

	HPrmBlkRec	pbr;

	/* get the header */
	fread( &mbh.version, 1, 128, stdin );


	/* create and open the file */
	strncpy( pbr.ioNamePtr, mbh.fName, 64 );
	pbr.ioVRefNum = 0;
	pbr.u.hfp.ioFVersNum = 0;
	PBCreate( &pbr, FALSE );


	/* get finder info */
	pbr.u.hfp.ioFDirIndex = 0;
	pbr.u.hfp.ioDirID = 0L;
	PBGetInfo( &pbr, FALSE );
	
	/* set finder info */
	pbr.u.hfp.ioFlFndrInfo.fdFldr = 0;
	pbr.u.hfp.ioFlCrDat = mbh.crDat;
	pbr.u.hfp.ioFlMdDat = mbh.mdDat;
	pbr.u.hfp.ioFlFndrInfo = mbh.finderInfo;
	PBSetInfo( &pbr, FALSE );

	/* get and write out the fork */
	pbr.u.iop.ioPermssn = 0;
	pbr.u.iop.ioMisc = 0L;
	for ( i = 1; i > 0 ; i-- ) {
		if (i == 1 ) {
			PBOpen( &pbr, FALSE );
			size = (mbh.dfLength + 127) & 0xffffff80;
		}
		else {
			PBROpen( &pbr, FALSE );
			size = (mbh.rfLength + 127) & 0xffffff80;
		}
		while ( size > 0 )
			if ( (count = fread( buf, 1, BUFSIZ, stdin )) != 0 ) {
				size -= count;
				pbr.u.iop.ioBuffer = buf;
				pbr.u.iop.ioReqCount = count;
				PBWrite ( &pbr, FALSE);
			}
		PBClose( &pbr, FALSE );
	}
#endif
	return 1;
}
