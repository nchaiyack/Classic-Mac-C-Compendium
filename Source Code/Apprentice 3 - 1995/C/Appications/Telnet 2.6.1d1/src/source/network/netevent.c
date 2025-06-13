/*
*    netevent.c
*	 Originally by Gaige B. Paulsen
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*    Network event handler for NCSA Telnet for the Macintosh
*
*	Called by:
*		event.c
*		maclook.c
*
*	Revisions:
*	7/92	Telnet 2.6:  added the 2 global structures, and cleaned up defines -- Scott Bulmahn
*/

#ifdef MPW
#pragma segment Network
#endif

#include <stdio.h>
#include <string.h>

#include "TelnetHeader.h"

#include "netevent.proto.h"
#include "InternalEvents.h"
#include "wind.h"
#include "mydnr.proto.h"
#include "bkgr.proto.h"
#include "maclook.proto.h"
#include "network.proto.h"
#include "menuseg.proto.h"
#include "rsmac.proto.h"
#include "vrrgmac.proto.h"
#include "tekrgmac.proto.h"
#include "vsdata.h"
#include "vskeys.h"
#include "translate.proto.h"
#include "debug.h"
#include "parse.proto.h"
#include "ftpbin.proto.h"
#include "Connections.proto.h"
#include "event.proto.h"

#include "telneterrors.h"

extern WindRec
	*screens;					/* The screen array from Maclook */
extern char *tempspot;			/* 256 bytes for temp strings */
extern short scrn;
extern MenuHandle myMenus[];

short
	FileInTransit,
	blocksize,					/*	how large do we make the blocks when we read? */
	gQueueError = 0,			//	Set to 1 if the queue fills up.
	gQueueLength = 0,			//	Used to monitor queue length for debugging purposes.
	gQueueInUse = 0;			//		Ditto.
	
QHdr	gEventsQueue, gEventsFreeQueue;
		
unsigned char *readspace;		/* main buffer space */

static void RangeError(short i);
void	ftppasteText(short scrn);

#define PFTP 1
#define PRCP 2
#define PDATA 3

//	Our "give time" routine
void Stask( void)
{
}

//	Every time we open or close a network connection, we add or remove a few elements
//	to/from the queue to assure that there will always be some free elements laying
//	around.  This allows us to avoid Dequeue'ing at interrupt time, which causes 
//	nasty mutex problems since we walk the queue in netgetevent at non-interrupt time.
void	ChangeQueueLength(short delta)
{
	internal_event	*theEvent;
	
	if (delta > 0)
		while (delta != 0) {
			theEvent = (internal_event *) NewPtrClear(sizeof(internal_event));
			Enqueue((QElemPtr)theEvent, &gEventsFreeQueue);
			gQueueLength++;
			delta--;
			}
	else
		while (delta != 0) {
			theEvent = (internal_event *)gEventsFreeQueue.qHead;
			Dequeue((QElemPtr)theEvent, &gEventsFreeQueue);
			if (theEvent) DisposePtr((Ptr)theEvent);
			gQueueLength--;
			delta++;
			}
}
			
/***********************************************************************/
/*  netgetevent
*   Retrieves the next event (and clears it) which matches bits in
*   the given mask.  Returns the event number or -1 on no event present.
*   Also returns the exact class and the associated integer in reference
*   parameters.
*/

short netgetevent(
//	short	mask,
	short	*class,
	short	*data1,
	long	*data2
  )
{
	internal_event	*theEvent;
	short			event;
//#define	QUEUE_STATS
#ifdef	QUEUE_STATS
	char			qs[255];
	static	long	timer = 0;
	
	if (TickCount() - timer > 60*30) {
		sprintf(qs, "QueueLength: %d, InUse: %d", gQueueLength, gQueueInUse);
		timer = TickCount();
		putln(qs);
		}
#endif	QUEUE_STATS

	if (gQueueError)				// Yikes, we actually ran out of queue elements!
		DebugStr("\pOut of Queue Elements, please quit as quickly as possible!");
		
	// Spin until we hit the end of the queue or if we Dequeue an element that is not
	// already dequeued.  If we don't do this check on Dequeue, someone else could come
	// in and dequeue an element that we are about to dequeue.  Thus, WHAM! we both have
	// that queue element.
	while ((theEvent = (internal_event *)gEventsQueue.qHead) != NULL) {
		if (Dequeue((QElemPtr)theEvent, &gEventsQueue) == noErr)
			break;
		}
		
//	while ((theEvent != NULL) && !(theEvent->eclass & mask))
//		theEvent = (internal_event *)theEvent->qLink;
	
	if (theEvent == NULL) return(-1);
		
//	(void) Dequeue((QElemPtr)theEvent, &gEventsQueue);

	*class = theEvent->eclass;
	*data1 = theEvent->data1;
	*data2 = theEvent->data2;
	event = theEvent->event;
	
	Enqueue((QElemPtr)theEvent, &gEventsFreeQueue);
	gQueueInUse--;
	return(event);
}

