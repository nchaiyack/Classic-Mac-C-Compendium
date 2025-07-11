/* 
	IPPB.h	
	C definitions of parameter block entries needed for IP calls

    Copyright Apple Computer, Inc. 1988-91 
    All rights reserved
	
*/

/* control codes */
#define ipctlEchoICMP		17			/* send icmp echo */
#define ipctlLAPStats		19			/* get lap stats */

#define icmpEchoTimeoutErr 		-23035			/* ICMP echo timed-out */

#define ParamBlockHeader 	\
	struct QElem *qLink; 	\
	short qType; 			\
	short ioTrap; 			\
	Ptr ioCmdAddr; 			\
	ProcPtr ioCompletion; 	\
	OSErr ioResult; 		\
	StringPtr ioNamePtr; 	\
	short ioVRefNum;		\
	short ioCRefNum;		\
	short csCode
	
typedef void (*ICMPEchoNotifyProc) (struct ICMPParamBlock *iopb);


struct IPParamBlock {
	ParamBlockHeader;					/* standard I/O header */
	union {
		struct {
			ip_addr	dest;				/* echo to IP address */
			wdsEntry data;
			short timeout;
			Ptr options;
			unsigned short optLength;
			ICMPEchoNotifyProc icmpCompletion;
			unsigned long userDataPtr;
			} IPEchoPB;
		struct {
			struct LAPStats *lapStatsPtr;
			} LAPStatsPB;
		} csParam;
	};

struct ICMPParamBlock {
	ParamBlockHeader;					/* standard I/O header */
	short params[11];
	struct {
		unsigned long echoRequestOut;	/* time in ticks of when the echo request went out */
		unsigned long echoReplyIn;		/* time in ticks of when the reply was received */
		struct rdsEntry echoedData;		/* data received in responce */
		Ptr options;
		unsigned long userDataPtr;
		} icmpEchoInfo;
	};

	
typedef struct LAPStats {
	short	ifType;
	char	*ifString;
	short	ifMaxMTU;
	long	ifSpeed;
	short	ifPhyAddrLength;
	char	*ifPhysicalAddress;
	union {
		struct arp_entry *arp_table;	
		struct nbp_entry *nbp_table;
		} AddrXlation;
	short	slotNumber;
	};
	
#define NBP_TABLE_SIZE	20				/* number of NBP table entries */
#define NBP_MAX_NAME_SIZE	16+10+2

struct nbp_entry {
	ip_addr 	ip_address;				/* IP address */
	AddrBlock	at_address;				/* matching AppleTalk address */
	Boolean		gateway;				/* TRUE if entry for a gateway */
	Boolean		valid;					/* TRUE if LAP address is valid */
	Boolean		probing;				/* TRUE if NBP lookup pending */
	int			age;					/* ticks since cache entry verified */
	int			access;					/* ticks since last access */
	char		filler[116];			/* for internal use only !!! */
	};

#define ARP_TABLE_SIZE	20				/* number of ARP table entries */

typedef struct Enet_addr {
	b_16 en_hi;
    b_32 en_lo;
	} Enet_addr;

typedef struct arp_entry {
	short		age;			/* cache aging field */
	b_16		protocol;		/* Protocol type */
	ip_addr 	ip_address;		/* IP address */
	Enet_addr	en_address;		/* matching Ethernet address */
	};
