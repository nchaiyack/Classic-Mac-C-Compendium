/*
	IP.h
	
	Header file for IP.c
	
	01/30/94 dn - Created
	
*/

#pragma once

#ifndef __H_IP__
#define __H_IP__

#include <IPPB.h>

// error codes
enum {
	ipBaseErr=23250,			// base error value for this section
	ipNilRefNum				// the IPiopb ioCRefNum was nil.
};

// macros for allocating new blocks...
#define Newnbp_entryPtr() NEWPTR(nbp_entryPtr,sizeof(nbp_entry))
#define Disposenbp_entryPtr(a) DISPOSEPTR(a)
#define Newnbp_entryHdl() NEWHDL(nbp_entryHdl,sizeof(nbp_entry))
#define Disposenbp_entryHdl(a) DISPOSEHDL(a)

#define NewEnet_addrPtr() NEWPTR(Enet_addrPtr,sizeof(Enet_addr))
#define DisposeEnet_addrPtr(a) DISPOSEPTR(a)
#define NewEnet_addrHdl() NEWHDL(Enet_addrHdl,sizeof(Enet_addr))
#define DisposeEnet_addrHdl(a) DISPOSEHDL(a)

#define Newarp_entryPtr() NEWPTR(arp_entryPtr,sizeof(arp_entry))
#define Disposearp_entryPtr(a) DISPOSEPTR(a)
#define Newarp_entryHdl() NEWHDL(arp_entryHdl,sizeof(arp_entry))
#define Disposearp_entryHdl(a) DISPOSEHDL(a)

#define NewLAPStatsPtr() NEWPTR(LAPStatsPtr,sizeof(LAPStats))
#define DisposeLAPStatsPtr(a) DISPOSEPTR(a)
#define NewLAPStatsHdl() NEWHDL(LAPStatsHdl,sizeof(LAPStats))
#define DisposeLAPStatsHdl(a) DISPOSEHDL(a)

#define NewIPEchoPtr() NEWPTR(IPEchoPBPtr,sizeof(IPEchoPB))
#define DisposeIPEchoPtr(a) DISPOSEPTR(a)
#define NewIPEchoHdl() NEWHDL(IPEchoPBHdl,sizeof(IPEchoPB))
#define DisposeIPEchoHdl(a) DISPOSEHDL(a)

#define NewLAPStatsPBPtr() NEWPTR(LAPStatsPBPtr,sizeof(LAPStatsPB))
#define DisposeLAPStatsPBPtr(a) DISPOSEPTR(a)
#define NewLAPStatsPBHdl() NEWHDL(LAPStatsPBHdl,sizeof(LAPStatsPB))
#define DisposeLAPStatsPBHdl(a) DISPOSEHDL(a)

#define NewIPParmBlkPtr() NEWPTR(IPParmBlkPtr,sizeof(IPParamBlock))
#define DisposeIPParmBlkPtr(a) DISPOSEPTR(a)
#define NewIPParmBlkHdl() NEWHDL(IPParmBlkHdl,sizeof(IPParamBlock))
#define DisposeIPParmBlkHdl(a) DISPOSEHDL(a)

#define NewICMPEchoInfoPtr() NEWPTR(ICMPEchoInfoPtr,sizeof(ICMPEchoInfo))
#define DisposeICMPEchoInfoPtr(a) DISPOSEPTR(a)
#define NewICMPEchoInfoHdl() NEWHDL(ICMPEchoInfoHdl,sizeof(ICMPEchoInfo))
#define DisposeICMPEchoInfoHdl(a) DISPOSEHDL(a)

#define NewICMPParmBlkPtr() NEWPTR(ICMPParmBlkPtr,sizeof(ICMPParmBlk))
#define DisposeICMPParmBlkPtr(a) DISPOSEPTR(a)
#define NewICMPParmBlkHdl() NEWHDL(ICMPParmBlkHdl,sizeof(ICMPParmBlk))
#define DisposeICMPParmBlkHdl(a) DISPOSEHDL(a)

#ifdef __cplusplus
extern "C" {
#endif

OSErr IPEchoICMP(IPiopb* pb,Boolean async);
OSErr IPLAPStats(IPiopb* pb,Boolean async);

#ifdef __cplusplus
}
#endif

#endif /* __H_IP__ */
