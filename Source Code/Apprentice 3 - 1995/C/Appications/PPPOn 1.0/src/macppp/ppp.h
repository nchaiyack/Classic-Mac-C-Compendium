/*
 * Include file for MacPPP.  Should be pre-compiled with THINK C
 *	to generate the ppp.h file.
 *
 * Copyright 1992-1993 Merit Network, Inc. and The Regents of the
 *  University of Michigan.  Usage of this source code is restricted
 *  to non-profit, non-commercial purposes.  The source is provided
 *  "as-is", without warranty.
 */
#ifndef _PPP_H
#define _PPP_H

#include <Timer.h>

/* uncomment this if you want alot of DebugStr statements */
#define PPP_DEBUG_CHECKS(x)	/* DebugStr(x) */
/* uncomment below if you want to log certain events to memory */
/* #define LOG */
#pragma options(check_ptrs)

#define	uchar(x) ((unsigned char)(x))

#include "MacTCPCommonTypes.h"

typedef struct LapInfo	LapInfo;
typedef struct PPPiopb	PPPiopb;
typedef struct sdiopb	sdiopb;
typedef struct TMtimer	TMtimer;

#include "headers.h"		/* TCP/IP header structures */
#include "slhc.h"

/* iopbs to queue transmit requests */
#define	NUMIOPBS 24

/* define size for transmit fifo */
#define XMITQLEN 256
#define MAXXMIT 64

/* size for receive buffer (to get data from serial driver with PBRead) */
#define RXBUFSIZE 192

/* size for receive driver input buffer */
/* this needs to be fairly large for slow Macs to run at higher speeds */
#define SDINBUFLEN 1024

#define PARAM_DOWN	0x81
#define PARAM_UP	0x82

#define TASK_QUEUED 0x8000

/* Lap Transition events */
#define	TransitionOpen  0L
#define	TransitionClose 2L

/* Basic buffer structure */
struct bufheader {
	b_8		*dataptr;	/* pointer to start of data */
	short	length;		/* length of data */
};

#define	YANKBYTE(bufptr)\
 (bufptr->length != 0 ? \
 (bufptr->length--, *bufptr->dataptr++) : -1)

#define PPP_IP_PROTOCOL 	0x0021
#define PPP_VJ_COMP_PROTOCOL	0x002D
#define PPP_COMPR_PROTOCOL	0x002D
#define PPP_VJ_UNCOMP_PROTOCOL	0x002F
#define PPP_UNCOMP_PROTOCOL	0x002F
#define PPP_IPCP_PROTOCOL	0x8021
#define PPP_LCP_PROTOCOL	0xC021
#define PPP_PAP_PROTOCOL	0xC023
#define HDLC_ALL_ADDR	0xff	/* HDLC all-station */
#define HDLC_UI			0x03	/* HDLC Unnumbered Information */
#define PPP_HDR_LEN	4	/* Max bytes for PPP/HDLC envelope header */

/* character defines */
#define ENTER 3
#define RETURN 13
#define TAB 9
#define BACKSPACE 8

/*
 *  Timer
 */

#define NULLTIMER	(struct TMtimer *)0

struct TMtimer
{
	TMTask			atm;		/* time manager task struct */
	long			duration;	/* millisecond delay (must be positive) */
	struct fsm_s	*fsm_p;		/* pointer to fsm to which this belongs */
	ProcPtr		proc;		/* procedure to execute on timeout */
	b_8				state;		/* state of the this timer */
};

/* Timer states */
#define TIMER_STOP	0
#define TIMER_RUN	1
#define TIMER_EXPIRE	2

struct TMprocess
{
	TMTask			atm;
	union {
		LapInfo			*lap;		/* place to save our lap pointer */
		struct fsm_s	*fsm_p;		/* fsm pointer */
	} tmsavptr;
};

/*
 *  PPP characters
 */

enum {	PPP_FLAG	=   0x7E,   /* Opening/closing frame flag */
	PPP_ESCAPE	=   0x7D   /* escape next character */
};

typedef enum { s_Idle, s_Data, s_Escape, 
                  s_Init, s_Finish, s_SendFCS, s_Loopback } HDLCState;

