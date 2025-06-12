/*
*   Util.c
*   utility library for use with the Network kernel
*
*/
#include "stdio.h"
#include "memory.h"
#include "whatami.h"
#include "hostform.h"

#ifdef PC
#include "string.h"
#else
#ifdef AZTEC
#define strchr(A,B) index(A,B)
#endif
#endif

char 							/* special function types */
			*neterrstring();
int32 time();					/* don't forget this sucker! */

extern struct machinfo *Sns;

static unsigned char
			*Ssstemps[] = {
			"capfile",
			"hp.out",
			"ps.out",
			"tek.out"
			};

char Sptypes[NPORTS];		/* port types assigned for session use */


extern struct config Scon;		/* hardware configuration */

#define NTIMES 30

/*
*  timer queue of events which will be placed into the event queue
*  when the time is up.
*/
struct {
	unsigned char
		eclass,					/* event queue data */
		event;
	int
		next,					/* next item in list */
		idata;
	int32 when;				/* when timer is to go off */
} Stq[NTIMES];

static int
		domwait = 0,			/* is domain waiting for nameserver? */
		Stfirst,Stfree;			/* pointers for timer queue */

#define PFTP 1
#define PRCP 2
#define PDATA 3
#define PDOMAIN 4

/************************************************************************/
/*  Snetinit
*   Do network initialization for those who want the defaults all
*   set for them.  Recommend that neterrchange be called before
*   initializing network stuff.
*/
Snetinit()
	{
	int i;

/*
*  set up the file names
*/
	Scon.capture = Ssstemps[0];
	Scon.hpfile = Ssstemps[1];
	Scon.psfile = Ssstemps[2];
	Scon.tekfile = Ssstemps[3];

	neteventinit();				/* initializes for error messages to count */

	for (i=0; i<NPORTS; i++)
		Sptypes[i] = -1;			/* clear port type flags */

	for (i=0; i<NTIMES; i++)
		Stq[i].next = i+1;			/* load linked list */
	Stq[NTIMES-1].next = -1;		/* anchor end */
	Stfirst = -1;
	Stfree = 0;

	if (!Sreadhosts()) { 			/* parses config file */
#ifdef PC
		netparms(Scon.irqnum,Scon.address,Scon.ioaddr);
#endif
		netconfig(Scon.hw);

		if (!netinit()) {			/* starts up hardware */
/*
*  Check for the need to RARP and do it
*/
			netgetip(Scon.myipnum);	/* get stored ip num */
			if (comparen(Scon.myipnum,"RARP",4)) {	/* need RARP */
				if (netgetrarp())	/* stores in nnipnum at lower layer */
					return(-2);
				netgetip(Scon.myipnum);
				netsetip(Scon.myipnum);	
			}

/*
*  Give the lower layers a chance to check to see if anyone else
*  is using the same ip number.  Usually generates an ARP packet.
*/
			netarpme(Scon.myipnum);		

			Ssetgates();			/* finishes IP inits */
			Stask();
			return(0);
		}
	}

	return(-1);	
}

/**************************************************************************/
/*  Snetopen
*
*   Takes a pointer to a machine record, looked up with Sgethost and
*   initiates the TCP open call.
*
*/
Snetopen(m,tport)
	struct machinfo *m;
	int tport;
	{
	int j;

	if (!m || m->mstat < HAVEIP)
		return(-1);

	j = netxopen(m->hostip,tport,m->retrans,m->mtu,m->maxseg,m->window);
									/* do the open call */

	if (j >= 0) {
		Sptypes[j] = -1;			/* is allocated to user */
		Stimerset(CONCLASS,CONFAIL,j,m->conto);
		Stimerset(SCLASS,RETRYCON,j,m->retrans/TICKSPERSEC+2);
	}

	return(j);
}


/**************************************************************************/
/*
*  special domain data structures
*/
#define DOMSIZE 512				/* maximum domain message size to mess with */

/*
*  Header for the DOMAIN queries
*  ALL OF THESE ARE BYTE SWAPPED QUANTITIES!
*  We are the poor slobs who are incompatible with the world's byte order
*/
struct dhead {
uint16
	ident,				/* unique identifier */
	flags,	
	qdcount,			/* question section, # of entries */
	ancount,			/* answers, how many */
	nscount,			/* count of name server RRs */
	arcount;			/* number of "additional" records */
};

