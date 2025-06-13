/*			sio.c


				Serial I/O for rz/sz


				Copyright (c) 1986 Stuart Lynne

				June 1986

				Modified for Think C
				
				Sak Wathanasin 1989

			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/
#include "sio.h"

#include <stdio.h>
#ifndef  THINK_C
#include <pb.h>

#include <osutil.h>

/* #include <max/macglobals.h> */
#include <max/debug.h>
#include <max/asc.h>
#else   THINK_C
# include "dcp.h"
#endif   THINK_C

#ifndef FALSE
# define FALSE 0
#endif FALSE
#ifndef TRUE
# define TRUE 1
#endif TRUE

#ifdef Upgrade
static int g_setting;
#endif Upgrade

struct SIOPort curPort={0};

#ifdef DOASYNC

static struct async_write *asyncout;
static int async_head, async_tail, async_pending;
static int allowInterrupts = FALSE;
static int interruptOccurred = FALSE;
static int sleepLimit = 3;
extern SysEnvRec SysEnv;

#endif

#define baud38400 1

#include "sio.proto.h"
static unsigned int getspeed(int code);
static unsigned getbaud(char *code);

struct {
	unsigned int baudr;
	int speedcode;
} speeds[] = {
	300,	baud300,
	600,	baud600,
	1200,	baud1200,
	2400,	baud2400,
	4800,	baud4800,
	9600,	baud9600,
	19200,	baud19200,
	38400, 	baud38400,
	57600, 	baud57600,
	0,
};

static char otherIn[] = "\p.?In";
static char otherOut[] = "\p.?Out";

ConnectionHandler serialHandler = {
	SIOInit,
	SIOInBuffer,
	SIOOutBuffer,
	SIOSpeed,
	SIOHandshake,
	SIOSetting,
	SIOClose,
	SIOSetFlowCtl,
 	SIOAllowInterrupts,
	SIOInterrupt,
	SIOPurge,
	SIOPutchar,
	SIOWrite,
	SIOAvail,
	SIOWStr,
	SIORead,
	SIOIdle,
	SIOEvent,
	SIOBreak,
	SIOSetParity
};


static unsigned int
getspeed(int code)
{
	register n;

	for (n=0; speeds[n].baudr; ++n)
		if (speeds[n].speedcode == code)
			return speeds[n].baudr;
	return -1;
}


static unsigned
getbaud(char *code) {
	register n;
	register long int Baudrate;
	
	Baudrate = atol(code);
	
	for(n=0 ; speeds[n].baudr ; ++n)
		if(speeds[n].baudr == Baudrate)
			return(speeds[n].speedcode);
	return -1;
}



SIOInit (char *whichport, char *speed, char *phone)
{
	char i;
	SerShk	handshake;
	int setting, baudCode, speedVal;
	int err;
	long int bytes;

	/*  fprintf( stderr, "sioinit %s %s\n", whichport, speed ); /* */
	
	if ( 
		strncmp( whichport, ".a", 2 ) == 0 || 
		strncmp( whichport, "a",1 ) == 0 ||
		strcmp( whichport, "modem") == 0
		) 
	{
		curPort.in = AIN;
		curPort.out = AOUT;
	}
	else if (
		strncmp( whichport, ".b", 2 ) == 0 || 
		strncmp( whichport, "b", 1 ) == 0 ||
		strcmp( whichport, "printer") == 0 
		) 
	{
		curPort.in = BIN;
		curPort.out = BOUT;
	}
	else if (strlen(whichport) == 1 && whichport[0] >= 'c' && whichport[0] <= 'z') {
		otherIn[2] = otherOut[2] = whichport[0];
		curPort.in = otherIn;
		curPort.out = otherOut;
	} else if (strlen(whichport) == 2 && whichport[0] == '.' && whichport[1] >= 'c' && whichport[1] <= 'z') {
		otherIn[2] = otherOut[2] = whichport[1];
		curPort.in = otherIn;
		curPort.out = otherOut;
	} else
		return( -1 );

	baudCode = getbaud(speed);
	if (baudCode == -1) {
		fprintf(stderr, "Baud rate %s is not supported or not recognized\n", speed);
		return (-1);
	}
	
	speedVal = getspeed(baudCode);
	
	if (speedVal <= 9600) {
		sleepLimit = 3;
	} else if (speedVal < 57600) {
		sleepLimit = 1;
	} else {
		sleepLimit = 0;
	}

	if ((err = OpenDriver ( (StringPtr)curPort.in,  &curPort.refin )) !=0 ) {
		fprintf( stderr, "Err: %d\n", err);
#ifdef	DEBUG
		debugMsg ( ctop( curPort.in ) );
#endif
		SysBeep (20); 
		return( -1 );
	}

	
	if ((err=OpenDriver ( (StringPtr)curPort.out, &curPort.refout )) !=0 ) {
		fprintf( stderr, "Err: %d\n", err);
#ifdef	DEBUG
		debugMsg ( ctop( curPort.out ) );
#endif
		SysBeep (20); 
		return( -1 );
	}
	
	SIOHandshake( FALSE, FALSE, FALSE, XON, XOFF );
	SIOSetting( speed, noParity, stop10, data8 );

#ifdef DOASYNC

	bytes = BUFFERS * sizeof (struct async_write);

	asyncout = (struct async_write *) NewPtr(bytes);
	if (asyncout) {
		memset(asyncout, 0, bytes);
	}
	async_head = async_tail = 0;
	allowInterrupts = interruptOccurred = FALSE;

#endif

	currentConnection = &serialHandler;

	return( 0 );
}