/***********************************************************************/
/*  netputevent
*   add an event to the queue.
*   Will probably get the memory for the entry from the free list.
*   Returns 0 if there was room, 1 if an event was lost.
*/
short netputevent
  (
	short	class,
	short	what,
	short	data1,
	long	data2
  )
{
	internal_event	*theEvent;
	
	while ((theEvent = (internal_event *)gEventsFreeQueue.qHead) != NULL) {
		if (Dequeue((QElemPtr)theEvent, &gEventsFreeQueue) == noErr)
			break;
		}
			
	if (theEvent == NULL) {
		gQueueError = 1;					// Darn, we filled the queue, alert the user.
		return(-1);
		}
	else
		gQueueInUse++;
		
	theEvent->qType = 0;	
	theEvent->eclass = class;
	theEvent->event = what;
	theEvent->data1 = data1;
	theEvent->data2 = data2;
	
	Enqueue((QElemPtr)theEvent, &gEventsQueue);
	return(0);
}

/***************************************************************************/
/*  netputuev
*   put a unique event into the queue
*   First searches the queue for like events
*/
short netputuev
  (
	short	class,
	short	what,
	short	data1,
	long	data2
  )
{
	internal_event	*theEvent = (internal_event *)gEventsQueue.qHead;
	
	while((theEvent != NULL) && ((theEvent->eclass != class) || (theEvent->event != what)
			|| (theEvent->data1 != data1) || (theEvent->data2 != data2)))
				theEvent = (internal_event *)theEvent->qLink;
				
	if (theEvent != NULL) return(0);
	
	return(netputevent(class, what, data1, data2));
}

void RangeError(short i)
{
	char temp[20];

	sprintf(temp,"%d in %d", -i,TelInfo->numwindows);
	putln(temp);
}

short	WindByPort(short port)
{
	short i=0;

	while (i<TelInfo->numwindows &&
			(screens[i].port != port || 
				((screens[i].active != CNXN_ACTIVE) && (screens[i].active != CNXN_OPENING)))
			) i++;

	if (i>=TelInfo->numwindows) {
		i = 0;
		while ((i<TelInfo->numwindows) &&
				((screens[i].ftpport != port) ||
					((screens[i].active != CNXN_ACTIVE) && (screens[i].active != CNXN_OPENING)))
				) i++;
		if (i>=TelInfo->numwindows) {					/* BYU */
			putln("Can't find a window for the port # in WindByPort");	/* BYU */
			RangeError(i);			/* BYU */
			if (i==0) i=999;		/* BYU */
			return(-i);				/* BYU */
			}						/* BYU */
		}							/* BYU */

	return(i);
}

void FlushNetwork(short scrn)
{
	short throwsize,cnt=512;
	
	if (blocksize < 512)
		throwsize = 512;
	else
		throwsize = blocksize;

	RSskip(screens[scrn].vs, 1);				/* Don't do any output */

	while (cnt>0) {
		cnt = netread(screens[scrn].port,readspace,throwsize);
		parse( &screens[scrn ], readspace, cnt);
		}

	RSskip(screens[scrn].vs, 0);				/* Do output now */
	SetPort( screens[scrn].wind);
	InvalRect(&screens[scrn].wind->portRect);	/* Don't forget to redraw */
}