/*
*  flag masks for the flags field of the DOMAIN header
*/
#define DQR		0x8000			/* query = 0, response = 1 */
#define DOPCODE	0x7100			/* opcode, see below */
#define DAA		0x0400			/* Authoritative answer */
#define DTC		0x0200			/* Truncation, response was cut off at 512 */
#define DRD		0x0100			/* Recursion desired */
#define DRA		0x0080			/* Recursion available */
#define DRCODE	0x000F			/* response code, see below */

								/* opcode possible values: */
#define DOPQUERY	0			/* a standard query */
#define DOPIQ		1			/* an inverse query */
#define DOPCQM		2			/* a completion query, multiple reply */
#define DOPCQU		3     		/* a completion query, single reply */
/* the rest reserved for future */

								/* legal response codes: */
#define DROK	0				/* okay response */
#define DRFORM	1				/* format error */
#define DRFAIL	2				/* their problem, server failed */
#define DRNAME	3				/* name error, we know name doesn't exist */
#define DRNOPE	4				/* no can do request */
#define DRNOWAY	5				/* name server refusing to do request */

#define DTYPEA	1				/* host address resource record (RR) */
#define DTYPEPTR	12			/* a domain name ptr */

#define DIN		1				/* ARPA internet class */
#define DWILD	255				/* wildcard for several of the classifications */

/*
*  a resource record is made up of a compressed domain name followed by
*  this structure.  All of these ints need to be byteswapped before use.
*/
struct rrpart {
	uint16
		rtype,					/* resource record type = DTYPEA */
		rclass;					/* RR class = DIN */
	uint32
		rttl;					/* time-to-live, changed to 32 bits */
	uint16
		rdlength;				/* length of next field */
	uint8
		rdata[DOMSIZE];			/* data field */
};

/*
*  data for domain name lookup
*/
struct useek {
	struct dhead h;
	uint8 x[DOMSIZE];
} question;

qinit()
	{
	question.h.flags = intswap(DRD);
	question.h.qdcount = intswap(1);
	question.h.ancount = 0;
	question.h.nscount = 0;
	question.h.arcount = 0;
}


/*********************************************************************/
/*  packdom
*   pack a regular text string into a packed domain name, suitable
*   for the name server.
*/
packdom(dst,src)
	char *src,*dst;
	{
	char *p,*q,*savedst;
	int i,dotflag,defflag;

	p = src;
	dotflag = defflag = 0;
	savedst = dst;

	do {							/* copy whole string */
		*dst = 0;
		q = dst + 1;

/*
*  copy the next label along, char by char until it meets a period or
*  end of string.
*/
		while (*p && (*p != '.')) 
			*q++ = *p++;

		i = p - src;
		if (i > 0x3f)
			return(-1);
		*dst = i;
		*q = 0;

		if (*p) {					/* update pointers */
			dotflag = 1;
			src = ++p;
			dst = q;
		}
		else if (!dotflag && !defflag && Scon.defdom) {
			p = Scon.defdom;		/* continue packing with default */
			defflag = 1;
			src = p;
			dst = q;
			netposterr(801);		/* using default domain */
		}

	} while (*p);

	q++;
	return(q-savedst);			/* length of packed string */
}

/*********************************************************************/
/*  unpackdom
*  Unpack a compressed domain name that we have received from another
*  host.  Handles pointers to continuation domain names -- buf is used
*  as the base for the offset of any pointer which is present.
*  returns the number of bytes at src which should be skipped over.
*  Includes the NULL terminator in its length count.
*/
unpackdom(dst,src,buf)
	char *src,*dst,buf[];
	{
	int i,j,retval;
	char *savesrc;

	savesrc = src;
	retval = 0;

	while (*src) {
		j = *src;

		while ((j & 0xC0) == 0xC0) {
			if (!retval)
				retval = src-savesrc+2;
			src++;
			src = &buf[(j & 0x3f)*256+*src];		/* pointer dereference */
			j = *src;
		}

		src++;
		for (i=0; i < (j & 0x3f) ; i++)
			*dst++ = *src++;

		*dst++ = '.';
	}

	*(--dst) = 0;			/* add terminator */
	src++;					/* account for terminator on src */

	if (!retval)
		retval = src-savesrc;

	return(retval);
}