#define FCS_INIT    0xFFFF
#define FCS_TERM    0xF0B8
/* config packet header */
struct config_hdr {
	b_8 code;
#define CONFIG_REQ	 1
#define CONFIG_ACK	 2
#define CONFIG_NAK	 3
#define CONFIG_REJ	 4
#define TERM_REQ	 5
#define TERM_ACK	 6
#define CODE_REJ	 7
#define PROT_REJ	 8
#define ECHO_REQ	 9
#define ECHO_REPLY	10
#define DISCARD_REQ	11

	b_8 id;
	b_16 len;
};
#define CONFIG_HDR_LEN	4	/* Length of config packet header */

/* config option header */
struct option_hdr {
	b_8 type;		/* protocol dependant types */
	b_8 len;
};
#define OPTION_HDR_LEN	2	/* Length of option header */

/* Supported Configuration Protocol index */
enum { Lcp, Pap, IPcp, fsmi_Size };

/* Protocol Constants needed by State Machine */
struct fsm_constant_s {
	b_16	protocol;		/* Protocol number */
	b_8		fsmi;			/* Finite State Machine index */
	b_8		try_req;		/* # of tries for config request */
	b_8		try_nak;		/* # tries for nak substitutes */
	b_8		try_terminate;	/* # tries for terminate */
	b_8		*option_lengths;	/* ptr to array of option lengths */
	b_8		option_limit;	/* # of options recognized */
	b_8		timeout;		/* Time for timeouts (seconds)*/
};

/* FSM states */
enum { fsmINITIAL, fsmSTARTING, fsmCLOSED, fsmSTOPPED, fsmCLOSING, fsmSTOPPING,
		fsmREQ_Sent, fsmACK_Rcvd, fsmACK_Sent, fsmOPENED, fsmState_Size };

/* State Machine Control Block */
struct fsm_s {
	b_8		state;			/* FSM state */
	b_8		lastid;			/* ID of last REQ we sent */
	b_8		retry;			/* counter for timeouts */
	b_8		retry_nak;		/* counter for naks of requests */
	LapInfo *lap;			/* place to stuff lap pointer */
	struct	TMtimer timer;
	struct	fsm_constant_s pdc;	/* protocol dependent constants */
	void	*pdv;			/* pointer to protocol dependent variables */
};

/* Link Phases */
enum {
	pppDEAD = 0,	/* Link Dead, Waiting for physical layer */
	pppESTABLISH,	/* Link Establishment (LCP) phase */
	pppAUTHENTICATE,	/* Authentication Phase */
	pppNETWORK,		/* Network-Layer Protocol Phase */
	pppTERMINATE,	/* Link Termination Phase */
	pppPhase_Size
};

					/* LCP option types */
#define LCP_MRU			0x01
#define LCP_ACCM		0x02
#define LCP_AUTHENT		0x03
#define LCP_QUALITY		0x04
#define LCP_MAGIC		0x05
#define LCP_PFC			0x07
#define LCP_ACFC		0x08
#define LCP_OPTION_LIMIT	0x08	/* highest # we can handle */
#define LCP_N_MRU		(1 << LCP_MRU)
#define LCP_N_ACCM		(1 << LCP_ACCM)
#define LCP_N_AUTHENT	(1 << LCP_AUTHENT)
#define LCP_N_MAGIC		(1 << LCP_MAGIC)
#define LCP_N_QUALITY	(1 << LCP_QUALITY)
#define LCP_N_PFC		(1 << LCP_PFC)
#define LCP_N_ACFC		(1 << LCP_ACFC)

/* Table for LCP configuration requests */
struct lcp_value_s {
	b_16	mru;			/* Maximum Receive Unit */
	b_16	authentication;	/* Authentication protocol */
	b_32	accm;			/* Async Control Char Map */
	b_32	magic_number;	/* Magic number value */
};

/* for test purposes, accept anything we understand in the NAK */
#define LCP_NEG LCP_N_MRU | LCP_N_ACCM | LCP_N_AUTHENT | LCP_N_PFC | LCP_N_ACFC | LCP_N_MAGIC

