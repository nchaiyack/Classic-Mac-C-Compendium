/*
   For best results in visual layout while viewing this file, set
   tab stops to every 4 columns.
*/

/*
   dcpgpkt.c

   Revised edition of dcp

   Stuart Lynne May/87

   Copyright (c) Richard H. Lamb 1985, 1986, 1987
   Changes Copyright (c) Stuart Lynne 1987

			Portions Copyright � David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

   Maintenance notes:

   25Aug87 - Allow for up to 7 windows - Jal
   01Nov87 - those strncpy's should really be memcpy's! - Jal
   11Sep89 - Raise TimeOut to 15 - ahd
   30Apr90 - Add Jordon Brown's fix for short packet retries.
             Reduce retry limit to 20                             ahd
   22Jul90 - Change error retry limit from per host to per
             packet.                                              ahd
   22Jul90 - Add error message for number of retries exceeded     ahd
   08Sep90 - Drop memmove to memcpy change supplied by Jordan
             Brown, MS 6.0 and Turbo C++ agree memmove insures
             no overlap
*/

/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */

/* 7-window "g" ptotocol */

/*
   Thanks goes to John Gilmore for sending me a copy of Greg Chesson's
   UUCP protocol description -- Obviously invaluable.
   Thanks also go to Andrew Tannenbaum for the section on Siding window
   protocols with a program example in his "Computer Networks" book.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dcp.h"
#include "sio.h"

#define PKTSIZE   MAXPACK

#define HDRSIZE   6
#define MAXTRY 4

/* g-packet type definitions */

#define DATA   0
#define CLOSE  1
#define NAK    2
#define SRJ    3
#define ACK    4
#define  INITC 5
#define INITB  6
#define INITA  7

#define POK    -1

#define MAXWINDOW 7
#define DFLWINDOW 3
#define NBUF   8  /* always SAME as MAXSEQ ? */
#define MAXSEQ 8

#define between(a,b,c) ((a<=b && b<c) || (c<a && a<=b) || (b<c && c<a))

#define nextpkt(x)    ((x + 1) % MAXSEQ)

#include "dcpgpkt.proto.h"
static int gmachine(long int timeout);
static void gspack(int type, int yyy, int xxx, int len, int xmit, char *data);
static int grpack(int *yyy, int *xxx, int *len, char *data, int timeoutX);
static unsigned int checksum(char *data, int len);

/* packet definitions */

static int rwl, swl, swu, rwu, nbuffers, irec, timeout;
static int nerr;
static int GOT_SYNC, GOT_HDR, skipped_to_sync;
static int fseq[NBUF], outlen[NBUF], inlen[NBUF], arrived[NBUF], xmitlen[NBUF];
static int nwindows;
static char *outbuf[NBUF];
static char *inbuf[NBUF];
static long ftimer[NBUF], acktmr, naktmr;
static long nakholdoff;
static char rescan[PKTSIZE+HDRSIZE+10];
static int rescanct, rescanoffset;
static int shifts;
static boolean smart_packets;

static int  gmachine(long int timeout);
static void gspack(int  type,int  yyy,int  xxx,int  len,int xmit,char  *data);
static int  grpack(int  *yyy,
                   int  *xxx,
                   int  *len,
                   char *data,
                   const int timeout);

static unsigned int checksum(char *data, int len);

/****************** SUB SUB SUB PACKET HANDLER ************/

/*
   g o p e n p k
*/