/*********************************************************************/
/*  sendom
*   put together a domain lookup packet and send it
*   uses port 53
*/
sendom(s,towho,num)
	char *s,*towho;
	int16 num;
	{
	uint16 i,ulen;
	uint8 *psave,*p;

	psave = (uint8 *)question.x;

	i = packdom(question.x,s);

/*
*  load the fields of the question structure a character at a time so
*  that 68000 machines won't barf.
*/
	p = &question.x[i];
	*p++ = 0;				/* high byte of qtype */
	*p++ = DTYPEA;			/* number is < 256, so we know high byte=0 */
	*p++ = 0;				/* high byte of qclass */
	*p++ = DIN;				/* qtype is < 256 */

	question.h.ident = intswap(num);
	ulen = sizeof(struct dhead)+(p-psave);

	netusend(towho,53,997,&question,ulen);
	
}

/**************************************************************************/
/*  Sdomain
*   DOMAIN based name lookup
*   query a domain name server to get an IP number
*	Returns the machine number of the machine record for future reference.
*   Events generated will have this number tagged with them.
*   Returns various negative numbers on error conditions.
*/
Sdomain(mname)
	char *mname;
	{
	struct machinfo *m;

	if (!Sns) 							/* no nameserver, give up now */
		return(-1);

	while (*mname && *mname < 33)		/* kill leading spaces */
		mname++;
	if (!(*mname))
		return(-1);

	if (!(m = Smadd(mname)))
		return(-1);						/* adds the number to the machlist */

	if (domwait < Scon.domto)
		domwait = Scon.domto;			/* set the minimum timeout */

	qinit();							/* initialize some flag fields */

	netulisten(997);					/* pick a return port */
	if (!m->hname)
		m->hname = m->sname;			/* copy pointer to sname */

	sendom(m->hname,Sns->hostip,m->mno);	/* try UDP */

	Stimerset(SCLASS,UDPTO,m->mno,domwait);	/* time out quickly first time */
	m->mstat = UDPDOM;

	return(m->mno);

}

/*********************************************************************/
/*  getdomain
*   Look at the results to see if our DOMAIN request is ready.
*   It may be a timeout, which requires another query.
*/

udpdom()
	{
	struct machinfo *m;
	int i,uret,num;
	char *p;

	uret = neturead(&question);

	if (uret < 0) {
/*		netputevent(USERCLASS,DOMFAIL,-1);  */
		return(-1);
	}

	num = intswap(question.h.ident);		/* get machine number */
/*
*  check to see if the necessary information was in the UDP response
*/
	m = Slooknum(num);				/* get machine info record */
	if (!m) {
		netputevent(USERCLASS,DOMFAIL,num);
		return(-1);
	}

/*
*  got a response, so reset timeout value to recommended minimum
*/
	domwait = Scon.domto;

	i = ddextract(&question,m->hostip);

	switch (i) {
		case 3:						/* name does not exist */
			netposterr(802);
			p = neterrstring(-1);
			strncpy(p,m->hname,78);		/* what name */
			netposterr(-1);
			netputevent(USERCLASS,DOMFAIL,num);
			Stimerunset(SCLASS,UDPTO,num);
			return(-1);
		case 0:						/* we found the IP number */
			Stimerunset(SCLASS,UDPTO,num);
			m->mstat = DOM;			/* mark that we have it from DOMAIN */
			netputevent(USERCLASS,DOMOK,num);
			return(0);
		case -1:					/* strange return code from ddextract */
			netposterr(803);
			break;
		default:
			netposterr(804);
			break;
	}

	return(0);

}

/**************************************************************************/
/*  domto
*   Handle time out for DOMAIN name lookup
*   Retry as many times as recommended by config file
*/
domto(num)
	int num;
	{
	struct machinfo *m;

	m = Slooknum(num);
	if (!m)
		return(-1);

	if (m->mstat > UDPDOM + Scon.ndom) {	/* permanent timeout */
		netputevent(USERCLASS,DOMFAIL,num);
		return(-1);
	}
	else
		m->mstat++;			/* one more timeout */
	
	if (domwait < 20)		/* exponential backoff */
		domwait <<= 1;

	Snewns();				/* rotate to next nameserver */

	qinit();

	netulisten(997);					/* pick a return port */
	sendom(m->hname,Sns->hostip,num);		/* try UDP */

	Stimerset(SCLASS,UDPTO,num,domwait);	/* time out more slowly */

	return(num);
	
}

