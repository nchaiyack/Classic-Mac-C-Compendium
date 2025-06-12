/*
*    Whatami.h
*  Machine specific information for NCSA Telnet TCP/IP kernel
*   Defines for TCP/IP library, are you a Mac or a PC?
*/

#ifndef MAC
#define PC 1
#define ETHER 1
#define UB 1 
#endif

#define ETHER

#ifndef ETHER
#define ATALK 1
#endif

/*
*   Defines which have to do with Ethernet addressing versus Appletalk
*   addressing.  Ethernet has 6 bytes of hardware address, ATALK has 4
*/
#ifdef ETHER
#define DADDLEN 6
#define WINDOWSIZE 4096
#define TSENDSIZE 512
#define DEFWINDOW 1024
#define DEFSEG	1024
#define TMAXSIZE 1024
#define UMAXLEN 1024
#define ICMPMAX 300 
#else
/*  define for AppleTalk */
#define DADDLEN 4
#define WINDOWSIZE 4096
#define TSENDSIZE 512
#define DEFWINDOW 512
#define DEFSEG	512
#define TMAXSIZE 512
#define UMAXLEN 512
#define ICMPMAX 300 
#endif


#ifdef PC
/*
*  define length of an integer
*/
typedef char int8;
typedef unsigned char uint8;
typedef int int16;
typedef long int int32;
typedef unsigned int uint16;
typedef unsigned long int uint32;


#define TICKSPERSEC 18
#define SMINRTO 5
#define WRAPTIME 86400L				/* in seconds, only for PC */
#define NPORTS	30
#define CONNWAITTIME 20				/* default contime in seconds */
uint32 longswap();

#else
/*
*  define length of an integer
*/

#define NFDEF {0,0,0}
#define NBDEF {65535,65535,65535}
#define BFDEF {0,61183,11060}
#define BBDEF {61183,2079,4938}
#define UFDEF {1,0,0}
#define UBDEF {0,0,0}

typedef char int8;
typedef unsigned char uint8;

#ifdef MPW
typedef short int int16;
typedef long int int32;
typedef unsigned int uint;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
#define ctop(x) x
#define ptoc(x) x
#define index(x,y) strchr(x,y)

#include <memory.h>
#define malloc(A) NewPtr(A)
#define free(A) DisposPtr((Ptr) A)

#else
typedef int int16;
typedef unsigned int uint;
typedef long int int32;
typedef unsigned int uint16;
typedef unsigned long int uint32;
#endif

#define TICKSPERSEC 60
#define SMINRTO 25
#define NPORTS	30
#define CONNWAITTIME 20				/* default contime in seconds */

#define	longswap(x) x
#define intswap(x) x
#ifndef MPW
#define movebytes(x,y,z) movmem(y,x,z)
#define movenbytes(x,y,z) movmem(y,x,z)
#else
#define movebytes(x,y,z) memcpy(x,y,z)
#define movenbytes(x,y,z) memcpy(x,y,z)
#endif 
#define n_putchar(x) putchar(x)
#define n_puts(x) putln(x)
#define NPORTS	30

#ifdef MPW
#define FALSE false
#define TRUE true
#endif MPW
#endif

#include "netevent.h"


