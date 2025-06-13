
#ifndef __TCPLIB__
#define __TCPLIB__

#ifndef __MACTCPCOMMONTYPES__
#include <MacTCPCommonTypes.h>
#endif
#ifndef __TCPPB__
#include <TCPPB.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define	WDS(bufCount) struct {				\
	wdsEntry			block[bufCount];	\
	unsigned short		zero;				\
}

StreamPtr	tcpCreate(int bufSize, TCPNotifyUPP asr, Ptr usrData);
OSErr		tcpOpen(StreamPtr stream, ip_addr host, tcp_port port);
OSErr		tcpOpenReserved(StreamPtr stream, ip_addr host, tcp_port port);
OSErr		tcpRead(StreamPtr stream, void* buffer, short* len);
OSErr		tcpWrite(StreamPtr stream, void* buffer, short len);
OSErr		tcpWriteWDS(StreamPtr stream, wdsEntry* wds);
OSErr		tcpClose(StreamPtr stream);
OSErr		tcpStatus(StreamPtr stream, TCPiopb* pb);

#ifdef __cplusplus
}
#endif

#endif