/*********************************************************************/
/*  ddextract
*   extract the ip number from a response message.
*   returns the appropriate status code and if the ip number is available,
*   copies it into mip
*/
ddextract(qp,mip)
	struct useek *qp;
	unsigned char *mip;
	{
	uint16 i,j,nans,rcode;
	struct rrpart *rrp;
	uint8 *p,space[260];

	nans = intswap(qp->h.ancount);				/* number of answers */
	rcode = DRCODE & intswap(qp->h.flags);		/* return code for this message*/
	if (rcode > 0)
		return(rcode);

	if (nans > 0 &&								/* at least one answer */
		(intswap(qp->h.flags) & DQR)) {			/* response flag is set */
		p = (uint8 *)qp->x;					/* where question starts */
		i = unpackdom(space,p,qp);				/* unpack question name */
/*  spec defines name then  QTYPE + QCLASS = 4 bytes */
		p += i+4;
/*
*  at this point, there may be several answers.  We will take the first
*  one which has an IP number.  There may be other types of answers that
*  we want to support later.
*/
		while (nans-- > 0) {					/* look at each answer */
			i = unpackdom(space,p,qp);			/* answer name to unpack */
/*			n_puts(space);*/
			p += i;								/* account for string */
			rrp = (struct rrpart *)p;			/* resource record here */
/*
*  check things which might not align on 68000 chip one byte at a time
*/
			if (!*p && *(p+1) == DTYPEA && 		/* correct type and class */
				!*(p+2) && *(p+3) == DIN) {
				movebytes(mip,rrp->rdata,4);	/* save IP # 		*/
				return(0);						/* successful return */
			}
			movebytes(&j,&rrp->rdlength,2);		/* 68000 alignment */
			p += 10+intswap(j);					/* length of rest of RR */
		}
	}

	return(-1);						/* generic failed to parse */
}


/***********************************************************************/
static int son=1;

Scwritemode(mode)
	int mode;
	{
		son = mode;
		return(0);
}

/***********************************************************************/
Scmode()
	{
	return(son);
}

/***********************************************************************/
static int tekon=1;

Stekmode(mode)
	int mode;
	{
		tekon = mode;
		return(0);
}

/***********************************************************************/
Stmode()
	{
	return(tekon);
}

/***********************************************************************/
#ifdef PC
static int rcpon=1;

Srcpmode(mode)
	int mode;
	{
		rcpon = mode;
		if (rcpon)
			setrshd();
		else
			unsetrshd();
		return(0);
}

/***********************************************************************/
Srmode()
	{
	return(rcpon);
}
#endif

/***********************************************************************/
static int ftpon=0;

Sftpmode(mode)
	int mode;
	{
		if (ftpon && mode)
			return(-1);

		ftpon = mode;
		if (ftpon)
			setftp();
		else
			unsetftp();
		return(0);
}

/***********************************************************************/
Sfmode()
	{
	return(ftpon);
}

/***********************************************************************/
/*  Snewcap
*   set a new capture file name
*/
Snewcap(s)
	char *s;
	{
	if (NULL == (Scon.capture = (char *)NewPtr(strlen(s)+1)))
		return(1);
	strcpy(Scon.capture,s);
	return(0);
}

/***********************************************************************/
/*  Snewps
*   set a new ps file name
*/
Snewpsfile(s)
	char *s;
	{
	if (NULL == (Scon.psfile = (char *)NewPtr(strlen(s)+1)))
		return(1);
	strcpy(Scon.psfile,s);
	return(0);
}

/***********************************************************************/
/*  Snewhpfile
*   set a new HPGL file name
*/
Snewhpfile(s)
	char *s;
	{
	if (NULL == (Scon.hpfile = (char *)NewPtr(strlen(s)+1)))
		return(1);
	strcpy(Scon.hpfile,s);
	return(0);
}

