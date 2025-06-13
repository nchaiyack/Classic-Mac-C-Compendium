/*----------------------------------------------------------------------------

	ftp.c

	This handle all transactions with FTP servers.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"

#include "glob.h"
//#include "dlgutil.h"
#include "ftp.h"
#include "tcp.h"
#include "util.h"
#include "DLOG129.h"

Boolean 	gCancel = false;			/* flag set when user cancels an action */
short 		gMemError;
Handle		gLifeBoat;					/* lifeboat memory -- de-allocated when 
										   memory gets low */
Boolean		gSinking = true;			/* flag set after lifeboat has been 
										   jettisoned */
Boolean 	gOutOfMemory = false;		/* flag set when out of memory - and luck */
static char	gStatusMsg[256];

static unsigned long gControlConnectionId;
static unsigned long gDataConnectionId;
static Ptr gBuffer;
static Boolean gAlreadyGot226;



//----------------------------------------------------------------
	void UnexpectedErrorMessage(short s)
//----------------------------------------------------------------
{
	Alert(143,nil);
}

//----------------------------------------------------------------
	void ErrorMessage(char *s)
//----------------------------------------------------------------
{
	c2pstr(s);
	ParamText((StringPtr)s,"\p","\p","\p");
	Alert(130,nil);
}

//----------------------------------------------------------------
	void MailOrFTPServerErrorMessage(char *s)
//----------------------------------------------------------------
{
	Alert(139,nil);
}	
	
/*	MyIOCheck can be called to display the result of a routine returning
	an OSErr.  If the value in err is zero, the routine simply terminates.
*/
/*	IsAppWindow returns true if the window belongs to the application
*/

//----------------------------------------------------------------
	Boolean IsAppWindow (WindowPtr wind)
//----------------------------------------------------------------
{
	short		windowKind;
	
	if (wind == nil)
		return false;
	else {
		windowKind = ((WindowPeek)wind)->windowKind;
		return windowKind >= userKind;
	}
}

//----------------------------------------------------------------
	Boolean IsStatusWindow (WindowPtr wind)
//----------------------------------------------------------------
{
	TWindow **info;

	if (!IsAppWindow(wind)) return false;
	info = (TWindow**)GetWRefCon(wind);
	return (**info).kind==kStatus;
}
/*	StatusWindow displays a movable-modal status window indicating
	the current state of the program.
*/

//----------------------------------------------------------------
	Boolean StatusWindow (char *text)
//----------------------------------------------------------------
{
	WindowPtr statusWind;
	Rect bounds = {0,0,75,420};
	Rect buttonBounds = {42,347,62,407};
	Rect msgBounds = {13, 13, 40, 420};
	TWindow **info;
	
	if (!IsStatusWindow(statusWind = FrontWindow())) {
			OffsetRect(&bounds,100,100);
		info = (TWindow**) MyNewHandle(sizeof(TWindow));
		if (MyMemErr() != noErr)
			return false;
		strcpy(gStatusMsg,text);
		(**info).kind = kStatus;
		statusWind = NewWindow(nil, &bounds, "\pStatus", true,
			movableDBoxProc, (WindowPtr)-1,
			false, (unsigned long)info);
		SetPort(statusWind);
		NewControl(statusWind, &buttonBounds, "\pCancel", true, 0, 0, 0, 
			pushButProc, 0);
		ValidRect(&statusWind->portRect);
		SpinCursor(0);
	} else {
		SetPort(statusWind);
		strcpy(gStatusMsg,text);
		EraseRect(&msgBounds);
	}
	TextFont(systemFont);
	TextSize(12);
	MoveTo(13,29);
	DrawText(gStatusMsg,0,strlen(gStatusMsg));
	return true;
}


/*	UpdateStatus is called in response to update events for the status
	window.  This routine will redraw sections of the window as necessary.
*/

//----------------------------------------------------------------
	void UpdateStatus (void)
//----------------------------------------------------------------
{
	GrafPtr savePort;
	WindowPtr statusWind;
	
	statusWind = FrontWindow();
	if (!IsStatusWindow(statusWind)) return;
	GetPort(&savePort);
	SetPort(statusWind);
	EraseRect(&statusWind->portRect);
	DrawControls(statusWind);
	TextFont(systemFont);
	TextSize(12);
	MoveTo(13,29);
	DrawText(gStatusMsg,0,strlen(gStatusMsg));
	SetPort(savePort);
}


/*	CloseStatusWindow is called when the status window should be removed.
*/

//----------------------------------------------------------------
	void CloseStatusWindow (void)
//----------------------------------------------------------------
{
	WindowPtr statusWind;
	GrafPtr savePort;
	TWindow **info;
	
	if (IsStatusWindow(statusWind = FrontWindow())) {
		//SetPt(&gPrefs.statusWindowLocn,0,0);
		GetPort(&savePort);
		SetPort(statusWind);
		//LocalToGlobal(&gPrefs.statusWindowLocn);
		info = (TWindow**)GetWRefCon(statusWind);
		SetPort(savePort);
		MyDisposHandle((Handle)info );
		DisposeWindow(statusWind);
	}
}

