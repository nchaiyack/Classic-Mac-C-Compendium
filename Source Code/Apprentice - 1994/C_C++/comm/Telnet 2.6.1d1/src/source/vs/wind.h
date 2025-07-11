/*
 *		MacLook Window Management Data Structure Definition
 */
 
#include "kerberos.h" 

#define MAXFTP	128			/* BYU mod */
#define	MAXKB	256			/* BYU mod */

#define TYPE_I_ON_CLOSE	1	/* BYU mod */
#define CAPTURE_DATA	2	/* BYU mod */
#define MGET_STATE		4	/* BYU LSC */
#define MPUT_STATE		8	/* BYU LSC */
#define PASTE_IN_PROGRESS	0x4000 /* BYU LSC */

//	The following are for the "active" field.
#define	CNXN_NOTINUSE	-2
#define	CNXN_DNRWAIT	-1
#define	CNXN_ACTIVE		 0
#define	CNXN_OPENING	 1
#define	CNXN_ISCORPSE	 2
#define MHOPTS_BASE		37	// Base option for {my,his}opts (Authenticate)
							//  {my,his} opts should only be used for telnet options
							//  in the range starting at MHOPTS_BASE and limited
							//  by MHOPTS_SIZE. This saves memory.
#define MHOPTS_SIZE		2	// Number of options supported in {my,his}opts

#define SLC_ARRAY_SIZE	18	// This should match the value of SLC_MAX from parse.h

struct WindRec {
short	
	vs,				/* virtual screen number */					/* BYU 2.4.15 */
	port,			/* TCP/IP port number */
	active,			/* See above for definition of possible values */
	enabled,		/* Are we currently enabled for receive? */

	vtemulation,	/* 0 = VT100, 1 = VT 220 */
	bsdel,			/* backspace or delete is default */
	eightbit,		/* eight bit font displayed (false is seven bit display */	/* BYU 2.4.9 */
	national,		/* LU/MP: translation table to use for this connection */
	arrowmap,		/* MAT: should we allow the arrow keys to be mapped?? */
	showErrors,		/* show ALL errors if this is set */
	pgupdwn,		/* JMB/MAT: should we have page up/down do local window movement? */
	emacsmeta,		/* JMB/SMB:	should option key work as EMACS meta key? */
	Xterm,			/* JMB/WNR:	should Xterm sequences be recognized? */
	halfdup,		/* If true then half-duplex mode */
	forcesave,		/* NCSA 2.5: force lines to be saved */
	crmap,			/* BYU mod - CR's second byte is ... */

	tekclear,		/* Does tektronix (1) clear screen or (0) add window */
	tektype,		/* -1 = TEK not allowed, 0 = 4014, 1 = 4105 */
	curgraph,		/* Associated Tek drawing */

	maxscroll,		/* Requested amount of scroll back */
	width,			/* Width we started with */
	wrap,			/* whether vtwrap is set or not for this screen */
	echo,			/* NOTE: this is backwards - I am echoing your chars */
	ESscroll,		/* will Clear Screen Save Lines? */
	termstate,		/* Emulation State (-1 is none, 0 is VT100) */
					/* = TEKTYPE, send data to VGwrite */
					/* = RASTYPE, send data to VRwrite */

	naws,			/* NCSA: will negotiate NAWS */
	lineAllow,		/* allow linemode? */
	Isga,			/* I am supressing go ahead */
	Usga,			/* You are supressing go ahead */
	Ittype,			/* I am sending terminal type negotiations.. */
	telstate,		/* Telnet State */
	timing,			/* True if waiting for timing mark */
	substat,		/* Telnet Subnegot. State */
	parseIndex,		/* save an index into the subnegotiation parsed data */

	portNum,		/* port number from the application save set */
	ftpport,		/* BYU mod - FTP port number */
	ftpstate,		/* BYU mod - Telnet session = 0, Ftp session != 0 */
	ftpnext,		/* BYU mod - Next ftpstate after waiting for response from "PORT" */
	xfer,			/* File X-fer status */
	kblen,			/* Pointer to next char in buffer to be used */
	capturesize,	/* BYU mod - ftp client size of captured data */
	clientflags;	/* BYU mod - boolean flags for ftp client */

Str63
	machine;		// Name of machine we are connecting to

Str32
	answerback;		/* Message to send when server sends TERMTYPE Telnet option */

/* 	The following are used during paste operations */
short
	outlen,			/* Length of text remaining to be pasted */
	pastemethod,	/* 1 = All at once, 0 = in blocks of size pasteblock */
	pastesize;		/* Size of paste "blocks" */
	
char
	*outptr,		/* Pointer to next char to send */
	**outhand;		/* Handle to text from the Paste */

long   
	incount,		/* BYU LSC - Count of bytes into this port */
	outcount;		/* BYU LSC - Count of bytes out this port */
/* ---- */

short
	rows,			/* handshaking */
	col,
	lmode,			/* Current linemode MODE.  Currently support EDIT and TRAPSIG */
	slc[SLC_ARRAY_SIZE+1];
	
char
	TELstop,		/* Character for scrolling to stop */
	TELgo,			/* Character for scrolling to go */
	TELip,			/* Character for interrupt process */
	ftpbuf[MAXFTP],	/* BYU mod - Outgoing ftp commands buffer */
	kbbuf[MAXKB];	/* The keyboard buffer (echo mode ) */
	
GrafPtr 
	wind;
	
Handle
	myInitParams;	//	So we can look up stuff after we have DNS'd.

	ADATA			// Kerberos/encryption support

unsigned char
	myopts[MHOPTS_SIZE],	// My telnet options. Warning: see MHOPTS_BASE
	hisopts[MHOPTS_SIZE],	// His telnet options. Warning: see MHOPTS_BASE
	parsedat[450];			// save the subnegotiation data here
						    //   must be big enough for a Kerberos AP message

char
	cannon[80];		// Cannonical hostname

Boolean
	authenticate,	// true if authenticating connection wanted
	encrypt;		// true if encrypting connection wanted

};

typedef struct WindRec WindRec;