void	ftppasteText(short scrn)		/* BYU */
{						/* BYU */
	char *ascii;		/* BYU */
						/* BYU */
	ascii = screens[scrn].outptr;					/* BYU */
	while ((screens[scrn].outlen > 0) && 			/* BYU */
		(*ascii>31) && (*ascii <127)) {				/* BYU */
		parse( &screens[ scrn],(unsigned char *) ascii, 1);		/* BYU LSC */
		screens[scrn].kbbuf[ screens[scrn].kblen++ ] = *ascii;	/* BYU */
		screens[scrn].outlen--;						/* BYU */
		ascii++;									/* BYU */
		}											/* BYU */
	screens[scrn].outptr = ascii;					/* BYU */
	if (*ascii == '\015') {							/* BYU */
		parse( &screens[scrn],(unsigned char *) "\015\012",2);	/* BYU LSC */
		screens[scrn].kbbuf[ screens[scrn].kblen++ ] = 0;		/* BYU */
		ftppi(screens[scrn].kbbuf);								/* BYU - ftp client */
		screens[scrn].kblen=0;									/* BYU */
		}									/* BYU */
	while ((screens[scrn].outlen > 0) && 	/* BYU */
		((*ascii<=31) || (*ascii >=127))) {	/* BYU */
		screens[scrn].outlen--;				/* BYU */
		screens[scrn].outptr++;				/* BYU */
		}				/* BYU */
}						/* BYU */

void pasteText(short scrn)
{
	short amount;
	if (!screens[scrn].outlen)
		return;


	if (netpush(screens[scrn].port) != 0) {			/* BYU 2.4.16 - wait until not busy */
		netputevent( USERCLASS, PASTELEFT, scrn,0);	/* BYU 2.4.16 */
		return;										/* BYU 2.4.16 */
	}												/* BYU 2.4.16 */
	if (screens[scrn].incount) {					/* BYU 2.4.16 */
		screens[scrn].incount = 0;					/* BYU 2.4.16 */
		screens[scrn].outcount = 0;					/* BYU 2.4.16 */
		netputevent( USERCLASS, PASTELEFT, scrn,0);	/* BYU 2.4.16 */
		return;										/* BYU 2.4.16 */
	}												/* BYU 2.4.16 */
	if (screens[scrn].outcount < 2) {				/* BYU 2.4.16 */
		screens[scrn].outcount++;					/* BYU 2.4.16 */
		netputevent( USERCLASS, PASTELEFT, scrn,0);	/* BYU 2.4.16 */
		return;										/* BYU 2.4.16 */
	}
	if (netqlen(screens[scrn].port) > 0) {			/* BYU 2.4.16 - wait until not full */
		netputevent( USERCLASS, PASTELEFT, scrn,0);	/* BYU 2.4.16 */
		return;										/* BYU 2.4.16 */
	}												/* BYU 2.4.16 */

	if (screens[scrn].ftpstate != 0) 				/* BYU */
		ftppasteText( scrn);						/* BYU */
	else {											/* BYU */
		if (!screens[scrn].pastemethod) {	// Do this all at once?
			amount = netwrite(screens[scrn].port, screens[scrn].outptr,
								screens[scrn].outlen);
			}
		else {		// Nope, do it in blocks
			if (screens[scrn].pastesize <= screens[scrn].outlen)
				amount = screens[scrn].pastesize;
			else
				amount = screens[scrn].outlen;
			amount = netwrite(screens[scrn].port, screens[scrn].outptr, amount);
			}
			
		if (screens[scrn].echo)										/* BYU */
			parse( &screens[scrn],(unsigned char *) screens[scrn].outptr,amount);	/* BYU LSC */
		screens[scrn].outlen -= amount;								/* BYU */
		screens[scrn].outptr += (long) amount;						/* BYU */
/*		screens[scrn].outcount += amount;			/* BYU LSC */
		}															/* BYU */

	if ( screens[scrn].outlen <=0) {
		screens[scrn].clientflags &= ~PASTE_IN_PROGRESS;	/* BYU LSC */
		HUnlock(screens[scrn].outhand);
		DisposHandle(screens[scrn].outhand);
		screens[scrn].outptr = (char *) 0L;			/* BYU LSC */
		screens[scrn].outhand = (char **) 0L;		/* BYU LSC */
		}
	else
		netputevent( USERCLASS, PASTELEFT, scrn,0);
}

