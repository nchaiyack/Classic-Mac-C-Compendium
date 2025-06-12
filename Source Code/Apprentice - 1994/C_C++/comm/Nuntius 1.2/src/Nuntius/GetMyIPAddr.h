/* 
	GetMyIPAddr.h	
	C definitions of parameter block entries needed for IP calls

    Copyright Apple Computer, Inc. 1989 
    All rights reserved
    Modifications by Steve Falkenburg, Apple MacDTS 8/91
	
*/

/* conditional #include -->SJF<-- */

#ifndef __GETMYIPADDR__
#define __GETMYIPADDR__

/* added for misc. definitions -->SJF<-- */
#ifndef __MACTCPCOMMONTYPES__
#include <MacTCPCommonTypes.h>
#endif


#define ipctlGetAddr        15          /* csCode to get our IP address */

/* changed from ParamBlockHeader -->SJF<-- */

#define GetIPParamBlockHeader   \
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

struct GetAddrParamBlock {                                      /* changed from IPParamBlock -->SJF<-- */
    GetIPParamBlockHeader;          /* standard I/O header */   /* changed from ParamBlockHeader -->SJF<-- */
	ip_addr	ourAddress;			/* our IP address */
	long	ourNetMask;			/* our IP net mask */
	};
    
#endif      /* conditional include -->SJF<-- */
