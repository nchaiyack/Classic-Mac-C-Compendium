/* MTPStuff - Implementation of the MTP client routines                       */

#include "MTPStuff.h"


		// � Toolbox
#ifndef __ERRORS__
#include "Errors.h"
#endif

#ifndef ipBadLapErr
#include "MacTCPCommonTypes.h"
#endif

#ifndef __RESOURCES__
#include "Resources.h"
#endif

#ifndef __STDIO__
#include "StdIO.h"
#endif

#ifndef TCPCreate
#include "TCPPB.h"
#endif


		// � Implementation use
#ifndef __FILTERS__
#include "Filters.h"
#endif

#ifndef __GLOBALS__
#include "Globals.h"
#endif


struct mtpheader
{
	unsigned char player[20];		/* name of the wizard */
	unsigned char passwd[15];		/* password */
	unsigned char filename[50];		/* name of the datafile */
	unsigned char directory[20];	/* directory containing datafile */
	unsigned short filelength;		/* length of datablock */
	short flags;					/* controlflags */
};

const int OVERWRITE = 1;
const int RECEIVE = 2;
const int VERSION_TWO = 4;

struct mtpmessage
{
	char message[200];
	short quit;
};

struct WDSBuffer
{
	unsigned short length;
	unsigned char *ptr;
	short endMarker;
};


//------------------------------------------------------------------------------

const int kTCPBufferSize = 8 * 1024;
const int kTmpBufferSize = 2 * 1024;
const int kMTPTimeout = 30;
const int kMaxFileSize = 65535;

//------------------------------------------------------------------------------

static long tcpStream;
static Ptr tcpBuffer;
static short dRef, rRef;

//------------------------------------------------------------------------------