/* Other configuration option values */
#define LCP_ACCM_DEFAULT	0xffffffffL
#define LCP_MRU_DEFAULT	1500
#define LCP_MRU_HI	1500	/* High MRU limit */
#define LCP_MRU_LO	128		/* Lower MRU limit */

#define PPP_OVERHEAD 8		/* room for addr, cntl, protocol and 32-bit FCS */
#define	PPP_BUFSIZE LCP_MRU_HI + PPP_OVERHEAD /* size of buf for PPP packet */

#define LCP_NAK_TRY	10		/* NAK attempts */
#define LCP_TERM_TRY 2		/* tries on TERM REQ */

/* PAP Parameters */
#define PAPMESSLEN 128
#define PAPUSERLEN 32
#define PAPPASSLEN 32
	/* PAP control block */
struct pap_s {
	b_8		username[PAPUSERLEN];	/* Username for REQ */
	b_8		password[PAPPASSLEN];	/* Password for REQ */
	b_8		message[PAPMESSLEN];	/* message from last ACK/NAK */
	b_8		IOFlag;			/* flag to indicate userio necessary */
};


					/* IPCP option types */
#define IPCP_COMPRESS		0x02
#define IPCP_ADDRESSES		0x01
#define IPCP_ADDRESS		0x03	/* newer IP ADDRESS negotiation */
#define IPCP_OPTION_LIMIT	0x03	/* highest # we can handle */
#define IPCP_N_ADDRESS		(1 << IPCP_ADDRESS)
#define IPCP_N_COMPRESS		(1 << IPCP_COMPRESS)
#define IPCP_N_ADDRESSES	(1 << IPCP_ADDRESSES)

/* Table for IPCP configuration requests */
struct ipcp_value_s {
	b_32	address;		/* address for this side */
	b_32	other;			/* address for other side */
	b_16	compression;	/* Compression protocol */
	b_16	slots;			/* Slots (0-n)*/
	b_8		slot_compress;	/* Slots may be compressed (flag)*/
};

/* for test purposes, accept anything we understand */
#define IPCP_NEG  IPCP_N_ADDRESSES | IPCP_N_COMPRESS | IPCP_N_ADDRESS;

#define IPCP_SLOT_DEFAULT	 16	/* Default # of slots */
#define IPCP_SLOT_HI		 16	/* Maximum # of slots (preallocated) */
#define IPCP_SLOT_LO 		 1	/* Minimum # of slots */
#define IPCP_SLOT_COMPRESS	0x01	/* May compress slot id */
#define	MAXSLOTS IPCP_SLOT_HI	/* determine how much space to get */	

#define IPCP_NAK_TRY	10		/* NAK attempts */
#define IPCP_TERM_TRY	2		/* tries on TERM REQ */

/*
 *	local.want:	Options to request and desired values
 *	local.will:	Options to accept in a NAK from remote.
 *	local.work:	Options currently being negotiated.
 *			Value is valid only when negotiate bit is set.
 *	remote.want:	Options to suggest by NAK if not present in REQ.
 *	remote.will:	Options to accept in a REQ from remote.
 *	remote.work:	Options currently being negotiated.
 *			Value is valid only when negotiate bit is set.
 */

union value_s {
	struct	lcp_value_s lcp_option;
	struct	ipcp_value_s ipcp_option;
};

struct option_s {
	b_16			will_negotiate, want_negotiate;
	union value_s	want;
	b_16			work_negotiate;
	union value_s	work;
};

struct proto_s {
	struct	option_s local, remote;
};

 /* structure to queue writes */

struct PPPiopb {
	QElemPtr	qLink;		/* next in queue */
	short		qType;		/* qType field */
	struct bufheader *bufptr;		/* ptr to buffer */
	struct ipbuf	*ipbuf;		/* ptr to ipbuf */
};

/* Data structure for PPP Preferences file */
/* Note: This struct MUST map into the option_s structure !! */

struct lcp_will_want {
	b_16	will_negotiate, want_negotiate;
	struct	lcp_value_s want;
};

struct lcpconfig {
	struct lcp_will_want local, remote;
	b_8		req_tries;
	b_8		timeout;
};

struct ipcp_will_want {
	b_16	will_negotiate, want_negotiate;
	struct	ipcp_value_s want;
};