//----------------------------------------------------------------
	OSErr MyIOCheck (OSErr err)
//----------------------------------------------------------------
{
	if (err != noErr) {
		UnexpectedErrorMessage(err);
	}
	return err;
}


/*	LowMemory is called when the program runs out of useable memory.
	If this is the first time this has happened, the program de-allocates
	lifeboat memory which was allocated when the program was launched.
	Otherwise, the user had better quit.
*/

//----------------------------------------------------------------
	static Boolean LowMemory (void)
//----------------------------------------------------------------
{
	Boolean result;
	
	if (MyMemErr() != memFullErr) {
		MyIOCheck(MyMemErr());
		return false;
	}
		
	if (gSinking) {
		result = false;
		gOutOfMemory = true;
		ErrorMessage("You have run out of memory");
	}
	else {
		HUnlock(gLifeBoat);
		DisposHandle(gLifeBoat);
		gSinking = true;
		result = true;
		ErrorMessage("Memory is getting low.  Some operations may fail.");
	}
	return result;
}


/*	This is a wrapper for the NewPtr routine which automatically checks
	the result of the call and takes appropriate action.
*/

//----------------------------------------------------------------
	Ptr MyNewPtr (Size byteCount)
//----------------------------------------------------------------
{
	Ptr thePtr;
	
	thePtr = NewPtrClear(byteCount);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			thePtr = MyNewPtr(byteCount);
		else
			thePtr = nil;
	}
	return thePtr;
}


/*	This is a wrapper for the NewHandle routine which automatically checks
	the result of the call and takes appropriate action.
*/

//----------------------------------------------------------------
	Handle MyNewHandle (Size byteCount)
//----------------------------------------------------------------
{
	Handle theHndl;
	
	theHndl = NewHandleClear(byteCount);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			theHndl = MyNewHandle(byteCount);
		else
			theHndl = nil;
	}
	return theHndl;
}

/*	This is a wrapper for the SetHandleSize routine which automatically checks
	the result of the call and takes appropriate action.
*/

//----------------------------------------------------------------
	void MySetHandleSize (Handle h, Size newSize)
//----------------------------------------------------------------
{
	long oldSize;

	oldSize = GetHandleSize(h);
	SetHandleSize(h,newSize);
	if ((gMemError = MemError()) != noErr) {
		if (LowMemory())
			MySetHandleSize(h,newSize);
	} else if (oldSize < newSize) {
		memset(*h+oldSize, 0, newSize-oldSize);
	}
}


/*	This is a wrapper for the HandToHand routine which automatically checks
	the result of the call and takes appropriate action.
*/

//----------------------------------------------------------------
	OSErr MyHandToHand (Handle *theHndl)
//----------------------------------------------------------------
{
	Handle oldHndl;
	OSErr result;
	
	oldHndl = *theHndl;
	result = gMemError = HandToHand(theHndl);
	if (result != noErr) {
		*theHndl = oldHndl;
		if (LowMemory())
			result = MyHandToHand(theHndl);
	}
	return result;
}


/*	This is a wrapper for the DisposPtr routine which automatically checks
	the result of the call and takes appropriate action.
	NT: extra errorchecking for stability incorporated
*/

//----------------------------------------------------------------
	OSErr MyDisposPtr (Ptr thePtr)
//----------------------------------------------------------------
{
	if(thePtr != nil)
	{
		DisposPtr(thePtr);
		gMemError = MemError();
	}
	else
	{
		gMemError = noErr;
	}
	return MyIOCheck(gMemError);
}


/*	This is a wrapper for the DisposHandle routine which automatically checks
	the result of the call and takes appropriate action.
	NT: extra errorchecking for stability incorporated
*/

//----------------------------------------------------------------
	OSErr MyDisposHandle (Handle theHndl)
//----------------------------------------------------------------
{
	if(theHndl != nil)
	{
		DisposHandle(theHndl);
		gMemError = MemError();
	}
	else
	{
		gMemError = noErr;
	}
	return MyIOCheck(gMemError);
}


/*	This is a wrapper for the MemError routine which automatically checks
	the result of the call and takes appropriate action.
*/

OSErr MyMemErr (void)
{
	return gMemError;
}

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
/*	The InitCursorCtl, SpinCursor, strcasecmp and strncasecmp functions 
	are missing in Think, so we include them here	*/

Handle gAcur = nil;

