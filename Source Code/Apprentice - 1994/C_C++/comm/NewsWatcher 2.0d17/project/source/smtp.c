/*----------------------------------------------------------------------------

	smtp.c

	This module handles all transactions with SMTP servers.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"

#include "glob.h"
#include "dlgutil.h"
#include "smtp.h"
#include "tcp.h"
#include "util.h"
#include "log.h"


/* global variables */

static char *gBuffer;     /* RecvData buffer */



/*  GetSMTPAddr is called to get the mail server IP address,
	given its name from the preferences file.
*/

static Boolean GetSMTPAddr (unsigned long *addr)
{
	OSErr err;
	
	p2cstr(gPrefs.mailServerName);
	err = IPNameToAddr((char*)gPrefs.mailServerName, addr);
	c2pstr((char*)gPrefs.mailServerName);
	if (err != noErr) {
		if (err != -1) ErrorMessage("Could not get mail server address.");
		return false;
	}
	return true;
}


/*	RcptMsg determines the recipients of the message and sends commands
	to the SMTP server specifying these people as recipients.
*/

static Boolean RcptMsg (char *text, unsigned short tLength, 
	unsigned long stream, char *header)
{
	Ptr current,current2,current3,lineEnd,textEnd;
	CStr255 sendData[4];
	unsigned short length,hdrLen,len;
	Boolean foundHeader;
	OSErr err;
	
	strcpy(sendData[0],"RCPT TO:<");
	strcpy(sendData[2],">");
	strcpy(sendData[3],CRLF);

	/* Find the header line. */
	
	current = text;
	hdrLen = strlen(header);
	textEnd = text + tLength;
	foundHeader = false;
	while (!foundHeader && current < textEnd && *current != CR) {
		if (strncasecmp(current,header,hdrLen) == 0) {
			foundHeader = true;
		} else {
			while (current < textEnd && *current != CR) current++;
			current += 2;
		}
	}
	if (!foundHeader) return true;
	
	/* Find the end of the header line. */
	
	current += hdrLen;
	if (current >= textEnd) return true;
	for (lineEnd = current; lineEnd < textEnd && *lineEnd != CR; lineEnd++);
	if (lineEnd >= textEnd) return true;
	
	/* Send a RCPT TO command for each address in the header line.
	   Addresses are delimited by commas, and may be in any of the
	   forms specified in RFC 1036:
	   
	   		addr
			addr (name)
			name <addr>
	*/
	
	while (current < lineEnd) {
		while (current < lineEnd && *current == ' ') current++;
		current2 = current;
		while (current2 < lineEnd && *current2 != ',') current2++;
		current3 = current2 - 1;
		while (current3 > current && *current3 == ' ') current3--;
		if (current3 > current) {
			if (*current3 == ')') {
				current3--;
				while (current3 > current && *current3 != '(') current3--;
				current3--;
				while (current3 > current && *current3 == ' ') current3--;
			} else if (*current3 == '>') {
				current3--;
				while (current3 > current && *current3 == ' ') current3--;
				while (current < current3 && *current != '<') current++;
				current++;
				while (current < current3 && *current == ' ') current++;
			}
		}
		len = current3 - current + 1;
		if (len > 0 && len < 255) {
			strncpy(sendData[1], current, len);
			sendData[1][len] = 0;
			if ((err = SendMultiData(stream,sendData,4)) != noErr) goto exit1;
			length = kBufLen;
			if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
			if (length < 3 || *gBuffer != '2') goto exit2;
		}
		current = current2 + 1;
	}
	
	return true;

exit1:

	UnexpectedErrorMessage(err);
	return false;
	
exit2:

	MailOrFTPServerErrorMessage(gBuffer);
	return false;
	
}


/*	SendSMTP sends a message through e-mail by contacting the local
	SMTP server and sending the mail.
*/

Boolean SendSMTP (char *text,unsigned short tLength)
{
	unsigned long stream = 0;
	unsigned short length;
	CStr255 sendData[4];
	unsigned long addr;
	CStr255 commStr;
	OSErr err;
	char *serverCommand = nil;
	
	gBuffer = nil;
	gBuffer = NewPtrClear(kBufLen);
	if ((err = MyMemErr()) != noErr) goto exit1;
	
	if (!GetSMTPAddr(&addr)) goto exit1;
		
	if ((err = CreateStream(&stream,kBufLen)) != noErr) goto exit1;
	if ((err = OpenConnection(stream,addr,kSMTPPort,20)) != noErr) {
		if (err != -1) ErrorMessage("Could not open connection to mail server.");
		goto exit3;
	}
		
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	strcpy(sendData[0],"HELO ");
	if ((err = GetMyIPName(sendData[1])) != noErr) {
		if ((err = GetMyIPAddrStr(sendData[1])) != noErr) goto exit1;
	}
	strcpy(sendData[2],CRLF);
	serverCommand = "HELO";
	if ((err = SendMultiData(stream,sendData,3)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	strcpy(sendData[0],"MAIL FROM:<");
	strcpy(sendData[1],gPrefs.address);
	strcpy(sendData[2],">");
	strcpy(sendData[3],CRLF);
	serverCommand = "MAIL FROM";
	if ((err = SendMultiData(stream,sendData,4)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	if (!RcptMsg(text,tLength,stream,"To: ")) goto exit3;
	if (!RcptMsg(text,tLength,stream,"Cc: ")) goto exit3;
	if (!RcptMsg(text,tLength,stream,"Bcc: ")) goto exit3;
	
	strcpy(commStr,"DATA");
	strcat(commStr,CRLF);
	serverCommand = "DATA";
	if ((err = SendData(stream,commStr,6)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '3') goto exit2;

	if (gPrefs.logActionsToFile) LogMailMessage(text, tLength);
	if ((err = SendData(stream,text,tLength)) != noErr) goto exit1;
	strcpy(commStr,CRLF);
	strcat(commStr,".");
	strcat(commStr,CRLF);
	if ((err = SendData(stream,commStr,5)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	strcpy(commStr,"QUIT");
	strcat(commStr,CRLF);
	serverCommand = "QUIT";
	if ((err = SendData(stream,commStr,6)) != noErr) goto exit1;
	length = kBufLen;
	if ((err = RecvData(stream,gBuffer,&length,true)) != noErr) goto exit1;
	if (length < 3 || *gBuffer != '2') goto exit2;
	
	if ((err = CloseConnection(stream, true)) != noErr) goto exit1;
	if ((err = ReleaseStream(stream)) != noErr) goto exit1;
	MyDisposPtr(gBuffer);
	return true;
	
exit1:

	UnexpectedErrorMessage(err);
	goto exit3;
	
exit2:

	MailOrFTPServerErrorMessage(gBuffer);

exit3:

	if (stream != 0) {
		AbortConnection(stream);
		ReleaseStream(stream);
	}
	MyDisposPtr(gBuffer);
	return false;
}