struct ipcpconfig {
	struct ipcp_will_want local, remote;
	b_8		req_tries;
	b_8		timeout;
};

#define MAXSLEN 42		/* max string length */
#define NUMCOMMANDS 8	/* number of strings */
#define PREF_VERSION 3	/* version of prefs file */
#define NUMCONFIGS 4	/* number of configurations */
struct ppp_pref {
	b_16	version;		/* version of Preferences file */
	b_8		portname[32];	/* port name (from CommToolBox) */
	Boolean	hangup;			/* send modem hangup command on close */
	Boolean	use_pulse;		/* use pulse for dialing */
	Boolean quiet;			/* Quiet mode flag */
	Boolean use_term;		/* bring up a terminal window on connection establishment */
	b_16	timeout;		/* idle timeout (minutes), 0 = none */
	b_16	echo;			/* PPP LCP echo interval  0 = off */
	b_16	active_config;	/* active config index */
	b_16	max_config;		/* maximum conifg index */
	b_16	echo_tries;		/* number of echos before down */
	b_16	max_window;		/* maximum window size */
};

struct ppp_config {
	struct	lcpconfig	lcpconf;
	struct	ipcpconfig	ipcpconf;
	b_8		pap_retries;				/* number of tries for PAP */
	b_8		pap_timeout;			/* timeout for PAP */
	b_16	connecttimeout;
	b_16	waittimeout;
	b_16	baudrate;		/* port speed */
	b_8		flags;
#define CTSBIT 0x1
#define RTSBIT 0x2
#define FLOWBITS CTSBIT+RTSBIT
#define USE_PULSE 0x4
	b_8		config_name[MAXSLEN + 1];		/* configuration name */
	b_8		defaultid[MAXSLEN + 1];		/* add one for length byte */
	b_8		defaultpw[MAXSLEN + 1];
	b_8		modeminit[MAXSLEN + 1];
	b_8		phonenum[MAXSLEN + 1];
	struct {
		Boolean	sendout;		/* should we send or wait for this string */
		Boolean addreturn;		/* add a carriage return to end of string */
		b_8		scriptstr[MAXSLEN + 1];	/* script string */
	} commands[NUMCOMMANDS];
};

struct IPCONFIG {
	long		version;
	long		flags;
	long		dfl_ip_addr;
	long		dfl_net_mask;
	long		dfl_broadcast_mask;
	long		dfl_gateway_addr;
	unsigned char	server_lap_address[8];
	long		configIPAddr;
	long		configNetMask;
	long		dfl_dyn_low;
	long		dfl_dyn_high;
	char		dfl_zone[33];		/* ### */
/* align to word */
	Byte		load;
	Byte		admin;
	Byte		netLock;
	Byte		subnetLock;
	Byte		nodeLock;
	Byte		filler1;
	long		activeLap;
	long		slot;
	char		filename[33];		/* ### */
};

struct icmpEchoInfo {
	unsigned long	echoRequestOut;	/* time when echo req. went out */
	unsigned long	echoReplyIn;	/* time when echo reply received */
	struct rdsEntry	echoedData;	/* data received in response */
	Ptr		options;	/* IP Options */
	unsigned long	userDataPtr;	/* userDataPtr for app stuff */
};

typedef struct ipbuf {
	IOParam		iop;		/* MAC OS I/O Param block */
	struct icmpEchoInfo	echoInfo;	/* ping stuff */
	struct ipbuf	*segipb;	/* pointer to segment's IPB if fragmented */
	struct LapInfo	*lap;		/* LAP pointer */
	ProcPtr		lap_ioc; 	/* local net completion routine */
	ProcPtr		ip_ioc;		/* IP completion routine */
	ProcPtr		tp_ioc;		/* transport completion routine */
	ProcPtr		data_ioc;	/* data IOC */
	struct wdsEntry	laphdr;		/* local net header */
	struct wdsEntry	ip;		/* IP header */
	struct wdsEntry	tp;		/* TCP/UDP or ICMP header */
	struct wdsEntry	data;		/* TCP/UDP data */
	struct wdsEntry	d1;
	struct wdsEntry	d2;
	struct wdsEntry	d3;
	struct wdsEntry	d4;		/* 8 wds entries plus 0 terminator */
	short		flag;		/* zero terminator to WDS */
	char		packet[];	/* start of variable length pkt */
} ipbuf;

