/*
 * iplow.h
 *
 * ANSI Headers for iplow.c
 *
 * Mike Trent 8/94
 *
 */

#ifndef __IPLOW_HEADER__
#define __IPLOW_HEADER__

#ifndef OSErr
#include <Types.h>
#endif

/*					*/
/* GENERIC ROUTINES */
/*					*/


OSErr LowInitMacTCP (void);
void LowSetSpin (Spin spinRoutine);
void LowSpin(void);

OSErr LowStringToAddr(char *name, unsigned long *iplong);
OSErr LowAddrToName(unsigned long ipNum, char *name);
OSErr LowGetMyIP (unsigned long *ipNum);


/*					*/
/*   UDP ROUTINES   */
/*					*/

OSErr LowUDPCreate (StreamPtr *stream, unsigned short *port, mac_socket *sockets);
OSErr LowUDPRead (StreamPtr stream, char timeout, Ptr buf, int *buflen, 
					unsigned long *remoteHost, unsigned short *remotePort);
OSErr LowUDPWrite (StreamPtr stream, Ptr wdsPtr, unsigned long remoteHost, 
					unsigned short remotePort);
OSErr LowUDPRelease (StreamPtr stream);
OSErr LowUDPMTU (StreamPtr, unsigned long remoteHost, unsigned short *MTU);


/*					*/
/*   TCP ROUTINES   */
/*					*/

void InitTCPPB (TCPiopb *pb);
OSErr LowTCPCreate (StreamPtr *stream, mac_socket *sockets);
OSErr LowTCPPassiveOpen (StreamPtr stream, char timeout,
	unsigned long *remoteHost, unsigned short *remotePort, unsigned long *localHost,
	unsigned short *localPort);
OSErr LowTCPActiveOpen (StreamPtr stream, char timeout,
	unsigned long remHost, unsigned short remotePort, unsigned long *localHost,
	unsigned short *localPort);
OSErr LowTCPSend (StreamPtr stream, char timeout, Ptr wdsPtr);
OSErr LowTCPRcv (StreamPtr stream, char timeout, Ptr rbuf, int *buflen);
OSErr LowTCPClose (StreamPtr stream, char timeout);
OSErr LowTCPAbort(StreamPtr stream);
OSErr LowTCPRelease(StreamPtr stream);
OSErr LowTCPSelect (StreamPtr stream, int *ans);

#endif /* __IPLOW_HEADER__ */