/*
*    data.h
*   Declarations of global variables for TCP/IP libraries
*
*/

/*
*  Start with declarations that tell the difference between PC and other
*  computers
*/
#ifdef PC
#include "pcdefs.h"
#else
#include "macdefs.h"
#endif

#ifdef MASTERDEF
unsigned char
	us[] = 
{"National Center for Supercomputing Applications"},
	nnmyaddr[DADDLEN],		/*  my ethernet hardware address */
	broadaddr[DADDLEN],		/*  the broadcast address */	
	nnipnum[4],				/*  my ip number */
	nnredir = 0,			/*  flag indicating need for redirect */
	nnicmpsave[4],			/*  address for icmp redirect */
	nnicmpnew[4],			/*  new gateway from icmp redirect */
	nnmask[4] = {0,0,0,0},	/*  the default subnet mask */
	nnamask[4] = {255,0,0,0},		/* class A mask */
	nnbmask[4] = {255,255,0,0}, 	/* class B mask */
	nncmask[4] = {255,255,255,0},	/* class C mask */
	broadip[4] = {0xff,0xff,0xff,0xff};

int
	nnipident					/*  ident field of outgoing ip packets */
			=1,	
	nnefirst					/* first entry in event q */
			=0,
	nnelast						/* last entry in event q */
			=0,
	nnefree						/* free list for event q */
			=0,
	nnemac						/* Macintosh is using direct EtherTalk */
			=0,
	nndto						/* dlayertimeout */
			=DLAYTIMEOUT,
	nnfromport					/* can force a port number selection */
			= 0,
	nncredit = CREDIT,			/* limited window in some cases */
	nnsegsize = MAXSEG;			/* maximum segment size  (intswapped) */

struct port *portlist[NPORTS];		/* allocate like iobuffers in UNIX */
struct uport ulist;					/* buffer for UDP */
struct pseudotcp tcps;				/* for checksums */
struct acache arpc[CACHELEN];		/* cache for hardware addresses */
struct eq nnq[NEVENTS];				/* event queue */

ARPKT arp;
DLAYER blankd;
IPKT blankip;
/*ICMPKT blankicmp;*/

#else
extern unsigned char
	nnmyaddr[DADDLEN],	/*  my ethernet hardware address */
	broadaddr[DADDLEN]		/*  the broadcast address */,
	broadip[4],
	nnipnum[4],
	nnredir,			/*  flag indicating need for redirect */
	nnicmpsave[4],			/*  address for icmp redirect */
	nnicmpnew[4],			/*  new gateway from icmp redirect */
	nnmask[4],
	nnamask[4],
	nnbmask[4],
	nncmask[4];

extern int
	nnipident				/*  ident field of ip */,
	nnefirst					/* first entry in event q */,
	nnelast						/* last entry in event q */,
	nndto						/* dlayertimeout */,
	nnefree,
	nnemac,						/* Macintosh is using direct EtherTalk */
	nnfromport,					/* can force a port number selection */
	nncredit,
	nnsegsize;			/* maximum segment size */

extern struct port *portlist[NPORTS];		/* allocate like iobuffers in UNIX */
extern struct uport ulist;					/* buffer for UDP */
extern struct pseudotcp tcps;				/* for checksums */
extern struct acache arpc[CACHELEN];		/* cache for hardware addresses */
extern struct eq nnq[NEVENTS];				/* event queue */

extern ARPKT arp;
extern DLAYER blankd;
extern IPKT blankip;
/*extern ICMPKT blankicmp;*/

#endif

/*
*   defines for types for functions, global to everyone 
*/
char *nbgets();
uint8 *getdlayer(),*netdlayer();
char *neterrstring();			/* some more static data for driver */

/*
*   defines of constants and macros that everyone needs to know
*/

#define nnerror(A)  netposterr(A)