struct rdStruct {
	ProcPtr	ph_rp;		/* pointer to "ReadPacket" routine (a4)	*/
	ProcPtr	ph_rr;		/* pointer to "ReadRest" routine 2(a4)	*/
	long	ph_bytesleft;	/* number of bytes left to read	*/
	union {
		struct {	/* storage for standard AppleTalk LAP protocol handler */
			long	phb_a4;		/* used by driver	*/
			long	phb_a0;		/* used by driver	*/
			long	phb_a1;		/* used by driver	*/
			long	phb_a2;		/* used by driver	*/
			} phb;
		struct {	/* storage for a buffered LAP interface */
			b_8	*lnb_ptr;	/* pointer to next byte to get from buffer	*/
			} lnb;
		struct {	/* storage for a reassembled IP packet */
			struct fragment	*rsmb_fragbuffer;	/* buffer to fragment data buffer */
			b_8	*rsmb_ptr;	/* pointer to next byte to get from buffer */
			long	rsmb_bytesleft;	/* number of bytes left in this buffer */
			long	rsmb_byteoffset;/* offset of next byte in rsm'd pkt */
			} rsm;
		} rdsparm;
	Byte	lapBroadcast;	/* LAP-level broadcast */
	Byte	ipBroadcast;	/* IP-level broadcast */
	struct LapInfo	*lap;
	struct wdsEntry	laphdr;	/* local net header */
	struct wdsEntry	ip;	/* ip header !!! (IPwdsEntry) ???*/
	struct wdsEntry	tp;	/* TCP/UDP or ICMP header */
	struct wdsEntry	data;	/* TCP/UDP data */
};

struct sdiopb {
	IOParam		iop;	/* IOPB for serial driver use */
	LapInfo		*lap;	/* ptr to LapInfo */
	};
	
typedef unsigned long (*longProcPtr)();

#define BUFOFFSET sizeof(struct bufheader) + sizeof(struct ipheader) + sizeof(struct tcpheader)
#define	BUFFERSIZE PPP_BUFSIZE + BUFOFFSET
#define NUMBUFFERS 4	/* number of buffers to allocate */
#define LCP_ECHO_BUFSIZE 12 /* buffer size for lcp echo requests */