void DoNetEvents(void)
{
	short		i, cnt;
	long	ftptime = 0;
	short	event, class, data1;
	long	data2, pos;
	
	if ((event = netgetevent(&class, &data1, &data2)) < 0) return;

	if ( (TickCount() - ftptime > 60*2) && FileInTransit) {
		ftptime = TickCount();
		Sftpstat(&pos);					/* get transfer status */
		if (pos <= 0) 
			ftpmess("FTP Status: transferring\015\012");
		else 
			{
			if (FileInTransit+2)
				sprintf((char *) tempspot,"FTP Status: %ld bytes remaining.\015\012", pos);		/* BYU LSC */
			else
				sprintf((char *) tempspot,"FTP Status: %ld bytes transferred.\015\012", pos);	/* BYU LSC */

			ftpmess((char *) tempspot);	/* BYU LSC */
			}
		}
		
	switch(class) {
		case SCLASS:
			switch (event) {
				case FTPACT:
					ftpd(0, data1);
					break;
				case CLOSEDONE:					/* Used in the drivers */
					netclose(data1);
					break;
				case CLOSEDONE+1:				/* Used in the drivers */
					netclose(data1);
					break;
				default:
					break;
			}
			break;
			
		case CONCLASS:							/* Connection type event */
			switch(GetPortType(data1)) {
				case PFTP:
					rftpd(event,data1);		/* BYU 2.4.16 */
					break;
				case PDATA:
					ftpd(event,data1);
					break;
				case UDATA:						/* BYU */
					userftpd(event,data1);		/* BYU */
					break;						/* BYU */
				default:
				case CNXN_TYPE:
					switch (event) {
						case CONOPEN:				/* connection opened or closed */
							i=WindByPort(data1);
							if (i<0) { 
								RangeError(i); 
								return;
								}
								 
		/* BYU mod - This tests TRUE if it is the telnet port or the main ftp port */
							if (data1 == screens[i].port) {		/* BYU - is this Telnet? */
								screens[ i].active= CNXN_ACTIVE;
								RSshow( screens[i].vs);			/* BYU */
								SelectWindow(screens[i].wind);	/* BYU */
								/*
								 * Send Kerberos initial options.
								 */
								send_auth_opt(&screens[i]);
		#if 0													/* BYU 2.4.20 */
								if (screens[i].ftpstate != 0) 	/* BYU 2.4.20 - Only if not FTP */
									userftpd(CONOPEN,data1);		/* BYU */
		#else													/* BYU 2.4.20 */
								if (screens[i].ftpstate == 0) {	/* BYU - Only if not FTP */
									netpush(screens[i].port);	/* BYU */
									netwrite(screens[i].port,"\377\375\001\377\375\003\377\374\43",9); /* BYU - Default telnet parms */
									// IAC DOTEL ECHO IAC DOTEL SGA IAC WONTTEL XDISPLOC
								} else {						/* BYU */
									userftpd(CONOPEN,data1);		/* BYU */
								}								/* BYU */
		#endif													/* BYU 2.4.20 */
								screens[i].Usga=1;		/* BYU */
								screens[i].echo = 1;
								changeport(scrn,i);		/* BYU */
								SetMenuMarkToLiveForAGivenScreen(scrn);			/* BYU */
								DoTheMenuChecks();		/* BYU */
							} else {					/* BYU - not Telnet so must be FTP */
							  userftpd(CONOPEN,data1);	/* BYU */
							}							/* BYU */
							break;
		 
						case CONDATA:				/* data arrived for me */
							i=WindByPort(data1);									/* BYU */
							if (i<0) { RangeError(i); return; }					/* BYU */
							if (TelInfo->ScrlLock || !screens[i].enabled)	/* BYU LSC */
								netputuev( CONCLASS, CONDATA, data1,0);
							else {
								if (screens[i].ftpstate == 0) 
									{					/* normal session connection */	
									cnt = netread(data1,readspace,blocksize);	/* BYU LSC */
									parse( &screens[i], readspace, cnt);	/* BYU LSC */
									screens[i].incount += cnt;				/* BYU LSC */
									}							/* BYU */
								else {							/* BYU */
									userftpd(CONDATA,data1);		/* BYU */
									}							/* BYU */
								}
							break;
		
						case CONFAIL:
							{
								short	i;
								Str255	scratchPstring;
								
								netclose( data1);
								i= WindByPort(data1);
								if (i<0) { RangeError(i); return; }
								BlockMove((Ptr)screens[i].machine, (Ptr)scratchPstring, Length(screens[i].machine)+1);
								PtoCstr(scratchPstring);
								DoError(807 | NET_ERRORCLASS, LEVEL2, (char *)scratchPstring);
								
								if (screens[i].active != CNXN_ACTIVE) destroyport(i);	// JMB - 2.6
								else removeport( i);		// JMB - 2.6
							}
							break;
		
						case CONCLOSE:
							{
								short i;
		
								i= WindByPort(data1);
								if (i<0) { 
									netclose( data1);			/* We close again.... */
									RangeError(i);
									return;
									}
		/* BYU - This tests TRUE if it is the telnet port or the main ftp port */
								if (data1 == screens[i].port) {		/* BYU */
									putln("Closing....");			/* BYU */
									FlushNetwork(i);				/* BYU */
									netclose( screens[i].port);		/* BYU */
									removeport(i);					/* BYU */
									}								/* BYU */
								else {								/* BYU */
									userftpd(CONCLOSE,data1);			/* BYU */
									}								/* BYU */
							}
							break;
		
						default:
							break;
						}
					break;	/* Case port type = CNXN_TYPE */
//				default:
//					putln("Received CONCLASS event for port w/o valid type.");
//					break;
				}
			break;	/* CONCLASS */

	case USERCLASS:
			switch (event) {
				case DOMAIN_DONE:	// data1 = screen #, data2 = ptr to info structure
					HandleDomainDoneMessage(data1, data2);
					break;
				case FTPBEGIN:
					FileInTransit=data1;
					ftptime=TickCount();
					break;
				case FTPEND:
					ftpmess("FTP Transfer Concluding\015\012");
					ftpmess("\015\012");
					FileInTransit=0;
					break;
				case FTPCOPEN:
					{
						ip_addrbytes	ftpinfo;
						
						Str255	remoteMachineName;
						
						if (gFTPServerPrefs->ResetMacBinary) 
							TelInfo->MacBinary = gFTPServerPrefs->UseMacBinaryII;
						updateMenuChecks();
						TelInfo->xferon=1;
						updateCursor(1);
						ftpmess("-----------------------------------------------------------------------------\015\012");
						ftpmess("FTP server initiated from host: ");
						ftpinfo.a.addr = Sftphost();
						
						if (!TranslateIPtoDNSname(ftpinfo.a.addr, remoteMachineName)) 
							sprintf((char *) &tempspot[4],"%u.%u.%u.%u\015\012",
									(unsigned char)ftpinfo.a.byte[0],
									(unsigned char)ftpinfo.a.byte[1],
									(unsigned char)ftpinfo.a.byte[2],
									(unsigned char)ftpinfo.a.byte[3]);
						else {
							PtoCstr(remoteMachineName);
							sprintf((char *) &tempspot[4],"%s\015\012",remoteMachineName);
							}
						ftpmess((char *) &tempspot[4]);		/* BYU LSC */
					}
					break;
				case FTPCLOSE:
					TelInfo->xferon=0;
					updateCursor(1);
					if (gFTPServerPrefs->ResetMacBinary) 
						TelInfo->MacBinary = gFTPServerPrefs->UseMacBinaryII;
					updateMenuChecks();
					ftpmess("FTP server ending session\015\012");
					ftpmess("-----------------------------------------------------------------------------\015\012");
					ftpmess("\015\012");

					break;
				case RG_REDRAW:			/* RGredraw event */
					if (VGalive(data1) && RGsupdate(data1))
						TekEnable(data1);
					break;
				case PASTELEFT:
					pasteText(data1);
					break;
				default:
					break;
			}
			break;	/* USERCLASS */

	default:
		break;
	} /* switch (CLASS) */
	
} /* DoNetEvents */
		
/*	setblocksize()
		Make sure that we have space for the block of data that is to be read each
		time the netread() is called. */
short setblocksize(short tosize)
{
	blocksize = tosize;							/* keep size of block */
	
	if (tosize < 512)							/* minimum buffer */
		tosize = 512;

	if (readspace)
		DisposPtr((Ptr)readspace);						/* free old block */
		
	if (NULL == (readspace = (unsigned char *) NewPtrClear(tosize + 100)))	/* BYU LSC */
		return(-1);

	return(0);
}