int gopenpk(int master)
{
   int i, xxx, yyy, len, force_big;
   int maxoffer;
   char tmp[PKTSIZE+1];
   int sync_timeout;

   pktsize = DFLPACK;   /* change it during parse & INIT */
   msgtime = MSGTIME;   /* not sure I need this for "g" proto */

   /* initialize proto parameters */
   swl = nerr = nbuffers = 0;
   if (!sync_timeout)
      sync_timeout = PacketTimeout + 1;
   swl = swu = 1;
   rwl = 0;
   rescanct = 0;
   rescanoffset = 0;
   nwindows = DFLWINDOW;
   smart_packets = FALSE;
   force_big = FALSE;
   (*currentConnection->SetParity)(data8, noParity);
   if (strlen(proto) > 1) {
	i = proto[1] - '0';
	if (i > 0 && i <= MAXWINDOW) {
		nwindows = i;
	}
	if (proto[2] == '/' || proto[2] == ':' || proto[2] == '!') {
		sscanf(proto+3, "%d", &i);
		xxx = 0;
		yyy = 32;
		while (yyy <= MAXPACK && yyy != i) {
			xxx ++;
			yyy *= 2;
		}
		if (yyy <= MAXPACK && yyy == i) {
			pktsize = i;
		}
		if (proto[2] == ':') {
			smart_packets = TRUE;
		} else if (proto[2] == '!') {
			smart_packets = TRUE;
			force_big = TRUE;
		}
	}
   }
   maxoffer = pktsize;
   rwu = nwindows - 1; /* dcp */
   for (i = 0; i < NBUF; i++) {
      ftimer[i] = 0;
      arrived[i] = FALSE;
   }
   GOT_SYNC = GOT_HDR = FALSE;
   skipped_to_sync = 0;
   printmsg(2, "Offer: window %d, packet size %d", nwindows, maxoffer);

   /* 3-way handshake */
   timeout = 2; /* want some timeout capability here */
   gspack(INITA, 0, 0, 0, 0, NULL);
rsrt:
   if (nerr >= MaxErr)
   {
      printmsg(0,
         "gopenpk: Consecutive error limit of %d exceeded, %ld total errors",
          MaxErr, nerr + remote_stats.errors);
      return(FAILED);
   }

   /* INIT sequence. */

   switch (grpack(&yyy, &xxx, &len, NULL, PacketTimeout )) {
   case INITA:
      printmsg(5, "**got INITA");
      if (yyy < nwindows) {
      	nwindows = yyy;
      }
#ifdef NOTDEF
      nwindows = yyy;
      if (nwindows > MAXWINDOW)
         nwindows = MAXWINDOW;
#endif
      rwu = nwindows - 1;
   	  printmsg(2, "Peer says: max window %d", yyy);
      gspack(INITB, 0, 0, 0, 0, NULL);  /* data segment (packet) size */
      goto rsrt;
   case INITB:
      printmsg(5, "**got INITB");
      i = 8 * (2 << (yyy+1));
   	  printmsg(2, "Peer says: packet size %d", i);
      if (i < pktsize && ! force_big)
         pktsize = i;
      gspack(INITC, 0, 0, 0, 0, NULL);
      goto rsrt;
   case INITC:
      printmsg(5, "**got INITC");
      if (yyy < nwindows)
      {
   	  	 printmsg(2, "Peer says: max window %d", yyy);
         nwindows = yyy;
         rwu = nwindows - 1;
      }
      break;
   default:
      nerr++;
      printmsg(5, "**got SCREW UP");
      gspack(INITA, 0, 0, 0, 0, NULL);
      goto rsrt;
   }

/*--------------------------------------------------------------------*/
/*                    Allocate the needed buffers                     */
/*--------------------------------------------------------------------*/

   i = 0;
   while( i < NBUF)
   {

      inbuf[i] = NewPtr( maxoffer );
      outbuf[i] = NewPtr( maxoffer );
      if (inbuf[i] == NULL || outbuf[i] == NULL) {
      	printmsg(0, "*** Could not allocate packet buffers!");
      	return(FAILED);
      }

      i ++;

   } /* while */
   
   nerr = 0;
   nakholdoff = 0;
   printmsg(1, "Channel open, %c%d/%d, smart packets %sabled", proto[0], nwindows, pktsize,
            smart_packets ? "en" : "dis");
   return(OK); /* channel open */

} /*gopenpk*/


/*
   g c l o s e p k
*/

int gclosepk(void)
{
   int i;
   char tmp[PKTSIZE+1];

   timeout = 1;
   for (i = 0; i < MAXTRY; i++) {
      gspack(CLOSE, 0, 0, 0, 0, NULL);
      if (gmachine(PacketTimeout) == CLOSE)
         break;
   }

   printmsg(0, "%ld packets transferred, %ld errors.", remote_stats.packets,
         nerr + remote_stats.errors);

/*--------------------------------------------------------------------*/
/*                        Release our buffers                         */
/*--------------------------------------------------------------------*/

   i = 0;

   while( i < NBUF )
   {
      DisposPtr(inbuf[i] );
      DisposPtr( outbuf[i] );
      inbuf[i] = outbuf[i] = NULL;
      i++;
   } /* while( i < NBUF ) */

   return(0);

} /*gclosepk*/


