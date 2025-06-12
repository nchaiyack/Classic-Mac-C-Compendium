/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for routines to make MacTCP more palatable
 ************************************************************************/
int MyTcpCreate(long *stream,UPtr buffer,int bSize,TCPNotifyProc asr);
int MyTcpOpen(long stream,long addr,short port,Byte toAction,Byte timeout);
int MyTcpSend(long stream,short count,UPtr buffer,int bSize, va_list extra);
short MyTcpRefN(short refN);
int MyTcpRecv(long stream,UPtr buffer,int *bSize,Byte timeout);
int MyTcpClose(long stream);
int MyTcpRelease(long stream);
int MyHostid(ip_addr *addr, long *mask);
int MyTcpSendWDS(long stream,wdsEntry *theWDS);
int MyTcpStatus(long stream,TCPiopb *pb);
int MyTcpAbort(long stream);