struct LapInfo {
	b_32		cur_ip_addr;	/* LAP's IP address */
	b_32		cur_net_mask;	/* LAP's IP net-mask */
	b_32		ip_broadcast_addr;	/* IP's broadcast address */
	struct IPCONFIG	lc;		/* copy of IP LAP cnfg rsrc */
	ProcPtr	lapInit;	/* ptr to LAP init routine */
	ProcPtr	lapOpen;	/* LAP open rtn */
	ProcPtr	lapClose;	/* LAP close rtn */
	ProcPtr		lapUnload;	/* LAP unload rtn, undoes lapInit */
	ProcPtr	lapAttach;	/* LAP attach PH rtn */
	ProcPtr	lapDetach;	/* LAP detach PH rtn */
	ProcPtr	lapOutput;	/* LAP output rtn */
	ProcPtr	lapControl;	/* LAP control rtn */
	ProcPtr		lapFault;	/* LAP fault isolation rtn */
	ProcPtr	lapStatistics;	/* LAP statistic reading rtn */
	ProcPtr		lapConfigure;	/* LAP configuration rtn */
	ProcPtr	lapProbe;	/* send a LAP-specific addr probe pkt */
	ProcPtr	lapRegister;	/* register our IP address on net */
	ProcPtr		lapFindGateway;	/* LAP-specific way to find gateway  */
	ProcPtr	lapGwyCheck;	/* LAP-specific way to gateway is up */
/* IP Parameters */
	ip_addr		dfl_dns_addr;	/* addr of DNS from config protocol */
	Handle		dnslHndl;	/* handle to DNS config rsrc */
	Ptr			dnsCache;	/* ptr to DNS cache area */
	long		dnsCacheSize;	/* size of cache in bytes */
/* LAP Parameters */
	long		headerSize;	/* LAP header space required */
	long		trailerSize;	/* LAP trailer space required */
	long		outMaxPacketSize;	/* max output packet size */
	long		inMaxPacketSize;	/* max input packet size */
	long		maxDataSize;	/* max size of data packet */
	long		numConnections;	/* number of separate net connections */
	unsigned long	versionFlags;	/* version number flags */
	ProcPtr	ip_ph;		/* ptr to IP protocol handler */
	Ptr			ipGlobals;	/* ptr to IP's A5 */
	short		link_unit;	/* unit number of link driver */
	Byte		addressConflict;	/* TRUE is address conflict */
	long		lapType;	/* IP LAP hardware type # */
	long		lapAddrLength;	/* size of LAP address field */
	unsigned char 	*lapAddrPtr;	/* ptr to LAP address field */
	unsigned long	reserved;	/* MacTCP reserved field */
	/* PPP specific storage */
	Boolean		ok_to_xmit;	/* ok to transmit on serial port */
	Boolean		term_mode;	/* terminal emulation flag */
	Boolean		needTxPrime;	/* transmit routine needs to be primed */
	Boolean		HasDeferredTasks;	/* indicates if Deferred Tasks available */
	short		serinrefnum;	/* serial input ref num */
	short		seroutrefnum;	/* serial output ref num */
	b_8			rxbuf[RXBUFSIZE];	/* small receive buffer to get data from driver */
	longProcPtr	transProc;	/* ptr to transition Proc */
	longProcPtr savProc;	/* place to save a copy of transition proc pointer */
	Ptr			LapA4;		/* holder for LAP's A4 */
	HDLCState	read_state;	/* state of PPP receiver */
	HDLCState	write_state;	/* state of PPP transmitter */
	short		echo_count;	/* count of outstanding LCP echo requests */
	short		idle_timer;	/* number of idle minutes */
	short		writerr;	/* error on write. Who knows what */
	short		readerr;	/* receiver errors */
	OSErr		lasterr;	/* rc of last PBRead != noErr */
	short		outofiopbs;	/* write failed; too many dgs q'd */
	short		faults;		/* PPPFault was called. */
	short		OutofBuffers;	/* # of getbuffer failures */
	long		OutTxOctetCount;/* # octets sent */
	long		OutOpenFlag;	/* # of open flags sent */
	short		OutError;		/* # packets with error on send */
	long		InRxOctetCount;	/* # octets received */
	long		InOpenFlag;		/* # of open flags */
	short		InUnknown;		/* # unknown packets received */
	short		InCheckSeq;		/* # packets with bad check sequence */
	short		InFramingErr;	/* # framing errors */
	short		InError;		/* # packets with other error */
	short		InIdleToss;		/* non-flag chars while Idle */
	short		InHeader;		/*  Header errors */
	short		InFrameOvr;		/* frame overrun */
	short		InSoftOvr;		/* software overrun */
	short		InHardOvr;		/* hardware overrun */
	struct bufheader	*bufptr; /* current buffer for receive data */
	b_8			*rddata;		/* pointer to end of data in rcv. buffer */
	b_8			xbuf[8];		/* small buffer for copying to fifo */
	struct { 
		b_8		block[BUFFERSIZE];
	} blockarray[NUMBUFFERS];	/* storage for memory blocks */
	struct bufheader *buflist;	/* pointer to buffer list */
	struct {					/* a small buffer for LCP echo requests */
		struct bufheader header;	/* a fake header for the buffer */
		b_8		buffer[LCP_ECHO_BUFSIZE];	/* the actual buffer */
	} lcp_echo_buf;
	QHdr		out_q;			/* q of packets to write */
	QHdr		pppbq;			/* Q of free PPPiopbs */
	PPPiopb		*active;		/* current PPPiopb to transmit */
	PPPiopb		pppiopbs[NUMIOPBS];	/* define storage for iopbs */
	sdiopb		w_iopb;			/* IOPB for serial port writes */
	sdiopb		r_iopb;			/* IOPB for serial port reads */
	CntrlParam	stat_pb;		/* cntrlParam block for status calls */
	struct rdStruct		rds;	/* for passing to ip layer */
	struct ppp_pref	prefdata;	/* Data from Preferences file */
	struct ppp_config configdata;	/* more preferences */
	b_32		PPP_RecvACM;	/* Async Control maps */
	b_32		PPP_XmitACM;
	b_32		PPP_activeACM;	/* current active transmit ACCM */
	b_16		fcstab[256];	/* FCS table for quicker FCS's */
	b_16		XmitFCS;		/* FCS for current transmit frame */
	b_16		RecvFCS;		/* FCS for current rx frame */
	struct	NMRec		closenmtask;	/* Notification task record to close link */
	struct	TMprocess	rxp_task;	/* receive processing task structure */
	struct	TMprocess	txp_task;	/* transmit process task structure */
	struct  TMprocess	echo_task;	/* LCP echo process */
	struct	TMprocess	timeout_task;	/* idle timeout process */
	DeferredTask defer_tx, defer_rx, defer_txcomplete;
	b_8		sdinbuf[SDINBUFLEN]; /* input buffer for serial driver receive */
	b_16	XmitQHead;			/* transmit queue head index */
	b_16	XmitQTail;			/* transmit queue tail index */
	b_16	XmitQSize;			/* transmit queue size */
	b_8		XmitQ[XMITQLEN];	/* transmit queue */
	struct proto_s	lcp_i, ipcp_i;
	struct pap_s	pap_i;
	b_8		ppp_phase;			/* phase of link initialization */
	b_8		ppp_id;				/* id counter for connection */
	b_32	ppp_upsince;		/* Timestamp when Link Opened */
	b_8		ppp_flags;
#define PPP_AP_REMOTE	0x01	/* remote authentication */
#define ECHO_FAIL		0x02	/* LCP echo timeout */
#define IDLE_TIMEOUT	0x04	/* idle timeout */
#define CLOSE_PPP		0x08	/* close PPP flag */
	struct fsm_s ppp_fsm[fsmi_Size];	/* finite state machines */
	struct slcompress	comp;
	struct cstate	rcvslots[MAXSLOTS],txslots[MAXSLOTS];
	b_8		*lcp_option_length_p; /* lcp option lengths */
	struct	lcp_value_s *lcp_default_p;	/* pointer to default option values */
	b_8		*ipcp_option_length_p; /* ipcp options lengths */
	struct	ipcp_value_s *ipcp_default_p;	/* pointer to default option values */
#ifdef LOG
	int		*logp;
	int		log[5*100];
	int		logend;
	int		cushion[10];
#endif
};