SIOSpeed(char *speed)
{
	SIOSetting( speed, curPort.parity, curPort.stopbits, curPort.databits );
}

SIOHandshake (int fInx, int fXOn, int fCTS, int xOn, int xOff)
{	
	int err;
	
	curPort.handshake.fInX = fInx;
	curPort.handshake.fXOn = fXOn;
	curPort.handshake.fCTS = fCTS;

	curPort.handshake.xOn = xOn;
	curPort.handshake.xOff = xOff;
	/* handshake.fXOn = TRUE; */
	
	curPort.handshake.errs = 0;
	curPort.handshake.evts = 0;

	if ((err=SerHShake (curPort.refout, &curPort.handshake ))!=0 ) {
		fprintf( stderr, "Err: %d\n", err);
#ifdef	DEBUG
		debugMsg ("\PSerHShake error");
#endif
		SysBeep (20); 
	}
}

SIOSetting(char *speed, int parity, int stopbits, int databits)
{
	int err;
	int setting;
	
	curPort.baud = getbaud(speed);
	curPort.parity = parity;
	curPort.stopbits = stopbits;
	curPort.databits = databits;
	
	setting = curPort.baud + parity + stopbits + databits;
#ifdef Upgrade
	/* store current settings in a global area */
	g_setting = setting;
#endif Upgrade

	if ((err=SerReset(curPort.refin, setting))!=0 ||
	    (err=SerReset(curPort.refout, setting))!=0)  {
		fprintf( stderr, "Err: %d\n", err);
#ifdef	DEBUG
		debugMsg ("\PSerReset error");
#endif
		SysBeep (20);
	}	
}

SIOInBuffer (char *buf, int size)
{
	curPort.inbuffer = buf;
	curPort.insize = size;
	SerSetBuf( curPort.refin, buf, size ); 
}

SIOOutBuffer (char *buf, int size)
{
	curPort.outbuffer = buf;
	curPort.outsize = size;
	SerSetBuf( curPort.refout, buf, size ); 
}


SIOClose (int dtr)
{
	CntrlParam SIOpb;
	
	long count;
	int i;

	printmsg(2, "Closing serial port session");
	if ( dtr != 0 && curPort.refin != 0) {
		printmsg(2, "Dropping DTR");
		/* drop DTR */
		SIOpb.ioCompletion = NULL;
		SIOpb.ioCRefNum = curPort.refin;
		SIOpb.csCode = 18;
		PBControl ((ParmBlkPtr)&SIOpb, FALSE);
	}
	
	printmsg(2, "Setting input buffer to NULL");
	SIOInBuffer( NULL, 0 );
	printmsg(2, "Setting output buffer to NULL");
	SIOOutBuffer( NULL, 0 );
	if (curPort.refin != 0) {
		printmsg(2, "Killing input port I/O");
		KillIO(curPort.refin);
		printmsg(2, "Closing input port");
		CloseDriver( curPort.refin );
	}
	if (curPort.refout != 0) {
		printmsg(2, "Killing output port I/O");
		KillIO(curPort.refout);
		printmsg(2, "Closing output port");
		CloseDriver( curPort.refout );
	}
	if (asyncout) {
		for (i = 0; i < BUFFERS; i++) {
			if (asyncout[i].buf) {
				printmsg(2, "Disposing output buffer at 0x%lX", asyncout[i].buf);
				DisposPtr(asyncout[i].buf);
			}
		}
		printmsg(2, "Disposing output control data at 0x%lX", asyncout);
		DisposPtr(asyncout);
		asyncout = NULL;
	}
	async_head = async_tail = async_pending = 0;
	curPort.refin = curPort.refout = 0;
	
	currentConnection = (ConnectionHandler *) NULL;
	printmsg(2, "Serial port teardown complete");
}


