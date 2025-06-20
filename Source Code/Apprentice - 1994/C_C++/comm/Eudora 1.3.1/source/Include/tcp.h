/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for dealing with tcp streams
 ************************************************************************/
int TCPConnectTrans(UPtr serverName, short port,Boolean silently);
int TCPSendTrans(short count, UPtr text,long size, ...);
int TCPRecvTrans(UPtr line,long *size);
int TCPDisTrans(void);
int TCPTransError(void);
int TCPSendWDS(wdsEntry *theWDS);
void TCPSilenceTrans(Boolean silence);
short ClearICMP(void);
int TCPDestroyTrans(void);
int GetTCPStatus(TCPiopb *pb);
UPtr TCPWhoAmI(Uptr who);
int GetHostByAddr(struct hostInfo *hi, long addr);

#ifndef SLOW_CLOSE
void TcpFastFlush(Boolean destroy);
typedef struct FastPB
{
	TCPiopb pb;
	Str63 buffer;
	Ptr streamBuffer;
	struct FastPB *next;
} FastPB,*FastPBPtr;
#endif

typedef struct
{
	Handle next;
	struct hostInfo hi;
} HostInfoQ, *HostInfoQPtr, **HostInfoQHandle;