typedef struct LapStats {
	short	ifType;
	char	*ifString;
	short	ifMaxMTU;
	long	ifSpeed;
	short	ifPhyAddrLength;
	char	*ifPhysicalAddress;
	union {
			char *noarp;
			} AddrXlation;
	short	slotNumber;
};
	
/* Prototypes and declarations for routines */

/* lap.c */
OSErr 	PPPInit(	LapInfo *, longProcPtr);
OSErr 	PPPOpen(	LapInfo *);
OSErr 	PPPClose(	LapInfo *);
void	PPPUnload(	LapInfo *);
OSErr 	PPPAttachPH(	LapInfo *, ProcPtr);
OSErr 	PPPDetachPH(	LapInfo *);
OSErr 	PPPWrite(	LapInfo *, ip_addr, struct ipbuf *);
OSErr 	PPPControl(	LapInfo *);
void 	PPPFault(	LapInfo *, ip_addr);
void 	PPPConfigure(	LapInfo *);
Boolean	PPPProbe(	LapInfo *, ip_addr);
Boolean	PPPRegister(	LapInfo *);
void	PPPFindGW(	LapInfo *);
OSErr 	PPPStatistics(	LapInfo *, struct LapStats *);

/* misc.c: */
void	AppendStr(b_8 *, b_8 *);
struct bufheader *getbuffer();
void	release(struct bufheader *);
void	makeroom(struct bufheader *, b_16);
b_16	yankbuf(struct bufheader *, b_8 *, b_16);
short	yankbyte(struct bufheader *);	/* returns -1 if nothing */
long	yank16(struct bufheader *);	/* returns -1 if nothing */
b_32	yank32(struct bufheader *);	/* returns  0 if nothing */
short	bytecmp(b_8 *, b_8 *, short);
b_16	get16(b_8 *);
b_32	get32(b_8 *);
b_8		*put16(b_8 *, b_16);
b_8		*put32(b_8 *, b_32);
void	tcp_window_fix(LapInfo *, struct bufheader *);