/***********************************************************************/
/*  Snewtekfile
*   set a new tek file name
*/
Snewtekfile(s)
	char *s;
	{
	if (NULL == (Scon.tekfile = (char *)NewPtr(strlen(s)+1)))
		return(1);
	strcpy(Scon.tekfile,s);
	return(0);
}

/***********************************************************************/
/*  Sopencap
*   returns a file handle to an open capture file
*   Uses the locally stored capture file name.
*/
FILE *
Sopencap()
	{
	FILE *retfp;

	if (NULL == (retfp = fopen(Scon.capture,"ab"))) 
		return(NULL);

	fseek(retfp,0L,2);		/* seek to end */

	return(retfp);
}

/**************************************************************************/
/*  Stask
*   A higher level version of netsleep
*
*   This manages the timer queue
*/

static int32 recent=0L;

Stask()
	{
	long t;
	int i;

#ifndef APPLE_DRIVERS
	netsleep(0);
#endif APPLE_DRIVERS

/*
*  Check the timer queue to see if something should be posted
*  First check for timer wraparound
*/
	t = time(NULL);
#ifdef PC
	if (t < recent) {
		i = Stfirst;
		while (i >= 0) {
			Stq[i].when -= WRAPTIME;
			i = Stq[i].next;
		}
	}
#endif
	recent = t;							/* save most recent time */

	while (Stfirst >= 0 && t > Stq[Stfirst].when) {		
								/* Q is not empty and timer is going off */
		i = Stfirst;
		netputevent(Stq[i].eclass,Stq[i].event,Stq[i].idata);
		Stfirst = Stq[Stfirst].next;	/* remove from q */
		Stq[i].next = Stfree;
		Stfree = i;						/* add to free list */
	}


}

/**************************************************************************/
/*  Stimerset
*  Sets an asynchronous timer which is checked in Stask()
*  usage:
*  Time is in seconds
*  Stimerset(class,event,dat,time)
*    int class,event,dat,time;
*    class,event,dat is the event which should be posted at the specified
*    time.  Accuracy is dependent on how often Stask is called.
*/

Stimerset(class,event,dat,howlong)
	int class,event,dat,howlong;
	{
	int i,j,jlast,retval;
	int32 gooff;

	retval = 0;
	gooff = time(NULL) + howlong;

	if (Stfree < 0) {				/* queue is full, post first event */
		Stfree = Stfirst;
		Stfirst = Stq[Stfirst].next;
		Stq[Stfree].next = -1;
		netputevent(Stq[Stfree].eclass,Stq[Stfree].event,Stq[Stfree].idata);
		retval = -1;
	}

	Stq[Stfree].idata = dat;				/* event to occur at that time */
	Stq[Stfree].event = event;
	Stq[Stfree].eclass = class;
	Stq[Stfree].when = gooff;
	i = Stfree;							/* remove from free list */
	Stfree = Stq[i].next;

	if (Stfirst < 0) {					/* if no queue yet */
		Stfirst = i;
		Stq[i].next = -1;				/* anchor active q */
	}

	else if (gooff < Stq[Stfirst].when) {	/* goes first on list */
		Stq[i].next = Stfirst;				/* at beginning of list */
		Stfirst = i;
	}

	else {									/* goes in middle */

		j = jlast = Stfirst;				/* search q from beginning */

		while (gooff >= Stq[j].when && j >= 0) {
			jlast = j;
			j = Stq[j].next;
		}
		Stq[i].next = j;					/* insert in q */
		Stq[jlast].next = i;
	}

	return(retval);
}

/****************************************************************************/
/*  Stimerunset
*   Remove all such timer events from the queue
*   They must match all three fields, event, class and optional data
*
*/
Stimerunset(class,event,dat)
	unsigned char event,class;
	int dat;
	{
	int i,ilast,retval;

	retval = ilast = -1;
	i = Stfirst;
	while (i >= 0 ) {					/* search list */

		if (Stq[i].idata == dat && 		/* this one matches */
			Stq[i].eclass == class && Stq[i].event == event) {

			retval = 0;					/* found at least one */
/*
* major bug fix -- if first element matched, old code could crash
*/
			if (i == Stfirst) {
				Stfirst = Stq[i].next;			/* first one matches */
				Stq[i].next = Stfree;			/* attach to free list */
				Stfree = i;
				i = Stfirst;
				continue;						/* start list over */
			}
			else {
				Stq[ilast].next = Stq[i].next;	/* remove this entry */
				Stq[i].next = Stfree;			/* attach to free list */
				Stfree = i;
				i = ilast;
			}
		}

		ilast = i;
		i = Stq[i].next;
	}

	return(retval);
}

