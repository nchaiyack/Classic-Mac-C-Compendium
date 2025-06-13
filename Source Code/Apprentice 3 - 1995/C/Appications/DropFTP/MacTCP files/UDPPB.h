/* 
	UDPPB.h	
	C definitions of parameter block entries needed for UDP calls

    Copyright Apple Computer, Inc. 1988-91 
    All rights reserved
		
*/

#ifndef __UDPPB__
#define __UDPPB__

#ifndef __MACTCPCOMMONTYPES__
#include <MacTCPCommonTypes.h>
#endif

#define UDPCreate		20
#define UDPRead			21
#define UDPBfrReturn	22
#define UDPWrite		23
#define UDPRelease		24
#define UDPMaxMTUSize	25
#define UDPStatus		26
#define UDPMultiCreate	27
#define UDPMultiSend	28
#define UDPMultiRead	29
#define UDPCtlMax		29

typedef enum UDPEventCode {
	UDPDataArrival = 1,
	UDPICMPReceived,
	lastUDPEvent = 32767
	} UDPEventCode;

#ifdef THINK_C

	typedef ProcPtr UDPNotifyProc;
	
#else

	typedef pascal void (*UDPNotifyProc) (
			StreamPtr udpStream, 
			unsigned short eventCode, 
			Ptr userDataPtr,
			struct ICMPReport *icmpMsg);
#endif

typedef void (*UDPIOCompletionProc) (struct UDPiopb *iopb);

typedef	unsigned short	udp_port;

typedef struct UDPCreatePB {			/* for create and release calls */
	Ptr 			rcvBuff;
	unsigned long	rcvBuffLen;
	UDPNotifyProc	notifyProc;
	unsigned short	localPort;
	Ptr				userDataPtr;
	udp_port		endingPort;
} UDPCreatePB;
	
typedef struct UDPSendPB {
	unsigned short	reserved;
	ip_addr			remoteHost;
	udp_port		remotePort;
	Ptr				wdsPtr;
	Boolean			checkSum;	
	unsigned short	sendLength;
	Ptr				userDataPtr;
	udp_port		localPort;
} UDPSendPB;
	
typedef struct UDPReceivePB {		/* for receive and buffer return calls */
	unsigned short	timeOut;
	ip_addr			remoteHost;
	udp_port		remotePort;
	Ptr 			rcvBuff;
	unsigned short	rcvBuffLen;
	unsigned short	secondTimeStamp;
	Ptr		 		userDataPtr;
	ip_addr			destHost;		/* only for use with multi rcv */
	udp_port		destPort;		/* only for use with multi rcv */
} UDPReceivePB;

typedef struct UDPMTUPB {
	unsigned short 	mtuSize;
	ip_addr			remoteHost;
	Ptr				userDataPtr;
} UDPMTUPB;

typedef struct UDPiopb {
	char 				fill12[12];
	UDPIOCompletionProc	ioCompletion;
	short 				ioResult;
	char 				*ioNamePtr;		
	short 				ioVRefNum;		
	short				ioCRefNum;			
	short 				csCode;
	StreamPtr		 	udpStream;				
	union {
		struct UDPCreatePB	create;
		struct UDPSendPB	send;
		struct UDPReceivePB	receive;
		struct UDPMTUPB		mtu;
	} csParam;
} UDPiopb;
	
#endif