/*
   g g e t p k t

   Gets no more than a packet's worth of data from
   the "packet I/O state machine".  May have to
   periodically run the packet machine to get some packets.

   on input:   don't care
   on return:  data+\0 and length in len.

   ret(0)   if all's well
   ret(-1) if problems (failed)
*/

int ggetpkt(char *data, int *len)
{
   int   i;
   int   retry = MaxErr;
   time_t start;

   irec = 1;

/*--------------------------------------------------------------------*/
/*                Loop to wait for the desired packet                 */
/*--------------------------------------------------------------------*/

   time( &start );
   while (!arrived[rwl] && retry)
   {
      if (gmachine(PacketTimeout) != POK)
         return(-1);

      if (!arrived[rwl] )
      {
         time_t now;
         if (time( &now ) > (start + PacketTimeout) )
         {
            printmsg(5,"ggetpkt: Timeout %d waiting for inbound packet %d",
                     MaxErr - --retry, remote_stats.packets + 1);
/*            timeouts++; */
            start = now;
         } /* if (time( now ) > (start + E_PacketTimeout) ) */
      } /* if (!arrived[rwl] ) */
   } /* while (!arrived[rwl] && i) */

   if (!arrived[rwl])
   {
      printmsg(0,"ggetpkt: Remote host failed to respond after %ld seconds",
               (long) PacketTimeout * MaxErr);
/* removed per Bo Holst-Christensen
     gclosepk();
*/
      return -1;
   }

/*--------------------------------------------------------------------*/
/*                           Got a packet!                            */
/*--------------------------------------------------------------------*/

   i = rwl; /*<-- mod(,rwindow) for larger than 8 seq no.s */
   *len = inlen[i];
   memcpy(data, inbuf[i], *len);
   nerr = 0; /* reset error counter */

   arrived[i] = FALSE;
   rwu = nextpkt(rwu)        ;  /* bump receive window */

   remote_stats.packets++;
   remote_stats.breceived += *len;

   return(0);

} /*ggetpkt*/




/*
   g s e n d p k t

   Put at most a packet's worth of data in the packet state
   machine for transmission.
   May have to run the packet machine a few times to get
   an available output slot.

   on input: data=*data; len=length of data in data.

   return:
    0 if all's well
   -1 if problems (failed)
*/

int gsendpkt(char *data, int len)
{
   int i1;
   int delta;
#ifdef _DEBUG
   int savedebug = debuglevel;
#endif

   irec = 0;
   /* WAIT FOR INPUT i.e. if weve sent SWINDOW pkts and none have been
      acked, wait for acks */
   while (nbuffers >= nwindows)
      if (gmachine(0) != POK)    /* Spin with no timeout             */
         return(-1);

   i1 = swu;   /* <--If we ever have more than 8 seq no.s, must mod() here */

/*--------------------------------------------------------------------*/
/*               Place packet in table and mark unacked               */
/*--------------------------------------------------------------------*/

   memcpy(outbuf[i1], data, len);

/*--------------------------------------------------------------------*/
/*                       Handle short packets.                        */
/*--------------------------------------------------------------------*/

   xmitlen[i1] = pktsize;
   if (smart_packets)
      while ( ((len * 2) < xmitlen[i1]) && (xmitlen[i1] > 32) )
         xmitlen[i1] /= 2;

   delta = xmitlen[i1] - len;
   if (delta > 127)
   {
      memmove(outbuf[i1] + 2, outbuf[i1], len);
      memset(outbuf[i1]+len+2, 0, delta - 2);
                              /* Pad with nulls.  Ugh.               */
      outbuf[i1][0] = (unsigned char) ((delta & 0x7f) | 0x80);
      outbuf[i1][1] = (unsigned char) (delta >> 7);
   } /* if (delta > 127) */
   else if (delta > 0 )
   {
      memmove(outbuf[i1] + 1, outbuf[i1], len);
      outbuf[i1][0] = (unsigned char) delta;
      memset(outbuf[i1]+len+1, 0, delta - 1);
                              /* Pad with nulls.  Ugh.               */
   } /* else if (delta > 0 )  */

/*--------------------------------------------------------------------*/
/*                            Mark packet                             */
/*--------------------------------------------------------------------*/

   outlen[i1] = len;
   ftimer[i1] = time((long int) NULL);
   swu = nextpkt(swu);  /* bump send window */
   nbuffers++;

   remote_stats.packets++;
   remote_stats.bsent += len;

/*--------------------------------------------------------------------*/
/*                              send it                               */
/*--------------------------------------------------------------------*/

   gspack(DATA, rwl, i1, outlen[i1], xmitlen[i1], outbuf[i1]);

#ifdef _DEBUG
   debuglevel = savedebug;
#endif

   return(0);

} /*gsendpkt*/


