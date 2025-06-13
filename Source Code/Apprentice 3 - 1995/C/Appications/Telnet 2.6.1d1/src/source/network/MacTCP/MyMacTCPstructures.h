#define	noError	0
#define TCPBUFSIZ	(1024*8)
#define	MAX_FDS_ELEMS	32
#define	MAX_SWDS_ELEMS	16

typedef struct exfds {
	short		inuse;					/* Is this being used? */
	wdsEntry	fds;					/* The real data */
	} exfds;
	
typedef struct StreamRec {				// 470 bytes
	StreamPtr	stream;					/* Apple's lovely Stream Pointer */
	char		*buffer;				/* Where the immovable TCP buffer is */
	short		push;					/* TRUE if we should push next data block */
	char		*sbuffer;				/* Where the send buffer is */
	wdsEntry	fds[MAX_FDS_ELEMS];		/* Free Data Structure list */
	exfds		exFDS[MAX_FDS_ELEMS];	/* exFDS entries */
	short		maxFDSused;				/* Max of the FDS's that have been used */
//	int			mseg;					/* BYU 2.4.15 */
//	int			service;				/* BYU 2.4.15 */
	short		portType;				/* UDATA, PDATA, PFTP, CNXN, NO_TYPE */
	Ptr			edata;					/* Ptr to Encryption data */
	} StreamRec, *StreamRPtr;

typedef struct MyTCPpb {				// 106 Bytes
	TCPiopb		pb;
	long		SavedA5;
	} MyTCPpb, *MyTCPpbPtr;