sfflushout (void)
{
}

SIOSetFlowCtl(int software, int hardware)
{
	SerShk flowRec;
	flowRec.fXOn = software;
	flowRec.fInX = software;
	flowRec.fCTS = hardware;
	flowRec.xOn =  0x11;
	flowRec.xOff = 0x13;
	flowRec.errs = flowRec.evts = 0;
/* 	SerHShake(curPort.refin, &flowRec); */
	SerHShake(curPort.refout, &flowRec);
}

SIOAllowInterrupts(int flag)
{
	int oldFlag;
	oldFlag = allowInterrupts;
	allowInterrupts = flag;
	interruptOccurred &= flag;
	return oldFlag;
}

SIOInterrupt(void)
{
	interruptOccurred = TRUE;
}

SIOPurge (void)
{	/*
	char		ch;

	while (sread (&ch, 1, 1) == 1);
	
	*/
	(void) KillIO(curPort.refin);
	(void) KillIO(curPort.refout);
}

SIOPutchar (char ch)
{
	/*fprintf(stderr, "<%02x>", ch);*/
	return( putu ( &ch, 1, curPort.refout ) );	
}

SIOWrite (char *buf, int count)
{
	return( putu (buf, count, curPort.refout) );
}

int SIOAvail(void)
{
	long int count;
	(void)SerGetBuf(curPort.refin, &count);
	return count;
}


SIOWStr (char *st)
{
	return( putu (st, strlen(st), curPort.refout) );
}

#define	Ticks			(*(long *)			0x16a)
	
 
/* timeout is in tenths of a second */
int SIORead (char *byt, int mincount, int maxcount, long int tenths)
{
	long	timea;
	short	actCount;

	int		i;
	
	timea = Ticks;
	tenths *= 6;
	/* fprintf (stderr, "Read  m= %d T= %ld", maxcount, Ticks  );/**/
	do {
		actCount = getn (byt, mincount, maxcount, curPort.refin );

		if (actCount > 0) {
			/* fprintf (stderr, "  OK: %d\n", actCount);/**/
			return actCount;
			}
		
		if (allowInterrupts && interruptOccurred) {
			return -1;
		}
			
	} while ((long)(Ticks - timea) < tenths);
	
	/* fprintf (stderr, "  Timed out: t= %d\n  T:%ld", tenths, Ticks );/**/
	return -1;
	}

srdchk (void)
{
	long count;

	(void)SerGetBuf(curPort.refin, &count);	
	if (count >= 1) 
		return TRUE;
	else 
		return FALSE;
}

SIOBreak(int tenths) {
	long int ticks;
	ticks = TickCount();
	SerSetBrk(curPort.refout);
	while (TickCount() <= ticks+tenths*6) {
		;;;;;;;;
	}
	SerClrBrk(curPort.refout);
}

SIOSetParity(int dataBitsCode, int parityCode) {
	int setting;
	OSErr err;
	setting = (g_setting & 0xC3FF) | parityCode | dataBitsCode;
	if ((err=SerReset(curPort.refin, setting))!=0  ||
	    (err=SerReset(curPort.refout, setting))!=0)  {
		fprintf( stderr, "Err: %d\n", err);
#ifdef	DEBUG
		debugMsg ("\PSerReset error");
#endif
		SysBeep (20);
	} else {
		g_setting = setting;
	}
}