int gfilepkt(void)
{
	return 0; /* begin data phase */
}

int geofpkt(void)
{
	int len;
	char spacket[MAXPACK];
	if(gsendpkt(spacket, 0))
		return('N');
	return 'Y';
}

int gsendresp(int state)
{
	return OK;
}

/**********  Packet Machine  ********** RH Lamb 3/87 */

/*
   g m a c h i n e

   Ideally we would like to fork this process off in an infinite loop and
   send and receive packets through "inbuf" and "outbuf". Can't do this in
   MS-DOS so we setup "getpkt" and "sendpkt" to call this routine often and
   return only when the input buffer is empty thus "blocking" the packet-
   machine task.
*/

static int gmachine(long int timeout)
{
   boolean done   = FALSE;    /* True = drop out of machine loop  */
   boolean close  = FALSE;    /* True = terminate connection upon
                                        exit                      */
   char rdata[PKTSIZE+1];

   while ( !done )
   {
      boolean resend = FALSE;    /* True = resend data packets       */
      boolean donak  = FALSE;    /* True = NAK the other system      */

      int packet, rack, rseq, rlen, i1;
      time_t expired;

      printmsg(10, "* send %d < W < %d, receive %d < W < %d, error %d",
         swl, swu, rwl, rwu, nerr);

/*--------------------------------------------------------------------*/
/*    Waiting for ACKs for swl to swu-1.  Next pkt to send=swu        */
/*    rwl=expected pkt                                                */
/*--------------------------------------------------------------------*/

      printmsg(7, "Bytes transfered %ld errors %d",
       (long) (remote_stats.packets * PKTSIZE) , nerr);

/*--------------------------------------------------------------------*/
/*             Attempt to retrieve a packet and handle it             */
/*--------------------------------------------------------------------*/

      packet = grpack(&rack, &rseq, &rlen, rdata, timeout);
      switch (packet) {

         case CLOSE:
            printmsg(5, "**got CLOSE");
            close = done = TRUE;
            break;

         case EMPTY:
            printmsg(6, "**got EMPTY");
            expired = time(NULL) - PacketTimeout;
            for (rack = swl; between(swl, rack, swu); rack = nextpkt(rack))
            {
               printmsg(6, "---> seq, elapst %d %ld", rack,
                    ftimer[rack] - expired);
               if (ftimer[rack] && (ftimer[rack] <= expired))
               {
                   printmsg(5, "*** timeout %d", rack);
                         /* Since "g" is "go-back-N", when we time out we
                          must send the last N pkts in order.  The generalized
                          sliding window scheme relaxes this reqirment. */
                   nerr++;
                   resend = TRUE;
                   break;
               } /* if */
            } /* for */
            done = TRUE;
            break;

         case NAK:
         case ACK:
            if (packet == NAK)
            {
               nerr++;
               printmsg(5, "**got NAK %d", rack);
               resend = TRUE;
            }
            else
               printmsg(5, "**got ACK %d", rack);

            while(between(swl, rack, swu))
            {                             /* S<-- -->(S+W-1)%8 */
               printmsg(5, "*** ACK %d", swl);
               ftimer[swl] = 0;
               nbuffers--;
               done = TRUE;            /* Get more data for input */
               swl = nextpkt(swl);
            } /* while */
            if (!done && (packet == ACK)) /* Find packet?         */
            {
               printmsg(2,"gmachine: Received ACK for bad packet %d, window is %d-%d)", rack, swl, swu);
               nerr++;                 /* Count an error and proceed */
            }
            break;

         case DATA:
            printmsg(5, "**got DATA %d %d", rack, rseq);
            i1 = nextpkt(rwl);   /* (R+1)%8 <-- -->(R+W)%8 */
            if (i1 == rseq) {
               nakholdoff--;
               arrived[i1] = TRUE;
               inlen[i1] = rlen;
               memcpy(inbuf[i1], rdata, rlen);
               rwl = i1;
               printmsg(5, "*** ACK d %d", rwl);
               gspack(ACK, rwl, 0, 0, pktsize, rdata);
               done = TRUE;   /* return to caller when finished */
                              /* in a mtask system, unneccesary */
	           while(between(swl, rack, swu))
	            {                             /* S<-- -->(S+W-1)%8 */
	               printmsg(5, "*** implicit ACK %d", swl);
	               ftimer[swl] = 0;
	               nbuffers--;
	               swl = nextpkt(swl);
	            } /* while */
            } else {
               nerr++;
               printmsg(5, "*** unexpect %d ne %d - %d", rseq, rwl, rwu);
               donak = TRUE;
            }
            break;

         case ERROR:
            printmsg(5, "*** got BAD CHK");
            donak = TRUE;
            nakholdoff = 0; /* always NAK these to prevent stalling */
            break;

         default:
            printmsg(5, "*** got SCREW UP");
            break;

      } /* switch */

/*--------------------------------------------------------------------*/
/*      If we received an NAK or timed out, resend data packets       */
/*--------------------------------------------------------------------*/

      if (resend)
      for (rack = swl; between(swl, rack, swu); rack = nextpkt(rack))
      {                          /* resend rack->(swu-1)             */
         gspack(DATA, rwl, rack, outlen[rack], pktsize, outbuf[rack]);
         printmsg(5, "*** resent %d", rack);
         ftimer[rack] = time((long) NULL);
      } /* for */

/*--------------------------------------------------------------------*/
/*  If we have an error and have not recently sent a NAK, do so now.  */
/*  We then reset our counter so we receive at least a window full of */
/*                 packets before sending another NAK                 */
/*--------------------------------------------------------------------*/

      if ( donak )
      {
         nerr++;
         if ( nakholdoff <= 0 )
         {
            printmsg(5, "*** NAK d %d", rwl);
            gspack(NAK, rwl, 0, 0, pktsize, rdata);
            nakholdoff = nwindows + 1;
         } /* if ( nakholdoff < 1 ) */
      } /* if ( donak ) */

/*--------------------------------------------------------------------*/
/*    If we have an excessive number of errors, drop out of the       */
/*    loop                                                            */
/*--------------------------------------------------------------------*/

      if (nerr >= MaxErr)
      {
         printmsg(0,
            "gmachine: Consecutive error limit of %d exceeded, %ld total errors",
            MaxErr, nerr + remote_stats.errors);
         remote_stats.errors += nerr;
         nerr = 0;
         done = close = TRUE;
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*    Return to caller, gracefully terminating packet machine if      */
/*    requested                                                       */
/*--------------------------------------------------------------------*/

   if ( close )
   {
      gspack(CLOSE, 0, 0, 0, pktsize, rdata);
      return CLOSE;
   }
   else
      return POK;

} /*gmachine*/


/*************** FRAMING *****************************/

/*
   g s p a c k

   Send a packet

   type=type yyy=pkrec xxx=timesent len=length<=xmit xmit=pktsize data=*data
   ret(0) always
*/

static void gspack(int type, int yyy, int xxx, int len, int xmit, char *data)
{
   unsigned int check, i;
   unsigned char header[HDRSIZE];

#ifdef   LINKTEST
   /***** Link Testing Mods *****/
   unsigned char  dpkerr[10];
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

#ifdef   LINKTEST
   /***** Link Testing Mods - create artificial errors *****/
   printf("**n:normal,e:error,l:lost,p:partial,h:bad header,s:new seq--> ");
   gets(dpkerr);
   if (dpkerr[0] == 's')
      sscanf(&dpkerr[1], "%d", &xxx);
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

   if ( debuglevel > 4 )
      printmsg(5, "send packet type %d, yyy=%d, xxx=%d, len=%d, buf = %d",
               type, yyy, xxx, len, xmit);

   header[0] = '\020';
   header[4] = (unsigned char) (type << 3);

   switch (type) {

      case CLOSE:
         break;   /* stop protocol */

      case NAK:
         header[4] += yyy;
         break;   /* reject */

      case SRJ:
         break;

      case ACK:
         header[4] += yyy;
         break;   /* ack */

      case INITA:
      case INITC:
         header[4] += nwindows;
         break;

      case INITB:
         i = 32;
         while( i < pktsize )
         {
            header[4] ++;
            i *= 2;
         }
         break;

      case DATA:
         header[4] = (unsigned char) (0x80 + (xxx << 3) + yyy);
         if (len < pktsize)   /* Short packet?                       */
         {
            header[4] |= 0x40;/* Count byte handled at higher level */
            printmsg(7, "data=|%.*s|", len, data);
         }
         else
            printmsg(7, "data=|%.*s|", len, data);
         break;

      default:
         printmsg(0,"gspack: Invalid packet type %i",type);
         return;
   } /* switch */

/*--------------------------------------------------------------------*/
/*    Now we finish up the header.  For data packets, determine       */
/*    the K number in header[1], which specifies the number of        */
/*    actual data bytes transmitted as a power of 2; we also          */
/*    compute a checksum on the data.                                 */
/*--------------------------------------------------------------------*/

   if (type == DATA)
   {
      header[1] = 1;
      i = 32;
      while( i < xmit )
      {
         header[1] ++;
         i *= 2;
      }

      if ( i != xmit )        /* Did it come out exact power of 2?   */
      {
         printmsg(0,"Packet length error ... %i != %i for K = %i",
               i, xmit, (int) header[1]);
         return;             /* No --> Well, we blew THAT math      */
      } /* if ( i != xmit ) */

/*--------------------------------------------------------------------*/
/*                        Compute the checksum                        */
/*--------------------------------------------------------------------*/

      check = checksum(data, xmit);
      i = header[4]; /* got to do this on PC for ex-or high bits */
      i &= 0xff;
      check = (check ^ i) & 0xffff;
      check = (0xaaaa - check) & 0xffff;
   }
   else {
      header[1] = 9;          /* Control packet size K number (9)    */
      check = (0xaaaa - header[4]) & 0xffff;
                              /* Simple checksum for control         */
   } /* else */

   header[2] = (unsigned char) (check & 0xff);
   header[3] = (unsigned char) ((check >> 8) & 0xff);
   header[5] = (unsigned char)
            ((header[1] ^ header[2] ^ header[3] ^ header[4]) & 0xff) ;

#ifdef   LINKTEST
   /***** More Link Testing Mods *****/
   switch(dpkerr[0]) {
   case 'e':
      data[10] = - data[10];
      break;
   case 'h':
      header[5] = - header[5];
      break;
   case 'l':
      return;
   case 'p':
      swrite((char *) header, HDRSIZE);
      if (header[1] != 9)
         swrite(data, xmit - 3);
      return;
   default:
      break;
   }
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

   swrite((char *) header, HDRSIZE);      /* header is 6-bytes long */
   if (header[1] != 9)
      swrite(data, xmit);

} /*gspack*/



/*
   g r p a c k

   Read packet

   on return: yyy=pkrec xxx=pksent len=length<=PKTSIZE  data=*data

   ret(type)   ok
   ret(EMPTY)  input buf empty, or bad header

   ret(EMPTY)  lost packet timeout
   ret(ERROR)  checksum error

   NOTE (specifications for sread()):

   sread(buf, n, timeout)
      while(TRUE) {
         if (# of chars available >= n) (without dec internal counter)
            read n chars into buf (decrement internal char counter)
            break
         else
            if (time > timeout)
               break
      }
      return(# of chars available)

*/

static int grpack(int *yyy, int *xxx, int *len, char *data, int timeoutX)
{
   unsigned int type, check, checkchk, i, total;
   boolean badHeader;
   unsigned char c, c2;
   int hdroffset, hdrbytes, scanct, scanoffset, dbytes, dgot, doffset;
   unsigned char grpkt[PKTSIZE+HDRSIZE];
   const int sync_timeout = PacketTimeout + 1;
                              /* Time willing to wait for sync char  */
   time_t start;
   time_t elapsed;

   if (GOT_HDR)
      goto get_data;

/*--------------------------------------------------------------------*/
/*   Spin up to timeout waiting for a Control-P, our sync character   */
/*--------------------------------------------------------------------*/

   start = time((time_t) NULL); /* Rememeber we started looking for
                                 sync character                      */
   elapsed = 0;               /* No time has passed since start      */
   
   if (rescanct > 0) {
      printmsg(1, "grpack: rescanning %d bytes", rescanct);
   }

   while (!GOT_SYNC && (elapsed < sync_timeout))
   {
      if (rescanct > 0) {
         c = rescan[rescanoffset++];
         rescanct--;
      } else if (sread( (char *) &c , 1, sync_timeout) < 1)
      {
         nerr++;                 /* Otherwise, we never quit!        */
         return EMPTY;
      }
      if ((c & 0x7f) == '\020') {
         GOT_SYNC = TRUE;
         if (skipped_to_sync > 0) {
         	printmsg(2, "Skipped %d bytes\n", skipped_to_sync);
        	 skipped_to_sync = 0;
         }
      } else if ((c & 0x7f) != '\0') { /* Don't kvetch about nulls - known BSD bug */
         time_t   now = time((time_t) NULL);  /* Get new time          */
         elapsed = now - start;  /* Compute time used since start    */
         if (skipped_to_sync == 0) {
         	printmsg(2, "grpack: %02x not a sync, skipping...", c);
         }
         skipped_to_sync ++;
      } /* else */
   } /* while */

   if ( !GOT_SYNC )
      return EMPTY;

/*--------------------------------------------------------------------*/
/*   We are in are in sync with the other host; now process the       */
/*   header which should follow.                                      */
/*--------------------------------------------------------------------*/
   hdroffset = 1;
   hdrbytes = HDRSIZE - 1;
   scanct = rescanct;
   scanoffset = rescanoffset;
   while (rescanct > 0 && hdrbytes > 0) {
      grpkt[hdroffset++] = rescan[rescanoffset++];
      rescanct--;
      hdrbytes--;
   }
   if (hdrbytes > 0 && sread( (char *) &grpkt[hdroffset], hdrbytes, PacketTimeout) < hdrbytes) {
   		printmsg(2, "grpack: timeout during packet header");
   		return(EMPTY);
   }
   GOT_SYNC = FALSE;
   i = (unsigned)grpkt[1] ^ (unsigned)grpkt[2] ^
       (unsigned)grpkt[3] ^ (unsigned)grpkt[4] ^
       (unsigned)grpkt[5];

   i &= 0xff;
   printmsg(10, "prpkt %02x %02x %02x %02x %02x .. %02x ",
      grpkt[1], grpkt[2], grpkt[3], grpkt[4], grpkt[5], i);


   if (i != 0) {
		printmsg(0, "*** header cksum error ***");
		badHeader = TRUE;
   } else if (grpkt[1] != 9 /*ctl*/ && grpkt[1] > MAXPACKCODE) {  /* bad header */
		printmsg(0, "*** bad header ***");
		badHeader = TRUE;
   } else {
		badHeader = FALSE;
   }
   
   if (badHeader) {
      if (scanct > 0) {
      	rescanct = scanct;
      	rescanoffset = scanoffset;
      } else {
        rescanct = HDRSIZE - 1;
        rescanoffset = 0;
      	memcpy(rescan, &grpkt[1], rescanct);
      }
      return(EMPTY); /* DECUS paper says not to NAK it */
   }

   GOT_HDR = TRUE;
   if (grpkt[1] == 9) { /* control packet */
      if ( data != NULL )
         *data = '\0';
      *len = 0;
      c = grpkt[4];
      type = c >> 3;
      *yyy = c & 0x07;
      *xxx = 0;
      check = 0;
      checkchk = 0;
      GOT_HDR = FALSE;
   } else { /* data packet, packet size already verified */
get_data:
      if ( data == NULL )
      {
         printmsg(0,"grpack: Unexpected data packet!");
         return(ERROR);
      }
/*--------------------------------------------------------------------*/
/*             Compute the size of the data block desired             */
/*--------------------------------------------------------------------*/

      total = 8 * (2 << grpkt[1]);
      if (total > MAXPACK)    /* Within the defined limits?          */
      {                       /* No --> Other system has bad header,
                                 or the header got corrupted         */
         printmsg(0,"grpack: Invalid packet size %d (%d)",
            total, (int) grpkt[1]);
         GOT_HDR = FALSE;
         return(ERROR);
      }
	  if (rescanct > 0) {
	     if (rescanct > total) {
     	    dbytes = total;
     	 } else {
     	    dbytes = rescanct;
     	 }
     	 memcpy(grpkt+HDRSIZE, rescan+rescanoffset, (size_t) dbytes);
     	 doffset = dbytes;
     	 rescanoffset += dbytes;
     	 rescanct -= dbytes;
     	 dbytes = total - dbytes;
      } else {
      	dbytes = total;
      	doffset = 0;
      }
      if (dbytes > 0) {
      	 dgot = sread((char *) grpkt+HDRSIZE+doffset, dbytes, PacketTimeout);
      	 if (dgot < dbytes) {
         	rescanoffset = 0;
         	rescanct = doffset + dgot;
         	if (rescanct < 1) {
         		printmsg(1, "Timeout while reading packet, no data");
         		rescanct = 0;
         	} else {
	         	printmsg(1, "Timeout while reading packet, saving %d bytes", rescanct);
	         	memcpy(rescan, grpkt+HDRSIZE, (size_t) rescanct);
	        }
         	return(EMPTY);
         }
      }
      GOT_HDR = FALSE;
      type = 0;
      c2 = grpkt[4];
      c = (unsigned char) (c2 & 0x3f);
      *xxx = c >> 3;
      *yyy = c & 0x07;
      i = grpkt[3];
      i = (i << 8) & 0xff00;
      check = grpkt[2];
      check = i | (check & 0xff);
      checkchk = checksum((char *) grpkt+HDRSIZE, total);
      i = grpkt[4] | 0x80;
      i &= 0xff;
      checkchk = 0xaaaa - (checkchk ^ i);
      checkchk &= 0xffff;
      if (checkchk != check) {
         printmsg(4, "*** packet checksum error ***");
         rescanoffset = 0;
         rescanct = total;
         memcpy(rescan, grpkt+HDRSIZE, (size_t) rescanct);
         return(ERROR);
      }
/*--------------------------------------------------------------------*/
/*    The checksum is correct, now determine the length of the        */
/*    data to return.                                                 */
/*--------------------------------------------------------------------*/

      *len = total;

      if (c2 & 0x40)
      {
         int ii;
         if ( grpkt[HDRSIZE] & 0x80 )
         {
            ii = (grpkt[HDRSIZE] & 0x7f) + ((grpkt[HDRSIZE+1] & 0xff) << 7);
            *len -= ii;
            memcpy(data, grpkt + HDRSIZE + 2, *len);
         }
         else {
            ii = (grpkt[HDRSIZE] & 0xff);
            *len -= ii;
            memcpy(data, grpkt + HDRSIZE + 1, *len);
         } /* else */
      }
      else
         memcpy(data, grpkt + HDRSIZE, *len);
      data[*len] = '\0';
   }

   printmsg(12, "receive packet type %d, yyy=%d, xxx=%d, len=%d",
      type, *yyy, *xxx, *len);
   printmsg(13, " checksum rec=%04x comp=%04x\ndata=|%.*s|",
      check, checkchk, total, data);

   return(type);

} /*grpack*/


/*
   c h e c k s u m
*/

static unsigned int checksum(char *data, int len)
{
   int i, j;
   unsigned int tmp, chk1, chk2;
   chk1 = 0xffff;
   chk2 = 0;
   j = len;
   for (i = 0; i < len; i++) {
      if (chk1 & 0x8000) {
         chk1 <<= 1;
         chk1++;
      } else {
         chk1 <<= 1;
      }
      tmp = chk1;
      chk1 += (data[i] & 0xff);
      chk2 += chk1 ^ j;
      if ((chk1 & 0xffff) <= (tmp & 0xffff))
         chk1 ^= chk2;
      j--;
   }
   return(chk1 & 0xffff);

} /*checksum*/

gwrmsg(char *str, boolean nowait)
{
	char spacket[MAXPACK];
	int len;
	len = strlen(str) + 1;
	if (nowait) {
		return gsendpkt(str, len);
	}
	while (len > pktsize)  {
		strncpy((char *) spacket, str, pktsize);
		spacket[pktsize] = '\0';
		if (gsendpkt((char *) spacket, pktsize)) /* don't re-check len */
			return(1);
		str += pktsize;
		len -= pktsize;
	}
	strcpy((char *) spacket, str);
	if (gsendpkt((char *) spacket, len))
		return 1;      /* send 'S fromfile tofile user - tofile 0666' */
	return 0;
}

grdmsg(register char *str, int *bytes)
{
	return ggetpkt(str, bytes);
}