pascal void OpenConn (long tcpAddr, short tcpPort);
pascal long OpenStream ();
pascal void ReceiveData (unsigned char *buf, long len);
pascal void SendData (unsigned char *buf, long len);
pascal void SendFile (TMUDDoc *theDoc, short fRef);
pascal void ShowError (unsigned char *msg, TMUDDoc *aDoc);
pascal void ShowMsg (mtpmessage *msg, TMUDDoc *aDoc);

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void CloseStream ()
{
	TCPiopb pb;

	if (tcpStream != 0) {
		pb.ioNamePtr = NULL;
		pb.ioCRefNum = gTCPRef;
		pb.tcpStream = tcpStream;
		pb.csCode = TCPRelease;
		PBControl ((ParamBlockRec *) &pb, FALSE);
	}
	if (tcpBuffer != NULL) {
		DisposIfPtr (tcpBuffer);
		tcpBuffer = NULL;
	}
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void CopyP2CStr (unsigned char *aPStr, unsigned char *aCStr)
{
	int len;

	len = *aPStr++;
	for (int i = 0; i < len; i++) *aCStr++ = *aPStr++;
	*aCStr = 0;
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal long GetFileLength (TMUDDoc *theDoc, short fRef)
{
	long length, l, count, pos;
	unsigned char buffer[kTmpBufferSize];
	
	FailOSErr (GetEOF (fRef, &length));
	if (theDoc->fCvtTab) {
		l = length;
		length = 0;
		pos = 0;
		while (l > 0) {
			if (l > kTmpBufferSize)
				count = kTmpBufferSize;
			else
				count = l;
			FailOSErr (FSRead (fRef, &count, (Ptr) buffer));
			for (int i = 0; i < count; i ++) {
				if ((buffer [i] == chReturn) || (buffer [i] == chLineFeed)) {
					length += pos + 1;
					pos = 0;
				} else if (buffer [i] == chTab) {
					pos += theDoc->fSTab;
					pos -= pos % theDoc->fSTab;
				} else
					pos++;
			}
			l -= count;
		}
		FailOSErr (SetFPos (fRef, fsFromStart, 0));
	}
	return length;
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void MTPBuildPath (TMUDDoc *theDoc, AppFile *aFile, Str255 path)
{
	WDPBRec	pb;
	CInfoPBRec	ipb;
	short vRefNum;
	long dirID;
	Str255 name;
	
	pb.ioNamePtr = NULL;
	pb.ioVRefNum = aFile->vRefNum;
	pb.ioWDIndex = 0;
	pb.ioWDProcID = 0;
	pb.ioWDVRefNum = 0;
	FailOSErr (PBGetWDInfo (&pb, FALSE));
	vRefNum = pb.ioWDVRefNum;
	dirID = pb.ioWDDirID;
	CopyStr255 (aFile->fName, path);
	if (vRefNum == theDoc->fMTPVRefNum) {
		ipb.dirInfo.ioNamePtr = name;
		while ((dirID != 2) && (dirID != theDoc->fMTPDirID)) {
			ipb.dirInfo.ioVRefNum = vRefNum;
			ipb.dirInfo.ioFDirIndex = -1;
			ipb.dirInfo.ioDrDirID = dirID;
			FailOSErr (PBGetCatInfo (&ipb, FALSE));
			PStrCat (name, (unsigned char *) "\p/");
			PStrCat (name, path);
			CopyStr255 (name, path);
			dirID = ipb.dirInfo.ioDrParID;
		}
		if ((path [0] >= 50) || (dirID != theDoc->fMTPDirID))
			CopyStr255 (aFile->fName, path);
	}
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void MTPDirCheck (TMUDDoc *theDoc)
{
	Str255 tmp;
	ParamBlockRec pb;
	OSErr err;
	CInfoPBRec ipb;
	Boolean found;

	CopyStr255 (theDoc->fMTPVol, tmp);
	if (tmp [tmp [0] + 1] != ':') tmp [++(tmp [0])] = ':';
	pb.volumeParam.ioNamePtr = tmp;
	pb.volumeParam.ioVRefNum = 0;
	pb.volumeParam.ioVolIndex = -1;
	err = PBGetVInfo (&pb, FALSE);
	if (err != noErr) {
		MTPDirSetup (theDoc, gDefVRefNum, gDefDirID);
		found = FALSE;
	} else {
		theDoc->fMTPVRefNum = pb.volumeParam.ioVRefNum;
		CopyStr255 (pb.volumeParam.ioNamePtr, theDoc->fMTPVol);
		ipb.dirInfo.ioNamePtr = tmp;
		ipb.dirInfo.ioVRefNum = theDoc->fMTPVRefNum;
		ipb.dirInfo.ioDrDirID = theDoc->fMTPDirID;
		ipb.dirInfo.ioFDirIndex = -1;
		err = PBGetCatInfo (&ipb, FALSE);
		if ((err == dirNFErr) || (err == fnfErr)) {
			MTPDirSetup (theDoc, gDefVRefNum, gDefDirID);
			found = FALSE;
		} else {
			FailOSErr (err);
			CopyStr255 (ipb.dirInfo.ioNamePtr, theDoc->fMTPDir);
			found = TRUE;
		}
	}
	if (!found) {
		StdAlert (phNoMTPDirID);
		theDoc->fChangeCount += 1;
	}
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void MTPDirSetup (TMUDDoc *theDoc, short vRefNum, long dirID)
{
	Str255 tmp;
	CInfoPBRec pb;
	OSErr err;
	short refNum;
	
	theDoc->fMTPVRefNum = vRefNum;
	theDoc->fMTPDirID = dirID;
	theDoc->fMTPVol [0] = 0;
	theDoc->fMTPDir [0] = 0;
	pb.dirInfo.ioNamePtr = tmp;
	pb.dirInfo.ioVRefNum = theDoc->fMTPVRefNum;
	pb.dirInfo.ioDrDirID = 2;
	pb.dirInfo.ioFDirIndex = -1;
	err = PBGetCatInfo (&pb, FALSE);
	if (err == dirNFErr) {
		FailOSErr (GetVol ((StringPtr) tmp, &refNum));
		theDoc->fMTPVRefNum = refNum;
		theDoc->fMTPDirID = 2;
		CopyStr255 (tmp, theDoc->fMTPVol);
		CopyStr255 (tmp, theDoc->fMTPDir);
	} else
		FailOSErr (err);
	CopyStr255 (pb.dirInfo.ioNamePtr, theDoc->fMTPVol);
	pb.dirInfo.ioNamePtr = tmp;
	pb.dirInfo.ioVRefNum = theDoc->fMTPVRefNum;
	pb.dirInfo.ioDrDirID = theDoc->fMTPDirID;
	pb.dirInfo.ioFDirIndex = -1;
	err = PBGetCatInfo (&pb, FALSE);
	if (err == dirNFErr) {
		theDoc->fMTPDirID = 2;
		CopyStr255 (theDoc->fMTPVol, theDoc->fMTPDir);
	} else
		FailOSErr (err);
	CopyStr255 (pb.dirInfo.ioNamePtr, theDoc->fMTPDir);
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void MTPReceiveHandler (short , long , void * )
{
	CloseStream ();
	CloseFile (dRef, rRef);
}

pascal void MTPReceive (TMUDDoc *theDoc, Str255 path, Str255 fileName,
		short vRefNum)
{
	long length, rest, count;
	FailInfo fi;
	unsigned char hello[1];
	mtpheader header;
	mtpmessage msg;
	unsigned char buffer[kTmpBufferSize];
	
	tcpStream = 0;
	tcpBuffer = NULL;
	FailOSErr (Create (fileName, vRefNum, theDoc->fTextSig, 'TEXT'));
	FailOSErr (MAOpenFile (fileName, vRefNum, TRUE, FALSE, fsWrPerm, fsRdPerm,
		&dRef, &rRef));
	CatchFailures (&fi, MTPReceiveHandler, kDummyLink);
	tcpStream = OpenStream ();
	OpenConn (theDoc->fTCPAddr, theDoc->fMTPPort);
	hello[0] = '@';
	SendData (hello, 1);
	header.filelength = 0;
	header.flags = OVERWRITE + RECEIVE + VERSION_TWO;
	CopyP2CStr (theDoc->fPlayer, header.player);
	CopyP2CStr (theDoc->fPasswd, header.passwd);
	CopyP2CStr (path, header.filename);
	header.directory [0] = 0;
	SendData ((unsigned char *) &header, sizeof (header));
	ReceiveData ((unsigned char *) &msg, sizeof (msg));
	ShowMsg (&msg, theDoc);
	ReceiveData ((unsigned char *) &header, sizeof (header));
	length = header.filelength;
	rest = length;
	while (rest > 0) {
		if (rest > kTmpBufferSize)
			count = kTmpBufferSize;
		else
			count = rest;
		ReceiveData (buffer, count);
		rest -= count;
		for (int i = 0; i < count; i++)
			if (buffer [i] == chLineFeed) buffer [i] = chReturn;
		FailOSErr (FSWrite (dRef, &count, (Ptr) &buffer));
	}
	FailOSErr (SetEOF (dRef, length));
	ReceiveData ((unsigned char *) &msg, sizeof (msg));
	ShowMsg (&msg, theDoc);
	Success (&fi);
	CloseStream ();
	FailOSErr (CloseFile (dRef, rRef));
	FailOSErr (FlushVol (NULL, vRefNum));
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void MTPSendHandler (short , long , void * )
{
	CloseStream ();
	CloseFile (dRef, rRef);
}

pascal void MTPSend (TMUDDoc *theDoc, AppFile *theFile, Str255 path)
{
	long length;
	FailInfo fi;
	unsigned char hello[1];
	mtpheader header;
	mtpmessage msg;
	Str255 tmp;
	
	tcpStream = 0;
	tcpBuffer = NULL;
	FailOSErr (MAOpenFile (theFile->fName, theFile->vRefNum, TRUE, FALSE,
		fsRdPerm, fsRdPerm, &dRef, &rRef));
	CatchFailures (&fi, MTPSendHandler, kDummyLink);
	length = GetFileLength (theDoc, dRef);
	if ((length < 0) || (length > kMaxFileSize)) {
		ShowError ((unsigned char *) "MTP error: file too long.\12", theDoc);
		Failure (0, msgCancelled);
	}
	tcpStream = OpenStream ();
	OpenConn (theDoc->fTCPAddr, theDoc->fMTPPort);
	hello[0] = '@';
	SendData (hello, 1);
	header.filelength = (unsigned short) length;
	header.flags = OVERWRITE + VERSION_TWO;
	CopyStr255 (theDoc->fPlayer, tmp);
	if (tmp [0] > 19) tmp [0] = 19;
	CopyP2CStr (tmp, header.player);
	CopyStr255 (theDoc->fPasswd, tmp);
	if (tmp [0] > 14) tmp [0] = 14;
	CopyP2CStr (tmp, header.passwd);
	CopyP2CStr (path, header.filename);
	header.directory [0] = 0;
	SendData ((unsigned char *) &header, sizeof (header));
	ReceiveData ((unsigned char *) &msg, sizeof (msg));
	ShowMsg (&msg, theDoc);
	if (msg.quit == 1) Failure (0, msgCancelled);
	SendFile (theDoc, dRef);
	ReceiveData ((unsigned char *) &msg, sizeof (msg));
	ShowMsg (&msg, theDoc);
	Success (&fi);
	CloseStream ();
	FailOSErr (CloseFile (dRef, rRef));
	FailOSErr (FlushVol (NULL, theFile->vRefNum));
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void OpenConn (long tcpAddr, short tcpPort)
{
	TCPiopb pb;

	pb.ioNamePtr = NULL;
	pb.ioCRefNum = gTCPRef;
	pb.tcpStream = tcpStream;
	pb.csCode = TCPActiveOpen;
	pb.csParam.open.ulpTimeoutValue = kMTPTimeout;
	pb.csParam.open.ulpTimeoutAction = 1;
	pb.csParam.open.validityFlags = timeoutValue + timeoutAction;
	pb.csParam.open.remoteHost = tcpAddr;
	pb.csParam.open.remotePort = tcpPort;
	pb.csParam.open.localPort = 0;
	pb.csParam.open.tosFlags = 0;
	pb.csParam.open.precedence = 0;
	pb.csParam.open.dontFrag = FALSE;
	pb.csParam.open.timeToLive = 0;
	pb.csParam.open.security = 0;
	pb.csParam.open.optionCnt = 0;
	FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal long OpenStream ()
{
	TCPiopb pb;

	tcpBuffer = NewPermPtr (kTCPBufferSize);
	FailNIL (tcpBuffer);
	pb.ioNamePtr = NULL;
	pb.ioCRefNum = gTCPRef;
	pb.csCode = TCPCreate;
	pb.csParam.create.rcvBuff = tcpBuffer;
	pb.csParam.create.rcvBuffLen = kTCPBufferSize;
	pb.csParam.create.notifyProc = 0;
	FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
	return pb.tcpStream;
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void PStrCat (unsigned char *str, unsigned char *src)
{
	unsigned char *s2;
	short len;

	s2 = str + *str;
	len = *src;
	if (*str + len > 255) len = 255 - *str;
	*str += len;
	for (++len; --len; s2 [len] = src [len]);
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void ReceiveData (unsigned char *buf, long len)
{
	TCPiopb pb;
	unsigned short count;

	count = (unsigned short) len;
	while (count > 0) {
		pb.ioNamePtr = NULL;
		pb.ioCRefNum = gTCPRef;
		pb.tcpStream = tcpStream;
		pb.csCode = TCPRcv;
		pb.csParam.receive.commandTimeoutValue = kMTPTimeout;
		pb.csParam.receive.rcvBuff = (Ptr) buf;
		pb.csParam.receive.rcvBuffLen = count;
		pb.csParam.receive.rdsPtr = NULL;
		pb.csParam.receive.rdsLength = 0;
		FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
		count -= pb.csParam.receive.rcvBuffLen;
		buf += pb.csParam.receive.rcvBuffLen;
	}
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void SendData (unsigned char *buf, long len)
{
	TCPiopb pb;
	WDSBuffer wds;

	pb.ioNamePtr = NULL;
	pb.ioCRefNum = gTCPRef;
	pb.tcpStream = tcpStream;
	pb.csCode = TCPSend;
	pb.csParam.send.ulpTimeoutValue = kMTPTimeout;
	pb.csParam.send.ulpTimeoutAction = 1;
	pb.csParam.send.validityFlags = timeoutValue + timeoutAction;
	pb.csParam.send.pushFlag = 0;
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr) &wds;
	pb.csParam.send.sendFree = 0;
	pb.csParam.send.sendLength = 0;
	pb.csParam.send.userDataPtr = NULL;
	wds.length = (unsigned short) len;
	wds.ptr = buf;
	wds.endMarker = 0;
	FailOSErr (PBControl ((ParamBlockRec *) &pb, FALSE));
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void SendFile (TMUDDoc *theDoc, short fRef)
{
	long length, l, count, pos;
	int oc, i;
	unsigned char inBuf [kTmpBufferSize], outBuf [kTmpBufferSize];

	FailOSErr (GetEOF (fRef, &length));
	l = length;
	pos = 0;
	oc = 0;
	while (l > 0) {
		if (l > kTmpBufferSize)
			count = kTmpBufferSize;
		else
			count = l;
		FailOSErr (FSRead (fRef, &count, (Ptr) inBuf));
		i = 0;
		while (i < count) {
			if ((inBuf [i] == chReturn) || (inBuf [i] == chLineFeed)) {
				outBuf [oc++] = chLineFeed;
				i++;
				pos = 0;
			} else if ((inBuf [i] == chTab) && (theDoc->fCvtTab)) {
				outBuf [oc++] = ' ';
				pos++;
				if ((pos % theDoc->fSTab) == 0) i++;
			} else {
				outBuf [oc++] = inBuf [i];
				i++;
				pos++;
			}
			if (oc >= kTmpBufferSize) {
				SendData (outBuf, oc);
				oc = 0;
			}
		}
		l -= count;
	}
	if (oc > 0) SendData (outBuf, oc);
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void ShowError (unsigned char *msg, TMUDDoc *aDoc)
{
	long len;
	
	for (len = 0; msg [len] != 0; len++) ;
	aDoc->fEchoFilter->Filter (msg, len);
}

//------------------------------------------------------------------------------

#pragma segment SMTPStuff

pascal void ShowMsg (mtpmessage *msg, TMUDDoc *aDoc)
{
	long len;
	
	for (len = 0; msg->message [len] != 0; len++) ;
	aDoc->fEchoFilter->Filter ((unsigned char *) msg->message, len);
}

//------------------------------------------------------------------------------