pascal void InitCursorCtl (Handle id)
{
	short NoFrames, i, CursId;
	CursHandle	TheCursHndl;

	if (id == nil) {
		gAcur = (Handle) GetResource('acur',0);
		if(gAcur == nil) return;
		HLock(gAcur);
		NoFrames = ((short *)(*gAcur))[0];
		for(i = 0; i < NoFrames; i++) {
			CursId = ((short *)(*gAcur))[(2*i)+2];
			TheCursHndl = GetCursor(CursId);
			((CursHandle *)(*gAcur))[i+1] = TheCursHndl;
			HLock((Handle)TheCursHndl);
		}
	} else {
		gAcur = id;
		HLock(gAcur);
		NoFrames = ((short *)(*gAcur))[0];
		for(i = 0; i < NoFrames; i++) {
			CursId = ((short *)(*gAcur))[(2*i)+2];
			TheCursHndl = GetCursor(CursId);
			((CursHandle *)(*gAcur))[i+1] = TheCursHndl;
			HLock((Handle)TheCursHndl);
		}
	}
	((short *)(*gAcur))[1] = 0;
	DetachResource(gAcur);
}


pascal void SpinCursor (short num)
{
	short	NoFrames, CurrentFrame, CurrentCounter;
	Cursor	CurrentCursor;

	if (gAcur == nil) InitCursorCtl(nil);
	NoFrames = ((short *)(*gAcur))[0];
	CurrentCounter = ((((short *)(*gAcur))[1] + num) % (NoFrames * 32));
	((short *)(*gAcur))[1] = CurrentCounter;
	CurrentFrame = CurrentCounter / 32;
	CurrentCursor = **(((CursHandle *)(*gAcur))[CurrentFrame+1]);
	SetCursor(&CurrentCursor);
}

/*	GiveTime is called whenever the application is waiting for a slow
	process to complete.  The routine calls SpinCursor and WaitNextEvent
	to give time to currently running background applications.
*/
void DrawWindow(WindowPtr w);
void DrawWindow(WindowPtr w)
{
	extern char host[256];
	extern char user[256];	
	SetPort(w);
	
	MoveTo(5,20);
	DrawString("\pHost: ");
	DrawText(host,0,strlen(host));
	MoveTo(5,40);
	DrawString("\pUserName: ");
	DrawText(user,0,strlen(user));
}

void HandleUpdate (WindowPtr wind)
{
	GrafPtr savePort;
	
	if (((WindowPeek)wind)->windowKind < 0 || 
		((WindowPeek)wind)->windowKind == dialogKind) return;
	GetPort(&savePort);
	SetPort(wind);
	BeginUpdate(wind);
	if (!IsStatusWindow(wind)) {
		EraseRect(&wind->portRect);
		DrawWindow(wind);
	} else {
		UpdateStatus();
	}
	EndUpdate(wind);
	SetPort(savePort);
}

Boolean GiveTime (void)
{
	EventRecord ev;
	Boolean gotEvt;
	static long nextTime = 0;
	char keyPressed, charPressed;
	WindowPtr statusWind;
	GrafPtr savePort;
	ControlHandle cancel;
	long myticks;
	short part;
	WindowPtr theWindow;

	HiliteMenu(0);
	//SetMenusTo(false, 0, 0, 0, 0, 0, 0);
	ShowCursor();

	if (TickCount() >= nextTime) {
			
		SpinCursor(16);

		gotEvt = WaitNextEvent(everyEvent,&ev,0,nil);
		
		if ( gotEvt )
			switch (ev.what) {
				case mouseDown:
					part = FindWindow(ev.where, &theWindow);
					if (part == inMenuBar) {
						MenuSelect(ev.where);
					} else if (IsStatusWindow(FrontWindow())) {
						//HandleMouseDown(&ev);
					}
					break;
				case activateEvt:
					//HandleActivate((WindowPtr)ev.message,((ev.modifiers & activeFlag) != 0)); 
					break;
				case updateEvt:
					HandleUpdate((WindowPtr)(ev.message));
					break;
				case app4Evt:
					//HandleSuspendResume(ev.message);
					break;
				case keyDown:
				case autoKey:
					FlushEvents(keyDownMask+keyUpMask+autoKeyMask,0);
					charPressed = ev.message & charCodeMask;
					keyPressed = (ev.message & keyCodeMask) >> 8;
					if (keyPressed == escapeKeyCode || 
						(ev.modifiers & cmdKey) != 0 && charPressed == '.') {
						statusWind = FrontWindow();
						if (IsStatusWindow(statusWind)) {
							cancel = ((WindowPeek)statusWind)->controlList;
							GetPort(&savePort);
							SetPort(statusWind);
							HiliteControl(cancel,1);
							Delay(8,&myticks);
							HiliteControl(cancel,0);
							SetPort(savePort);
						}
						gCancel = true;
					}
					break;
			}
		nextTime = TickCount() + 5;
	}

	return !gCancel;
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

	if (host[0] == 0 ||user[0] == 0 || pswd[0] == 0 ) {
		if (DoPrefs(0)==0)
			return false;
	}
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
