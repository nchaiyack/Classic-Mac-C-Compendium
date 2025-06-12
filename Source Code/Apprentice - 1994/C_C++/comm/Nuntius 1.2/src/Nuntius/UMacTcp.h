// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UMacTCP.h

#define __UMACTCP__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

#include "TCPPB.h"

extern short gMacTcpRefNum;

//typedef wdsEntry *wdsEntryPtr;
//typedef rdsEntry *rdsEntryPtr;

class TProgress;

void InitUMacTCP();
void CloseDownUMacTCP();
pascal void KillMacTCPStuff(); // called from installed patch

long MyIP();
long DotName2IP(const CStr255 &name);
void IP2DotName(long ip, CStr255 &name);
void GetMyDotName(CStr255 &myDotName);

class PTcpStream : public PPtrObject
{
	public:
		// If any of these are aborted with Failure(0, 0) by TThread,
		// the object should be disposed as the connection is aborted
		void SendData(void *bufferP, long len);
		void GetDataPointer(void *&p, long &bytesReceived);
		void ReceiveSomeData(void *bufferP, long bufferSize, long &bytesReceived);
		
		void OpenConnection(long hostIP, short port); // includes stream
		void CloseConnection(Boolean async = true);
		
		PTcpStream();
		void ITcpStream(long timeoutSecs);
		~PTcpStream();
	private:
		Boolean fConnectionIsOpen;
		Boolean fStreamIsOpen;
		Ptr fStreamBufferP;
		TCPiopb fStreamPB;
		TCPiopb fOpenPB, fClosePB, fAbortPB;
		TCPiopb fReceivePB, fSendPB;
		TCPiopb fStatusPB;
		StreamPtr fStreamP;
		wdsEntry fWdsEntry[5];
		long fTimeoutTicks, fTimeoutSecs;
		long fHostIP;
		short fHostPort;
		long fMacTCPID;

		// only TCPOpenConnection, read&write methods are async
		
		void TCPCreateStream();
		void TCPReleaseStream();
		
		void TCPOpenConnection();
		void TCPCloseConnection(Boolean async);
		void TCPAbortConnection();
		void TCPCloseOrAbortConnection(Boolean async);
		void TCPSendData();
		long TCPNumBytesAvailable();
		void TCPReceiveData(void *bufferP, unsigned short bufferSize,
											short &bytesReceived);

		void WaitForCompletion(TCPiopb &pb);
		void DebugCheckWds();
};