/* 	Fill array *cp with characters from serial buffer, starting
	at 0, until cmax. Return actual chars read. Don't read unless
	at least cmin are available.
*/
int getn (char *cp, int cmin, int cmax, short refin)
{	
	ParamBlockRec	SIOpb;
	register		ParmBlkPtr	pbp = &SIOpb;

	long			count;
	int				firstTry = TRUE;

	/* fprintf( stderr, " %d", cmin ); /* */
	
tryit:
	(void)SerGetBuf(refin, &count);
	SIOpb.ioParam.ioReqCount = count;
	if ( count >= cmin) {
		if ( count >  cmax) 
			SIOpb.ioParam.ioReqCount = (long) cmax;
   		SIOpb.ioParam.ioRefNum = refin;
	   	SIOpb.ioParam.ioBuffer = (Ptr) cp;
  	 	SIOpb.ioParam.ioPosMode = 0;
		PBRead (&SIOpb, FALSE);
		count = SIOpb.ioParam.ioActCount;
#ifdef MULTIFINDER
		if (Ticks-Last_Check_Event >= MF_DELAY)  
		  if (Check_Events(0)) {
		    if (Main_State == Abort_Program) {
		    	SIOClose(TRUE);
		    	exit(-1);
		    }
		  }
#endif 
		}
	else {
#ifdef MULTIFINDER
		/* the data hasn't been received, ok to delay a bit */
		if (Ticks-Last_Check_Event >= MF_DELAY)  
		  if (Check_Events(sleepLimit)) {
		    if (Main_State == Abort_Program)  {
		    	SIOClose(TRUE);
		    	exit(-1);
		    }
		   }
#endif 
		count = 0;
		if (firstTry) {
			firstTry = FALSE;
			goto tryit;
		}
	}
		

	return( (int)count );
}


static ParamBlockRec  		pb = {0};

int SIOWriteBusy(void)
{
	if (pb.ioParam.ioResult == 1) {
		return TRUE;
	} else {
		return FALSE;
	}
}
	

int putu(char *c, short count, short refout)
{
	register ParmBlkPtr	pbp = &pb;
	int spin;
	int isAsync;

#ifdef DOASYNC
	if (asyncout != NULL) {
		spin = TRUE;
		do {
			if (async_pending == 0) {
				spin = FALSE;
			} else {
				pbp = &asyncout[async_tail].pb;
				if (pbp->ioParam.ioResult != 1) {
					async_pending --;
					async_tail = (async_tail + 1) % BUFFERS;
				} else {			
#ifdef MULTIFINDER
					if (Ticks-Last_Check_Event >= MF_DELAY) 
					  if (Check_Events(0)) 
					    if (Main_State == Abort_Program)  {
					  		SIOClose(TRUE);
					    	exit(-1);
					    }
#endif
					if (async_pending < BUFFERS) {
						spin = FALSE;
					}
				}
			}
		} while (spin);
		
		pbp = &asyncout[async_head].pb;
		
		if (asyncout[async_head].bufSize < count) {
			if (asyncout[async_head].bufSize > 0) {
				DisposPtr(asyncout[async_head].buf);
				asyncout[async_head].buf = NULL;
				asyncout[async_head].bufSize = 0;
			}
			asyncout[async_head].buf = NewPtr(count);
			if (asyncout[async_head].buf == NULL) {
				goto writeSync;
			}
			asyncout[async_head].bufSize = count;
		}
				
		memcpy(asyncout[async_head].buf, c, (long) count);
		
		pbp->ioParam.ioCompletion = NULL;
		pbp->ioParam.ioRefNum = refout;
		pbp->ioParam.ioBuffer = asyncout[async_head].buf;
		pbp->ioParam.ioReqCount = count;
		pbp->ioParam.ioPosMode = 0;
		
		PBWrite(pbp, TRUE);

		async_head = (async_head + 1) % BUFFERS;
		async_pending ++;
	} else {
#endif
writeSync:
		pb.ioParam.ioCompletion = NULL;
		pb.ioParam.ioRefNum = refout;
		pb.ioParam.ioBuffer = c;
		pb.ioParam.ioReqCount = count;
		pb.ioParam.ioPosMode = 0;
	  
		PBWrite(&pb, FALSE );
#ifdef DOASYNC
	}
#endif
	
#ifdef MULTIFINDER
		if (Ticks-Last_Check_Event >= MF_DELAY) 
		  if (Check_Events(0)) 
		    if (Main_State == Abort_Program)  {
		  		SIOClose(TRUE);
		    	exit(-1);
		    }
#endif 

	return( (short) pb.ioParam.ioActCount );
}

SIOIdle()
{
}

int SIOEvent(EventRecord *anEvent)
{
	return 0;
}

