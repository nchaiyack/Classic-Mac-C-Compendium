/*----------------------------------------------------------------------------

	ftp.c

	This handle all transactions with FTP servers.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"

#include "glob.h"
#include "dlgutil.h"
#include "ftp.h"
#include "tcp.h"
#include "util.h"


static unsigned long gControlConnectionId;
static unsigned long gDataConnectionId;
static Ptr gBuffer;
static Boolean gAlreadyGot226;


/* Abort aborts a transaction with an FTP server. */

static void Abort (void)
{
	if (gControlConnectionId != 0) {
		AbortConnection(gControlConnectionId);
		ReleaseStream(gControlConnectionId);
	}
	if (gDataConnectionId != 0) {
		AbortConnection(gDataConnectionId);
		ReleaseStream(gDataConnectionId);
	}
	MyDisposPtr(gBuffer);
}


/* Init initializes a transaction with an FTP server. */

static Boolean Init (char *host, char *user, char *pswd,
	char *file, char *cmd)
{
	CStr255 sendData[4];
	unsigned long addr;
	unsigned long myAddr;
	unsigned short length;
	OSErr err;
	TCPiopb *pBlock;
	unsigned short localPort;
	Ptr p,pEnd;
	char *serverCommand = nil;
	
	gControlConnectionId = gDataConnectionId = 0;
	gBuffer = nil;
	pBlock = nil;
	
	/* Get host address. */

	if (IPNameToAddr(host, &addr) != noErr) {
		if (err != -1) ErrorMessage("Could not get host address.");
		goto exit3;
	}
	
	/* Allocate data buffer. */
	
	gBuffer = MyNewPtr(kBufLen);
	if ((err = MyMemErr()) != noErr) goto exit1;
	
	/* Create and open control stream connecton. */
	
	if ((err = CreateStream(&gControlConnectionId,kBufLen)) != noErr) goto exit1;
	if ((err = OpenConnection(gControlConnectionId,addr,kFTPPort,20)) != noErr) {
		if (err != -1) ErrorMessage("Could not open connection to host.");
		goto exit3;
	}
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	/* Send USER command. */
	
	strcpy(sendData[0],"USER ");
	strcpy(sendData[1],user);
	strcpy(sendData[2],CRLF);
	if ((err = SendMultiData(gControlConnectionId,sendData,3)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '3') goto exit4;
	
	/* Send PASS command. */
	
	strcpy(sendData[0],"PASS ");
	strcpy(sendData[1],pswd);
	strcpy(sendData[2],CRLF);
	if ((err = SendMultiData(gControlConnectionId,sendData,3)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit4;
	
	/* Create and open data stream connection. */
	
	if ((err = CreateStream(&gDataConnectionId,kBufLen)) != noErr) goto exit1;
	if ((err = AsyncWaitForConnection(gDataConnectionId,0,0,0,0,&pBlock)) !=
		noErr) goto exit1;
		
	/* Wait for MacTCP to assign port number. */
		
	while ((localPort = pBlock->csParam.open.localPort) == 0 && GiveTime());
	if (gCancel) goto exit3;

	/* Send PORT command. */
	
	if ((err = GetMyIPAddr(&myAddr)) != noErr) goto exit1;
	sprintf(sendData[0],"PORT %hu,%hu,%hu,%hu,%hu,%hu",
		(unsigned short)((myAddr>>24)&0xff), 
		(unsigned short)((myAddr>>16)&0xff), 
		(unsigned short)((myAddr>>8)&0xff), 
		(unsigned short)(myAddr&0xff),
		(unsigned short)((localPort>>8)&0xff), 
		(unsigned short)(localPort&0xff));
	strcpy(sendData[1],CRLF);
	serverCommand = "PORT";
	if ((err = SendMultiData(gControlConnectionId,sendData,2)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	/* Send RETR or STOR command. */
	
	strcpy(sendData[0],cmd);
	strcpy(sendData[1]," ");
	strcpy(sendData[2],file);
	strcpy(sendData[3],CRLF);
	serverCommand = cmd;
	if ((err = SendMultiData(gControlConnectionId,sendData,4)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '1') goto exit2;
	
	/* If the timing is just right, it is possible that the final 226 "data transfer
	   complete" message arrived as part of the buffer just received. We must check 
	   for this. */
	   
	p = gBuffer;
	pEnd = gBuffer+length-3;
	gAlreadyGot226 = false;
	while (p < pEnd ) {
		if (*p == CR && *(p+1) == LF) {
			p += 2;
			length = pEnd - p;
			if (length < 3 || *p != '2') {
				MailOrFTPServerErrorMessage(p);
				goto exit3;
			}
			gAlreadyGot226 = true;
			break;
		}
		p++;
	}
	
	/* Wait for server to open its end of the data stream connection. */
	
	while (pBlock->ioResult > 0 && GiveTime());
	err = gCancel ? -1 : pBlock->ioResult;
	if (err != noErr) goto exit1;
	MyDisposPtr((Ptr)pBlock);
	
	return true;
	
exit1:

	UnexpectedErrorMessage(err);
	goto exit3;
	
exit2:

	MailOrFTPServerErrorMessage(gBuffer);
	
exit3:

	Abort();
	MyDisposPtr((Ptr)pBlock);
	return false;
	
exit4:

	ErrorMessage("Invalid username or password.");
	goto exit3;
}


/* Term terminates a transaction with an FTP server. */

static Boolean Term (char *cmd, Boolean get)
{
	CStr255 commStr;
	unsigned short length;
	OSErr err;
	char *serverCommand;
	
	/* Close data stream. */
	
	if ((err = CloseConnection(gDataConnectionId, get)) != noErr) goto exit1;
	if ((err = ReleaseStream(gDataConnectionId)) != noErr) goto exit1;
	
	/* Check for final 226 "data transfer complete" reply to RETR or STOR command. */
	
	serverCommand = cmd;
	if (!gAlreadyGot226) {
		length = kBufLen;
		if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
		if (length < 3 || *gBuffer != '2') goto exit2;
	}
	
	/* Send QUIT command on control stream. */
	
	strcpy(commStr,"QUIT");
	strcat(commStr,CRLF);
	serverCommand = "QUIT";
	if ((err = SendData(gControlConnectionId,commStr,6)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(gControlConnectionId,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	/* Close control stream. */

	if ((err = CloseConnection(gControlConnectionId, true)) != noErr) goto exit1;
	if ((err = ReleaseStream(gControlConnectionId)) != noErr) goto exit1;

	/* Dispose the buffer. */
	
	MyDisposPtr(gBuffer);
	return true;
	
exit1:

	UnexpectedErrorMessage(err);
	goto exit3;
	
exit2:

	MailOrFTPServerErrorMessage(gBuffer);
	
exit3:

	Abort();
	return false;
}


/* FTPGetFile gets a file from a host. */	
	
Boolean FTPGetFile (char *host, char *user, char *pswd, char *file, Handle data)
{
	unsigned short length;
	long size, allocated;
	OSErr err;
	Ptr p,pEnd,q;

	if (!Init(host,user,pswd,file,"RETR")) return false;
	
	MySetHandleSize(data, kBufLen);
	size = 0;
	allocated = kBufLen;
	
	while (true) {
		length = kBufLen;
		if ((err = RecvData(gDataConnectionId,gBuffer,&length,true)) != noErr) break;
		if (size + length > allocated) {
			allocated += kBufLen;
			MySetHandleSize(data, allocated);
		}
		BlockMove(gBuffer, *data + size, length);
		size += length;
	}
	if (err != connectionClosing && err != connectionTerminated) goto exit1;
	
	p = q = *data;;
	pEnd = *data + size;
	while (p < pEnd) {
		if (*p == CR && *(p+1) == LF) {
			*q++ = CR;
			p += 2;
		} else {
			*q++ = *p++;
		}
	}
	size = q - *data;
	MySetHandleSize(data, size + 1);
	*(*data+size) = 0;

	return Term("RETR", true);
	
exit1:

	UnexpectedErrorMessage(err);
	Abort();
	return false;
}


/* FTPPutFile sends a file to a host. */	
	
Boolean FTPPutFile (char *host, char *user, char *pswd, char *file, Ptr data, long size)
{
	OSErr err;
	Ptr p,pEnd,q,qEnd;

	if (!Init(host,user,pswd,file,"STOR")) return false;
	
	p = data;
	pEnd = data + size;
	while (p < pEnd) {
		q = gBuffer;
		qEnd = q + kBufLen - 1;
		while (p < pEnd && q < qEnd) {
			if (*p == CR) {
				*q++ = CR;
				*q++ = LF;
				p++;
			} else {
				*q++ = *p++;
			}
		}
		if (q > gBuffer) {
			if ((err = SendData(gDataConnectionId,gBuffer,q-gBuffer)) != noErr) goto exit1;
		}
	}
	
	return Term("STOR", false);
	
exit1:

	UnexpectedErrorMessage(err);
	Abort();
	return false;
}