/****************************************************************************/
/*  Scheckpass
*   Check the password file for the user,password combination
*   Returns valid or invalid
*/
Scheckpass(us,ps)
	char *us,*ps;
	{
	char buf[81],*p,*strchr();
	FILE *fp;
	
	if (NULL == (fp = fopen(Scon.pass,"r"))) {
	
#ifdef MAC
/*
*  failed open, so try to find file in the system folder.
*/

		sysdir();							/* change to system folder */
		fp = fopen(Scon.pass,"r");
		
		setmydir();							/* reset back to default dir */
		
		if (NULL == fp)						/* still didn't work? */
#endif
			return(0);
	}

	while (NULL != fgets(buf,80,fp)) {
		p = strchr(buf,'\n');
		*p = '\0';							/* remove \n */

		p = strchr(buf,':');				/* find delimiter */
		*p++ = '\0';
		if (!strcmp(buf,us) &&			/* found user */
			Scompass(ps,p)) {			/* does password check ?*/
			fclose(fp);
			return(1);
		}
	}

	fclose(fp);
	return(0);
}

/****************************************************************************/
/* Sneedpass
*  For other routines to call and find out if a password is required
*/
Sneedpass()
	{
	if (Scon.pass == NULL)
		return(0);

	return(1);
}

/****************************************************************************/
/* Scompass
*  compute and check the encrypted password
*/
Scompass(ps,en)
	char *ps,*en;
	{
	int ck;
	char *p,c;

	ck = 0;
	p = ps;
	while (*p)				/* checksum the string */
		ck += *p++;

	c = ck;

	while (*en) {
		if ((((*ps ^ c) | 32) & 127) != *en)	/* XOR with checksum */
			return(0);
		if (*ps)
			ps++;
		else
			c++;			/* increment checksum to hide length */
		en++;
	}

	return(1);
}

/****************************************************************************/
/*  Sgetevent
*   gets events from the network and filters those for session related
*   activity.  Returns any other events to the caller.
*/
Sgetevent(class,what,datp)
	int class,*what,*datp;
	{
	int retval;

	if (retval = netgetevent(SCLASS,what,datp)) {	/* session event */
		switch (retval) {
			case FTPACT:
				ftpd(0,*datp);
				break;
#ifdef PC
			case RCPACT:				/* give CPU to rsh for rcp */
				rshd(0);
				break;
#endif
#ifdef MAC
			case CLOSEDONE:				/* Used in the drivers */
				netclose( *datp);
				break;
			case CLOSEDONE+1:				/* Used in the drivers */
				netclose( *datp);
				break;
#endif MAC
			case UDPTO:					/* name server not responding */
				domto(*datp);
				break;
			case RETRYCON:
				if (0 < netopen2(*datp)) 	/* connection open yet? */
					Stimerset(SCLASS,RETRYCON,*datp,4);  /* 4 is a kludge */
				break;

			default:
				break;
		}
	}

	Stask();						/* allow net and timers to take place */

	if (!(retval = netgetevent(class,what,datp)))
		return(0);
		
	if (retval == CONOPEN) 
		Stimerunset(CONCLASS,CONFAIL,*datp);   /* kill this timer */

	if ((*datp == 997) && (retval == UDPDATA)) {
		udpdom();
	}
	else if ((*what == CONCLASS) && (Sptypes[*datp] >= 0)) {
										/* might be for session layer */
		switch (Sptypes[*datp]) {
			case PFTP:
				rftpd(retval);
				break;
			case PDATA:
				ftpd(retval,*datp);
				break;
#ifdef PC
			case PRCP:
				rshd(retval);
				break;
#endif
			default:
				break;
		}
	}
	else
		return(retval);				/* let higher layer have it */

	return(0);
}

