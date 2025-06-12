/**************************************************************************/
/*  storage of machine information
*      structure for a linked list of information about other machines
*  that we may want to talk to.
*
*  Defines and structures for use in this file
*/


struct machinfo {
	unsigned char 
		*sname,					/* pointer to name of session */
		*hname,                 /* pointer to name of that machine */
		*font,					/* font name, if we can do it */
		hostip[4],				/* IP number of this machine */
		gateway,				/* gateway preference, start with 1 */
		nameserv,				/* nameserver preference, start with 1 */
		bksp,					/* backspace value */
		halfdup,				/* half duplex required */
		crmap,					/* Strange Berkeley 4.3 CR mode needed */
		ckey,					/* int,sus,res mappings, traditional ^C,^S,^Q */
		skey,
		qkey,
		vtwrap,					/* flag on when need wrap mode */
		vtwidth;				/* how wide screen should be for this session */
	int
		clearsave,				/* whether to save cleared lines */
		fsize,					/* font size in points */
		nfcolor[3],				/* normal foreground */
		nbcolor[3],				/* normal background */
		bfcolor[3],				/* blink             */
		bbcolor[3],
		ufcolor[3],             /* underline */
		ubcolor[3],
		port,					/* TCP port number to access, default = 23 (telnet) */
		mno,					/* machine number for reference */
		mstat,					/* status of this machine entry */
		bkscroll,				/* how many lines to save */
		nlines,					/* number of lines for VT100 screen */
		retrans,				/* initial retrans timeout */
		conto,					/* time out in seconds to wait for connect */
		window,					/* window, will be checked against buffers */
		maxseg,					/* maximum receivable segment size */
		mtu;					/* maximum transfer unit MTU (out) */

	struct machinfo *next;		/* surprise, its a linked list! */
};

struct machinfo *Sgethost(),*Shostlook(),*Slooknum(),*Slookip(),*Smadd();

/*
*  status fields for mstat, what do we know about that machine?
*/
#define NOIP 1					/* we don't have IP number */
#define UDPDOM 3				/* there is a UDP request pending on it */
/*  The next 20 numbers are reserved for UDPDOM */
#define HAVEIP 50				/* at least we have the # */
#define HFILE 70				/* we have IP number from host file */
#define DOM 71					/* we have an IP number from DOMAIN */
#define FROMKIP 72				/* have IP# from KIP server */

/*
*   Configuration information which 
*   the calling program may want to obtain from the hosts file.
*   The calling program should include hostform.h and call
*   Sgetconfig(cp)
*     struct config *cp;
*   which will copy the information to the user's data structure.
*/
struct config {
	unsigned char
		netmask[4],				/* subnetting mask being used */
		havemask,				/* do we have a netmask? */
		irqnum,					/* which hardware interrupt */
		myipnum[4],				/* what is my IP #? */
		me[32],					/* my name description (first 30 chars) */
		color[3],				/* default colors to use */
		hw[10],					/* hardware type for network */
		video[10],				/* video graphics hardware available */
		bios,					/* flag, do we want to use BIOS for screen access */
		tek,					/* flag, enable tektronix graphics */
		ftp,					/* flag, enable ftp server */
		rcp,					/* flag, enable rcp server */
		comkeys,				/* flag, commandkeys=yes */
		*termtype,				/* terminal type specification */
		*zone,					/* AppleTalk zone for KIP NBP */
		*defdom,				/* default domain */
		*capture,				/* pointer to where the capture file name is */
		*pass,					/* pointer to where the password file name is */
		*hpfile,				/* HP file name */
		*psfile,				/* PS file name */
		*tekfile;				/* tek file name */
int
		nstype,					/* nameserver = 1-domain 2-IEN116  */
		domto,					/* time out for UDP domain request */
		ndom,					/* number of retries for domain requests */
		timesl,					/* time slice */
		textblock,				/* text buffering block size */
		address,				/* segment address */
		ioaddr;					/* I/O address */
};