/* fsm.c */
void	htoncnf(struct config_hdr *, struct bufheader *);
short	ntohcnf(struct config_hdr *, struct bufheader *);
short	ntohopt(struct option_hdr *, struct bufheader *);

short	fsm_sendtermreq(struct fsm_s *);
short	fsm_sendtermack(struct fsm_s *, b_8);
void	fsm_reset(struct fsm_s *);
void	fsm_tld(struct fsm_s *);
void	fsm_tlu(struct fsm_s *);
void	fsm_tlf(struct fsm_s *);
void	fsm_tls(struct fsm_s *);
void	fsm_no_action(struct fsm_s *);
short	fsm_no_check(struct fsm_s *, struct config_hdr *, struct bufheader *);
void	fsm_log(struct fsm_s *, char *);
void	fsm_timer(struct fsm_s *);
void	fsm_timeout(void);
short	fsm_send(struct fsm_s *, b_8, b_8, struct bufheader *);
short	fsm_sendreq(struct fsm_s *);
void	fsm_proc(struct fsm_s *, struct bufheader *);
void	fsm_open(struct fsm_s *);
void	fsm_down(struct fsm_s *);
void	fsm_close(struct fsm_s *);
void	fsm_init(struct fsm_s *);

/* ppp.c */
void	ppp_ready(LapInfo *);
void	ppp_init(LapInfo *);
short	ppp_iostatus(LapInfo *, short);
void	htonppp(LapInfo *, b_16, struct bufheader *);
short	proc_request(struct fsm_s *, struct config_hdr *, struct bufheader *);
short	proc_ack(struct fsm_s *, struct config_hdr *, struct bufheader *);
short	proc_nak(struct fsm_s *, struct config_hdr *, struct bufheader *);
short	proc_reject(struct fsm_s *, struct config_hdr *, struct bufheader *);
short	option_check(struct bufheader *, struct fsm_s *,
							struct option_hdr *, short);
void	makeoptions(struct fsm_s *, struct bufheader *, union value_s *, b_16);
void	add_option(struct fsm_s *, struct bufheader *, union value_s *,
						b_8, b_8, struct bufheader *);
struct bufheader *makereq(struct fsm_s *);

/* link.c */
void	link_open(LapInfo *);
void	link_close();

/* asmutil.c */
void	bzero(b_8 *, short);
void	SetLAPPtr(LapInfo *);
LapInfo	*GetLAPPtr(void);
long	seta5(long);
long	geta5(void);
long	seta4(long);
long	geta4(void);
short	set_sr(short);
short	get_sr(void);

/* io.c */
void	RcvDeferred(void);
void	XmtDeferred(void);
void	TxCDeferred(void);
ProcPtr hdlcrioc(void);
ProcPtr	hdlcwioc(void);
OSErr	QueueFrame(LapInfo *, struct bufheader *, struct ipbuf *);
PPPiopb *get_iopb(LapInfo *);
OSErr	readpkt(struct rdStruct *, Ptr, long);
long	readrest(struct rdStruct *, Ptr, long);
void	rcvip(LapInfo *, struct bufheader *);
OSErr	xmit(sdiopb *, b_8 *, long);
void	ProcRcvPPP(void);
void	ProcXmtPPP(LapInfo *);
void	XmtTMProc(void);
void	IOCompleted(LapInfo *, struct ipbuf *);
void	SerComplete(void);
void	SerReadDone(void);

/* pap.c */
short	pap_remote(LapInfo *);
void	pap_down(struct fsm_s *);
void	pap_proc(struct fsm_s *, struct bufheader *);
OSErr	pap_userio(struct fsm_s *);

/* timer.c */
void	set_timer(TMtimer *, struct fsm_s *, ProcPtr);
void	start_timer(TMtimer *);
void	stop_timer(TMtimer *);

#endif /* _PPP_H */
