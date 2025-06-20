/*----------------------------------------------------------------------------

	nntp.c

	This module handles all transactions with the NNTP server.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include "AddressXlation.h"
#include "packages.h"

#include "glob.h"
#include "dlgutil.h"
#include "header.h"
#include "nntp.h"
#include "qsort.h"
#include "tcp.h"
#include "util.h"
#include "tcplow.h"
#include "menus.h"
#include "log.h"



#define kServerInfoDlg				141			/* Server info dialog */
#define kServerInfoTheInfoItem		3



static unsigned long	gNNTPAddr;			/* IP address of NNTP server */
static unsigned long	gConnID = 0;		/* connection id for nntp connection */
static char 			*gBuffer;			/* NNTP buffer */
static CStr255			gCurGroup;			/* current group on server */
static CStr255			gHello = "";		/* saved server "hello" message */
static Handle			gServerInfoText; 	/* handle to server info text */

/* The following are used to close idle news server connections asynchronously. */

typedef struct TQueuedClosingNewsConnection {
	TCPiopb pBlock;				/* PBControl param block */
	long myA5;					/* NewsWatcher's A5 register */
	long connID;				/* connection id */
	short ioCRefNum;			/* MacTCP ioCRefNum */
	Boolean destroy;			/* true if PBControl call error - destroy this connection */
	Boolean finished;			/* true when stream completely torn down and closed */
	char sendBuf[7];			/* buffer for sending "QUIT" command */
	char receiveBuf[256];		/* buffer for receiving final data from news server */
	struct wdsEntry wds[2];		/* WDS for sending "QUIT" command */
	struct TQueuedClosingNewsConnection *next;		/* pointer to next queue element */
	long startingTick;			/* tick count when close operation started (for testing) */
} TQueuedClosingNewsConnection;

static TQueuedClosingNewsConnection *gClosingNewsConnectionQueue = nil;  /* stream closing queue */

static unsigned long gLastNewsServerTransactionTime;	/* time of last transaction */



/*----------------------------------------------------------------------------
	InitMacTCP 
	
	Initialize MacTCP and allocate the NNTP buffer.
	
	Exit:	function result = true if MacTCP opened, false if error.
			gBuffer allocated.
----------------------------------------------------------------------------*/

static Boolean InitMacTCP (void)
{
	StatusWindow("Opening MacTCP.");
	if (InitNetwork() != noErr) {
		ErrorMessage("Could not open MacTCP.");
		return false;
	}
	gBuffer = MyNewPtr(kBufLen);
	return true;
}



/*----------------------------------------------------------------------------
	AbortNewsConnection 
	
	Aborts a connection to the remote NNTP server.
	
	Entry:	gConnID = connection id of NNTP stream.
	
	Exit:	function result = true if no error, false if error or canceled.
			gConnID = 0.
----------------------------------------------------------------------------*/

static void AbortNewsConnection (void)
{
	if (gConnID == 0) return;
	AbortConnection(gConnID);
	ReleaseStream(gConnID);
	gConnID = 0;
}



/*----------------------------------------------------------------------------
	GetNNTPAddr 
	
	Get the news server IP address, given its name from the preferences file.
	
	Exit:	function result = true if no error, false if error.
			gNNTPAddr = IP address of NNTP server.
----------------------------------------------------------------------------*/

static Boolean GetNNTPAddr (void)
{
	OSErr err;
	
	StatusWindow("Getting news server address.");
	p2cstr(gPrefs.newsServerName);
	err = IPNameToAddr((char*)gPrefs.newsServerName, &gNNTPAddr);
	c2pstr((char*)gPrefs.newsServerName);
	if (err != noErr) {
		ErrorMessage("Could not get news server address.");
		return false;
	}
	return true;
}



/*----------------------------------------------------------------------------
	ParseNum 
	
	Parse a number from a string.

	Entry:	*p = pointer to first char of number.

	Exit:	function result = parsed number, or -1 if error.
			*p = pointer to first non-white space char following number.
----------------------------------------------------------------------------*/

static long ParseNum (char **p)
{
	long num;
	char *endp;
	
	errno = 0;
	num = strtol(*p, &endp, 10);
	if (*p == endp || errno != 0) return -1;
	while (*endp == ' ' || *endp == '\t') endp++;
	*p = endp;
	return num;
}



/*----------------------------------------------------------------------------
	SendServerCommand 
	
	Send a command to the server.
	
	Entry:	gBuffer = 0-terminated command.
	
	Exit:	function result = true if no error, false if error.
	
	The command should be single line without a terminating CR or LF. This
	function appends the CRLF before sending the command to the server.
----------------------------------------------------------------------------*/

static Boolean SendServerCommand (void)
{
	OSErr err;

	strcat(gBuffer, CRLF);
	err = SendData(gConnID, gBuffer, strlen(gBuffer));
	GetDateTime(&gLastNewsServerTransactionTime);
	if (err == noErr) return true;
	UnexpectedErrorMessage(err);
	AbortNewsConnection();
	return false;
}



/*----------------------------------------------------------------------------
	GetShortServerResponse 
	
	Gets a one line response from the server.
	
	Exit:	function result = true if no error, false if error.
			gBuffer = server response.
			*length = length of server response.
			*serverCode = server response code.
----------------------------------------------------------------------------*/

static Boolean GetShortServerResponse (unsigned short *length, long *serverCode)
{
	OSErr err;
	unsigned short len;
	unsigned short next = 0;
	Boolean done = false;
	char *p;
	
	while (!done) {
		len = kBufLen - next;
		err = RecvData(gConnID, gBuffer + next, &len, true);
		GetDateTime(&gLastNewsServerTransactionTime);
		if (err != noErr) {
			UnexpectedErrorMessage(err);
			AbortNewsConnection();
			return false;
		}
		next += len;
		done = gBuffer[next-1] == LF;
	}
	*length = next;
	p = gBuffer;
	*serverCode = ParseNum(&p);
	return true;
}



/*----------------------------------------------------------------------------
	GetBigServerResponse 
	
	Gets a multi-line response from the server. The response is terminated
	with a final "." line.
	
	Exit:	function result = true if no error, false if error.
			*serverCode = server result code on first response line.
			*response = handle to response text.
			*responseLength = length of response.
			*numLinesReceived = number of lines received, not counting the
				initial server message line or the final "." line.
----------------------------------------------------------------------------*/

static Boolean GetBigServerResponse (long *serverCode,
	Handle *response, long *responseLength, short *numLinesReceived)
{
	Handle resp = nil;
	long respLen = 0;
	long respAlloc = kBufLen;
	unsigned short length;
	char *p, *pEnd;
	short numReceived = 0;
	OSErr err;
	Boolean start = true;

	resp = MyNewHandle(kBufLen);
	while (true) {
		length = kBufLen;
		err = RecvData(gConnID, gBuffer, &length, true);
		GetDateTime(&gLastNewsServerTransactionTime);
		if (err != noErr) {
			MyDisposHandle(resp);
			AbortNewsConnection();
			UnexpectedErrorMessage(err);
			return false;
		}
		if (respLen + length > respAlloc) {
			respAlloc += kBufLen;
			MySetHandleSize(resp, respAlloc);
		}
		BlockMove(gBuffer, *resp + respLen, length);
		respLen += length;
		pEnd = gBuffer + length;
		for (p = gBuffer; p < pEnd; p++) if (*p == LF) numReceived++;
		if (start && numReceived > 0) {
			HLock(resp);
			p = *resp;
			*serverCode = ParseNum(&p);
			HUnlock(resp);
			if (!(100 <= *serverCode && *serverCode <= 299)) break;
			start = false;
		}
		if (respLen >= 5) {
			p = *resp + respLen - 5;
			if (*p == CR && *(p+1) == LF && *(p+2) == '.' && 
				*(p+3) == CR && *(p+4) == LF) break;
		}
	}
	MySetHandleSize(resp, respLen);
	*response = resp;
	*responseLength = respLen;
	*numLinesReceived = numReceived - 2;
	return true;
}



/*----------------------------------------------------------------------------
	SendGroupCommand 
	
	Sends a GROUP command to the server.
	
	Entry:	groupName = name of group.
	
	Exit:	function result =
				0 if no error.
				1 if group does not exist or access is denied.
				2 if some other error.
			gBuffer = server response.
----------------------------------------------------------------------------*/

static short SendGroupCommand (char *groupName)
{
	unsigned short length;
	long serverCode;

	*gCurGroup = 0;
	strcpy(gBuffer,"GROUP ");
	strcat(gBuffer, groupName);
	if (gPrefs.logActionsToFile) LogGroupCommand(gBuffer);
	if (!SendServerCommand()) return 2;
	if (!GetShortServerResponse(&length, &serverCode)) return 2;
	if (gPrefs.logActionsToFile) LogGroupCommandResponse(gBuffer, length);
	GetDateTime(&gLastNewsServerTransactionTime);
	if (serverCode == 411 || serverCode == 502) return 1;
	if (serverCode == 211) {
		strcpy(gCurGroup, groupName);
		return 0;
	}
	NewsServerErrorMessage("GROUP", gBuffer);
	return 2;
}



/*----------------------------------------------------------------------------
	SetGroup 
	
	Sets the server group. A GROUP command is sent to the server only if 
	necessary.
	
	Entry:	groupName = name of group.
	
	Exit:	function result =
				0 if no error.
				1 if group does not exist.
				2 if some other error.
----------------------------------------------------------------------------*/

static short SetGroup (char *groupName)
{
	if (strcasecmp(gCurGroup, groupName) == 0) return 0;
	return SendGroupCommand(groupName);
}



/*----------------------------------------------------------------------------
	SendModeReader 
	
	Send "MODE READER" command (for INN).
	
	Exit:	function result = true if no error, false if error or canceled.
	
	Server error codes are not checked, just in case we might be talking to
	a non-INN server.
----------------------------------------------------------------------------*/

static Boolean SendModeReader (void)
{
	unsigned short length;
	long serverCode;

	strcpy(gBuffer,"MODE READER");
	if (!SendServerCommand()) return false;
	if (!GetShortServerResponse(&length, &serverCode)) return false;
	return true;
}



/*----------------------------------------------------------------------------
	GetHello 
	
	Gets the initial greeting message back from a newly opened NNTP connection.
	
	Exit:	function result = true if no error, false if error or canceled.
			gHello = saved server hello message.
----------------------------------------------------------------------------*/

static Boolean GetHello (void)
{
	unsigned short length;
	long serverCode;
		
	if (!GetShortServerResponse(&length, &serverCode)) return false;
	if (serverCode != 200 && serverCode != 201) {
		NewsServerErrorMessage(nil, gBuffer);
		return false;
	}
	gBuffer[length] = 0;
	if (length > 255) length = 255;
	while ((gBuffer[length-1] == CR || gBuffer[length-1] == LF) && length > 0) length--;
	gBuffer[length] = 0;
	strcpy(gHello, gBuffer);
	if (!gPrefs.noModeReader) return SendModeReader();
	return true;
}



/*----------------------------------------------------------------------------
	OpenNewsConnection 
	
	Open a connection to the remote NNTP server.
	
	Exit:	function result = true if no error, false if error or canceled.
			gConnID = connection id of NNTP stream.
----------------------------------------------------------------------------*/

static Boolean OpenNewsConnection (void)
{
	unsigned long recvLen;
	OSErr err;

	if (gCancel) return false;	
	*gCurGroup = 0;
	recvLen = kBufLen;
	err = CreateStream(&gConnID, recvLen);
	if (err != noErr) {
		if (gConnID != 0) ReleaseStream(gConnID);
		gConnID = 0;
		UnexpectedErrorMessage(err);
		return false;
	}
	err = OpenConnection(gConnID, gNNTPAddr, kNNTPPort, 30);
	if (err != noErr) {
		ReleaseStream(gConnID);
		gConnID = 0;
		if (err != -1) ErrorMessage("Could not open connection to news server.");
		return false;
	}
	return GetHello();
}



/*----------------------------------------------------------------------------
	CloseNewsConnection 
	
	Close a connection to the remote NNTP server.
	
	Entry:	gConnID = connection id of NNTP stream.
	
	Exit:	function result = true if no error, false if error or canceled.
			gConnID = 0.
----------------------------------------------------------------------------*/

static void CloseNewsConnection (void)
{
	unsigned short length;
	OSErr err;
	char *p;
	long serverCode;
	
	if (gConnID == 0) return;
	strcpy(gBuffer,"QUIT");
	strcat(gBuffer,CRLF);
	err = SendData(gConnID, gBuffer, strlen(gBuffer));
	if (err != noErr) goto exit;
	length = kBufLen;
	err = RecvData(gConnID, gBuffer, &length, true);
	if (err != noErr) goto exit;
	p = gBuffer;
	serverCode = ParseNum(&p);
	if (serverCode != 205) goto exit;
	err = CloseConnection(gConnID, true);
	if (err != noErr) goto exit;
	err = ReleaseStream(gConnID);	
	if (err != noErr) goto exit;
	gConnID = 0;
	return;
	
exit:
	AbortNewsConnection();
}



/*----------------------------------------------------------------------------
	ResetConnection 
	
	Checks to make sure that the nntp connection is still
	established. If not, it tries to reestablish the connection.
	
	Entry:	gConnID = connection id of NNTP stream.
	
	Exit:	function result = true if no error, false if error or canceled.
----------------------------------------------------------------------------*/

static Boolean ResetConnection (void)
{
	byte state = 0;
	
	if (gCancel) return false;
	if (gConnID != 0) {
		GetConnectionState(gConnID, &state);
		if (state == 8) return true;
		AbortNewsConnection();
	}
	return OpenNewsConnection();
}


/*----------------------------------------------------------------------------
	EstablishNewConnection 
	
	Closes and reopens the news server connection (for non-INN servers
	which only read the active file at the initial connection).

	Entry:	gConnID = connection id of old NNTP stream.
	
	Exit:	function result = true if no error, false if error or canceled.
			gConnID = connection id of new NNTP stream.
----------------------------------------------------------------------------*/

static Boolean EstablishNewConnection (void)
{
	/* If noNewConnection prefs set or startup time, don't close old connection - 
	   create a new one only if there is no current one. */

	if (gPrefs.noNewConnection || gStartingUp) return ResetConnection();
	
	/* Close old connection and open new one. */
	
	CloseNewsConnection();
	return OpenNewsConnection();
}



/*----------------------------------------------------------------------------
	StartNNTP 
	
	Initializes MacTCP and opens the NNTP connection.
	
	Exit:	function result = true if no error, false if error or canceled.
----------------------------------------------------------------------------*/

Boolean StartNNTP (void)
{
	OSErr err;
	
	if (!InitMacTCP()) return false;
	if (!GetNNTPAddr()) return false;
	StatusWindow("Opening news server connection.");
	if (!OpenNewsConnection()) return false;
	return true;
}



/*----------------------------------------------------------------------------
	EndNNTP 
	
	Closes the connection to the NNTP server.
----------------------------------------------------------------------------*/

void EndNNTP (void)
{
	if (gConnID != 0) {
		StatusWindow("Closing news server connection.");
		CloseNewsConnection();
	}
}



/*----------------------------------------------------------------------------
	GetGroupNames
	
	Gets a list of group names from the server and calls a processing 
	function for each group name in the list.
	
	Entry:	lastTime = 0: Fetch the entire full group list.
			lastTime != 0: Fetch just the groups which have been created
				since lastTime - 36 hours.
			func = pointer to function to call for each group.
	
	Exit:	function result = true if no error, false if error or canceled.
	
	The group processing function "func" should be declared as follows:
	
	Boolean func (const char *name)

	Entry: 	name = group name.
	
	Exit:	function result = true if no error, false if error or canceled.
	
	This function should call GiveTime to give time to other processes and
	check for user cancel operations.
----------------------------------------------------------------------------*/
 
Boolean GetGroupNames (unsigned long lastTime, NameFunc func)
{
	DateTimeRec timeRec;
	long expectedServerCode, serverCode;
	Handle response = nil;
	long responseLength;
	char *p, *q;
	short numLinesReceived;
	char *serverCommand;

	if (!ResetConnection()) return false;
	
	/* Build the server command. */

	if (lastTime == 0) {
	
		/* Get the whole list */
		
		strcpy(gBuffer, "LIST");
		serverCommand = "LIST";
		
	} else {
	
		/* Just get groups added since "lastTime". Subtract 36 hours to compensate 
		   for clock drift, daylight savings time, and server and client in 
		   different time zones. */
		   
		lastTime -= 60L*60L*36L;
		Secs2Date(lastTime, &timeRec);
		timeRec.year = timeRec.year % 100;	/* Only want last two digits */
		sprintf(gBuffer, "NEWGROUPS %02d%02d%02d %02d%02d%02d", 
				timeRec.year, timeRec.month, timeRec.day,
				timeRec.hour, timeRec.minute, timeRec.second);
		serverCommand = "NEWGROUPS";
		
	}
	
	/* Send the server command and get the server response. */

	if (!SendServerCommand()) return false;
	if (!GetBigServerResponse(&serverCode, &response, &responseLength, 
		&numLinesReceived)) return false;
	expectedServerCode = lastTime == 0 ? 215 : 231;
	if (serverCode != expectedServerCode) goto exit2;
	
	/* Process the response. */
	
	HLock(response);
	p = *response;
	while (numLinesReceived--) {
		while (*p != LF) p++;
		p++;
		q = p;
		while (*q != ' ' && *q != CR) q++;
		*q = 0;
		if (!(*func)(p)) goto exit1;
	}
	
	MyDisposHandle(response);
	return true;
	
exit1:
	MyDisposHandle(response);
	AbortNewsConnection();
	return false;
	
exit2:
	HLock(response);
	NewsServerErrorMessage(serverCommand, *response);
	MyDisposHandle(response);
	return false;
}



/*----------------------------------------------------------------------------
	GetArticle 
	
	Gets the full text of one article from the NNTP server.
	
	Entry:	groupName = name of group, or nil if fetching by message id.
			article = string containing article number or message id.
			
	Exit:	*text = handle to article text.
			*textLength = length of article text.
			function result =
				0 if no error.
				1 if article does not exist on server or group does not exist.
				2 if some other error.
				
	Backspace-underscore and underscore-backspace sequences are filtered out
	of the article, as are all non-printable characters except for TAB and CR.
	Trailing blank lines are deleted. CRLF line terminators are mapped to CR.
	Leading double-period characters on lines are mapped to single periods.
----------------------------------------------------------------------------*/

short GetArticle (char *groupName, char *article, Handle *text, long *textLength)
{
	short result;
	long serverCode;
	Handle response = nil;
	long responseLength;
	char *p, *pEnd, *q;
	short numLinesReceived;
	
	if (!ResetConnection()) return 2;
	
	/* Set group if fetching by article number. */
	
	if (groupName != nil) {
		result = SetGroup(groupName);
		if (result != 0) return result;
	}
	
	/* Send ARTICLE command. */
	
	strcpy(gBuffer, "ARTICLE ");
	strcat(gBuffer, article);
	if (!SendServerCommand()) return 2;
	
	/* Get server response. */
	
	if (!GetBigServerResponse(&serverCode, &response, 
		&responseLength, &numLinesReceived)) return 2;
	if (serverCode == 423 || serverCode == 430) {
		MyDisposHandle(response);
		return 1;
	}
	if (serverCode != 220) {
		HLock(response);
		NewsServerErrorMessage("ARTICLE", *response);
		MyDisposHandle(response);
		return 2;
	}
	
	/* Process server response. */
	
	p = *response;
	pEnd = *response + responseLength;
	q = *response;
	while (*p != LF) p++;	/* skip over initial 220 server response line */
	p++;
	while (p < pEnd) {
		if (*p == '.' && *(p+1) == '.' && *(p-1) == LF) {
			/* map ".." to "." at beginning of line. */
			*q++ = '.';
			p += 2;
		} else if (*p == '\b' && *(p+1) == '_') {
			/* filter out backspace-underscore */
			p += 2;
		} else if (*p == '_' && *(p+1) == '\b') {
			/* filter out underscore-backspace */
			p += 2;
		} else if ((*p >= ' ' && *p <= '~') || *p == CR || *p == '\t') {
			/* copy printable chars, CR, and TAB as is. */
			*q++ = *p++;
		} else {
			/* filter out other unprintable chars, including LF. */
			*p++;
		}
	}
	q -= 3;   /* trim trailing "." and CR */
	while (q > *response && *q == CR) q--;   /* trim trailing CR's */
	q++;
	responseLength = q - *response;
	
	/* Return. */
	
	if (responseLength <= 0) {
		MyDisposHandle(response);
		return 1;
	} else {
		MySetHandleSize(response, responseLength);
		*text = response;
		*textLength = responseLength;
		return 0;
	}
}



/*----------------------------------------------------------------------------
	PostArticle 
	
	Posts a news article.
	
	Entry:	text = pointer to new article text.
			textLength = length of text.
			
	Exit:	function result = true if no error, false if error or canceled.
	
	The caller must prepare the message text properly: It must begin with the
	full header, followed by an empty line, followed by the body. All lines
	must be terminated with CRLF. Periods at the beginnings of lines must
	be replaced by double-periods.
----------------------------------------------------------------------------*/

Boolean PostArticle (char *text, unsigned short textLength)
{
	unsigned short length;
	long serverCode;
	OSErr err;
	
	if (!ResetConnection()) return false;
	
	/* Send the POST command. */
	
	strcpy(gBuffer,"POST");
	if (!SendServerCommand()) return false;
	if (!GetShortServerResponse(&length, &serverCode)) return false;
	if (serverCode != 340) goto exit2;

	/* Send the article. */

	err = SendData(gConnID, text, textLength);
	GetDateTime(&gLastNewsServerTransactionTime);
	if (err != noErr) goto exit1;
	
	/* Send the terminating "." on a line by itself */

	strcpy(gBuffer, CRLF);
	strcat(gBuffer, ".");
	if (!SendServerCommand()) return false;
	if (!GetShortServerResponse(&length, &serverCode)) return false;
	if (serverCode != 240) goto exit2;

	return true;

exit1:
	UnexpectedErrorMessage(err);
	AbortNewsConnection();
	return false;

exit2:

	NewsServerErrorMessage("POST", gBuffer);
	return false;
}



/*----------------------------------------------------------------------------
	GetGroupArticleRange 
	
	Queries the NNTP server to get the current article range for a single group.
	
	Entry:	theGroup = pointer to group record.
	
	Exit:	function result =
				0 if no error.
				1 if group does not exist.
				2 if some other error.
			theGroup->firstMess = first article in range.
			theGroup->lastMess = last article in range.
			theGroup->numUnread = number of articles in range.
----------------------------------------------------------------------------*/
 
short GetGroupArticleRange (TGroup *theGroup)
{
	short result;
	char *p;
	CStr255 groupName;
	long first, last;

	if (!ResetConnection()) return 2;
	
	strcpy(groupName, *gGroupNames + theGroup->nameOffset);
	result = SendGroupCommand(groupName);
	if (result != 0) return result;

	p = gBuffer;
	ParseNum(&p);			/* Skip server response code */
	ParseNum(&p);			/* Skip number of articles estimate */
	first = ParseNum(&p);
	last = ParseNum(&p);
	if (first == 0 && last == 0) {
		/* Special case empty group: set firstMess = lastMess + 1. */
		theGroup->firstMess = theGroup->lastMess + 1;
	} else {
		if (first <= 0) first = 1;
		if (last < first) last = first - 1;
		theGroup->firstMess = first;
		theGroup->lastMess = last;
	} 
	theGroup->numUnread = theGroup->lastMess - theGroup->firstMess + 1;

	return 0;
}



/*----------------------------------------------------------------------------
	GetGroupArrayArticleRangesOneAtATime 
	
	Queries the NNTP server to get current article range info for designated 
	groups in a group list array. The GROUP commands are sent one at a time
	(not batched). This function is used for servers which do not support
	batched commands.

	Entry:	groupArray = handle to group array.
			numGroups = number of groups in group array.

			Each group in the array to be updated is marked with status='x'.

	Exit:	function result = true if no error, false if error or canceled.
			Deleted groups are marked with status='d'.
----------------------------------------------------------------------------*/

static Boolean GetGroupArrayArticleRangesOneAtATime (TGroup **groupArray, 
	short numGroups)
{
	TGroup *g, *gEnd;
	short result;
	
	if (gPrefs.logActionsToFile) LogGetGroupArrayArticleRangesOneAtATimeBegin();
	HLock((Handle)groupArray);
	g = *groupArray;
	gEnd = g + numGroups;
	while (g < gEnd) {
		if (g->status == 'x') {
			result = GetGroupArticleRange(g);
			if (result == 2) return false;
			if (result == 1) g->status = 'd';
		}
		g++;
	}
	HUnlock((Handle)groupArray);
	if (gPrefs.logActionsToFile) LogGetGroupArrayArticleRangesOneAtATimeEnd();
	
	return true;
}



/*----------------------------------------------------------------------------
	PartialGetGroupArrayArticleRanges 
	
	Queries the NNTP server to get current article range info for designated 
	groups in a group list array.

	Entry:	groupArray = handle to group array.
			numGroups = number of groups in group array.
			*firstGroup = index in array of first group to check.

			Each group in the array to be updated is marked with status='x'.

	Exit:	function result = true if no error, false if error or canceled.
			Deleted groups are marked with status='d'.
			*firstGroup = index in array of next group to check.

	For a large number of groups, this function may only check a partial
	collection of groups. In this case, the function should be called
	repeatedly until firstGroup == numGroups. The reason for doing this
	in pieces is because we cannot send more than 32K of data (GROUP commands)
	to the server at once.
----------------------------------------------------------------------------*/
 
static Boolean PartialGetGroupArrayArticleRanges (TGroup **groupArray, 
	short numGroups, short *firstGroup)
{
	long groupCmdsLen = 0;
	unsigned short len;
	short i;
	CStr255 groupName;
	CStr255 tmpStr;
	OSErr err;
	short numGroupsToCheck = 0;
	Handle response = nil;
	long responseLen = 0;
	char *p, *pEnd;
	TGroup *g;
	char *lineStart;
	long serverCode;
	char *gName;
	short fGroup;
	long first, last;
	long respAlloc = kBufLen;
	short numReceived = 0;
	Boolean start = true;
	
	/* Build the list of GROUP commands. */
	
	fGroup = *firstGroup;
	for (i = fGroup; i < numGroups; i++) {
		if ((*groupArray)[i].status == 'x') {
			strcpy(groupName, *gGroupNames + (*groupArray)[i].nameOffset);
			sprintf(tmpStr, "GROUP %s%s", groupName, CRLF);
			len = strlen(tmpStr);
			if (groupCmdsLen + len > kBufLen) break;
			BlockMove(tmpStr, gBuffer + groupCmdsLen, len);
			groupCmdsLen += len;
			numGroupsToCheck++;
		}
	}
	*firstGroup = i;
	
	/* Send the GROUP commands to the server. */
	
	err = SendData(gConnID, gBuffer, groupCmdsLen);
	if (err != noErr) goto exit1;
	if (gPrefs.logActionsToFile) 
		LogBatchGroupCommandSend(gBuffer, groupCmdsLen, numGroupsToCheck);
	
	/* Read the responses from the server. */

	response = MyNewHandle(kBufLen);
	while (numReceived < numGroupsToCheck) {
		len = kBufLen;
		err = RecvData(gConnID, gBuffer, &len, true);
		GetDateTime(&gLastNewsServerTransactionTime);
		if (err != noErr) goto exit3;
		if (responseLen + len > respAlloc) {
			respAlloc += kBufLen;
			MySetHandleSize(response, respAlloc);
		}
		BlockMove(gBuffer, *response + responseLen, len);
		responseLen += len;
		pEnd = gBuffer + len;
		for (p = gBuffer; p < pEnd; p++) if (*p == LF) numReceived++;
		if (gPrefs.logActionsToFile) LogBatchGroupCommandResponse(gBuffer, len, numReceived);
	}
	if (gPrefs.logActionsToFile) LogBatchGroupCommandsFinished();
	MySetHandleSize(response, responseLen);
	
	/* Process the responses.
	   
	   Valid responses are:
	   
	   211 num first last groupName
	   411 no such news group
	   502 access restriction or permission denied (treated same as no such group)
	*/
	
	HLock(response);
	HLock((Handle)groupArray);
	HLock(gGroupNames);
	p = *response;
	pEnd = p + responseLen;
	g = *groupArray + fGroup;
	while (p < pEnd) {
		lineStart = p;
		serverCode = ParseNum(&p);
		if (serverCode != 211 && serverCode != 411 && serverCode != 502) goto exit2;
		while (g->status != 'x') g++;
		if (serverCode == 211) {
			ParseNum(&p);
			first = ParseNum(&p);
			last = ParseNum(&p);
			if (first == 0 && last == 0) {
				/* Special case empty group: set firstMess = lastMess + 1. */
				g->firstMess = g->lastMess + 1;
			} else {
				if (first <= 0) first = 1;
				if (last < first) last = first - 1;
				g->firstMess = first;
				g->lastMess = last;
			} 
			g->numUnread = g->lastMess - g->firstMess + 1;
			gName = *gGroupNames + g->nameOffset;
			len = strlen(gName);
			if (strncasecmp(p, gName, len) != 0) goto exit2;
			p += len;
			if (*p != ' ' && *p != CR && *p != LF) goto exit2;
		} else {
			g->status = 'd';
		}
		g++;
		while (*p != LF) p++;
		p++;
	}
	HUnlock((Handle)groupArray);
	HUnlock(gGroupNames);
	MyDisposHandle(response);
	
	return true;
	
exit1:
	UnexpectedErrorMessage(err);
	AbortNewsConnection();
	goto exit3; 
	
exit2:
	NewsServerErrorMessage("GROUP", lineStart);
	
exit3:
	HUnlock((Handle)groupArray);
	HUnlock(gGroupNames);
	MyDisposHandle(response);
	return false;
}



/*----------------------------------------------------------------------------
	GetGroupArrayArticleRanges 
	
	Queries the NNTP server to get current article range info for designated 
	groups in a group list array.

	Entry:	groupArray = handle to group array.
			numGroups = number of groups in group array.

			Each group in the array to be updated is marked with status='x'.

	Exit:	function result = true if info retrieved, false if error.
			Deleted groups are marked with status='d'.
----------------------------------------------------------------------------*/
 
Boolean GetGroupArrayArticleRanges (TGroup **groupArray, short numGroups)
{
	short firstGroup = 0;

	if (!EstablishNewConnection()) return false;

	if (gPrefs.batchedGroupCmds) {
		*gCurGroup = 0;
		while (firstGroup < numGroups) { 
			if (!PartialGetGroupArrayArticleRanges(groupArray, 
				numGroups, &firstGroup)) return false;
		}
		return true;
	} else {
		return GetGroupArrayArticleRangesOneAtATime(groupArray, numGroups);
	}
}



/*----------------------------------------------------------------------------
	SortHeadersCompare 
	
	This is the comparison function used to sort a header array into 
	increasing order by article number.
	
	Entry:	p = pointer to first THeader struct.
			q = pointer to second THeader struct.
			
	Exit:	function result =
				-1 if p->number < q->number
				 0 if p->number == q->number
				 1 if p->number > q->number
----------------------------------------------------------------------------*/

static short SortHeadersCompare (THeader *p, THeader *q)
{
	static short Counter = 0;

	if((++Counter & 0x1f) == 0) {
		GiveTime();
		Counter = 0;
	}
	
	if (p->number < q->number) {
		return -1;
	} else if (p->number == q->number) {
		return 0;
	} else {
		return 1;
	}
}



/*----------------------------------------------------------------------------
	ReadHeaders
	
	Parses the response from an XHDR or XPAT server command.

	Entry:	strings = handle to strings block, or nil to return only
				article numbers.
			*nextStringOffset = offset of next available location in
				strings block, or nil if returning only article 
				numbers.
			func = pointer to header munging function, or nil if none.
			maxStringLen = maximum string length (if strings != nil).
			first = minimum expected article number.
			last = maximum expected article number.
			serverCommand = "XHDR" or "XPAT".
			
	Exit:	function result = 
				0 if no error.
				2 if error.
				3 if server does not support the command.
			*headers = handle to array of THeader records, sorted into
				increasing order by article number.
			*numHeaders = number of headers.
			*nextStringOffset updated.
			msg = server error message if server does not support the command.
	
	The header munging function "func" should be declared as follows:
	
	short func (char *header)

	Entry: 	header = header string.
	
	Exit:	header = modified header string.
			function result = length of modified header string.
	
	The modified header string must be shorter than or the same length as
	the original header string.
----------------------------------------------------------------------------*/

static short ReadHeaders (Handle strings, long *nextStringOffset,
	 HeaderMunge func, short maxStringLen, long first, long last, char *serverCommand, 
	 THeader ***headers, short *numHeaders, CStr255 msg)
{
	long serverCode;
	Handle response;
	long responseLength;
	short numLinesReceived, numLinesLeft;
	THeader **theHeaders;
	long strNext, strAllocated, strLen;
	char *p, *q, *r;
	THeader *h;
	Boolean sorted = true;
	long prevNumber = 0;
	short nHeaders, maxHeaders;
	
	/* Read the server response. */

	if (!GetBigServerResponse(&serverCode, 
		&response, &responseLength, &numLinesReceived)) return 2;
	if (serverCode == 500) {
		BlockMove(*response, msg, responseLength <= 256 ? responseLength : 256);
		MyDisposHandle(response);
		return 3;
	}
	if (serverCode != 221) {
		HLock(response);
		NewsServerErrorMessage(serverCommand, *response);
		MyDisposHandle(response);
		return 2;
	}
	
	/* Process the response. */
		
	theHeaders = (THeader**)MyNewHandle(numLinesReceived*sizeof(THeader));
	nHeaders = 0;
	maxHeaders = last - first + 1;
	if (strings != nil) {
		strNext = *nextStringOffset;
		strAllocated = GetHandleSize(strings);
	}
	HLock(response);
	HLock((Handle)theHeaders);
	p = *response;
	h = *theHeaders;
	while (*p != LF) p++;	/* skip over 221 server response */ 
	p++;
	numLinesLeft = numLinesReceived;
	while (numLinesLeft--) {
		h->number = ParseNum(&p);
		if (h->number < first || h->number > last) {
			while (*p != LF) p++;
			p++;
			continue;
		}
		sorted = sorted && h->number >= prevNumber;
		q = p;
		while (*p != LF) p++;
		r = p-1;
		p++;
		while (r >= q && (*r == CR || *r == ' ' || *r == '\t')) r--;
		r++;
		if (strings != nil) {
			strLen = r - q;
			if (strNext + strLen + 1 > strAllocated) {
				if (strLen > 9999) strLen = 9999;
				strAllocated += 10000;
				MySetHandleSize(strings, strAllocated);
			}
			BlockMove(q, *strings + strNext, strLen);
			*(*strings + strNext + strLen) = 0;
			if (func != nil) {
				HLock(strings);
				strLen = (*func)(*strings + strNext);
				HUnlock(strings);
			}
			if (strLen > maxStringLen) {
				strLen = maxStringLen;
				*(*strings + strNext + strLen) = 0;
			}
			h->offset = strNext;
			strNext += strLen + 1;
		}
		h++;
		nHeaders++;
		if (nHeaders >= maxHeaders) break;
	}
	MyDisposHandle(response);
	
	if (strings !=  nil) *nextStringOffset = strNext;
	
	if (!sorted) 
		if (!FastQSort(*theHeaders, nHeaders, sizeof(THeader), 
			(short(*)(void*,void*))SortHeadersCompare)) goto exit;
	
	HUnlock((Handle)theHeaders);
	if (nHeaders < numLinesReceived) 
		MySetHandleSize((Handle)theHeaders, nHeaders*sizeof(THeader));
	*headers = theHeaders;
	*numHeaders = nHeaders;
	return 0;

exit:

	MyDisposHandle((Handle)theHeaders);
	return 2;
}



/*----------------------------------------------------------------------------
	GetHeaders 
	
	Gets header lines from the news server.

	Entry:	groupName = the group name.
			headerName = the header name.
			first = first article number.
			last = last article number.
			strings = handle to strings block, or nil to return only
				article numbers.
			*nextStringOffset = offset of next available location in
				strings block.
			func = pointer to header munging function, or nil if none.
			maxStringLen = maximum string length (if strings != nil).
			
	Exit:	function result = 
				0 if no error.
				1 if group does not exist.
				2 if some other error.
			*headers = handle to array of THeader records.
			*numHeaders = number of headers.
			*nextStringOffset updated.
	
	The header munging function "func" should be declared as follows:
	
	short func (char *header)

	Entry: 	header = header string.
	
	Exit:	header = modified header string.
			function result = length of modified header string.
	
	The modified header string must be shorter than or the same length as
	the original header string.
----------------------------------------------------------------------------*/

short GetHeaders (char *groupName, char *headerName, long first,
	long last, Handle strings, long *nextStringOffset,
	HeaderMunge func, short maxStringLen, THeader ***headers, short *numHeaders)
{
	short result;
	CStr255 msg;
	
	if (!ResetConnection()) return 2;
	
	result = SetGroup(groupName);
	if (result != 0) return result;

	sprintf(gBuffer, "XHDR %s %ld-%ld", headerName, first, last);
	if (!SendServerCommand()) return 2;
	
	result = ReadHeaders(strings, nextStringOffset, func, maxStringLen, first, last,
		"XHDR", headers, numHeaders, msg);
	
	if (result != 3) return result;
	
	NewsServerErrorMessage("XHDR", msg);
	return 2;
}



/*----------------------------------------------------------------------------
	SearchHeadersTheHardWay 
	
	Searches header lines and returns matches. The search is for any 
	substring and is case-insensitive. This search function is called
	when the server does not support the XPAT command. In this case,
	we use the XHDR command to fetch all the headers and do the search
	ourselves.

	Entry:	groupName = the group name.
			headerName = the header name.
			first = first article number.
			last = last article number.
			pattern = search string.
			
	Exit:	function result = 
				0 if no error.
				1 if group does not exist.
				2 if some other error.
			*headers = handle to array of THeader records.
			*numHeaders = number of headers.
----------------------------------------------------------------------------*/

static short SearchHeadersTheHardWay (char *groupName, char *headerName, long first, long last,
	char *pattern, THeader ***headers, short *numHeaders)
{
	short result;
	Handle strings = nil;
	long nextStringOffset;
	THeader **theHeaders;
	short numTheHeaders, numMatchedHeaders;
	THeader *pHeader, *pHeaderEnd, *qHeader;
	short patternLen, strLen;
	char *pString, *pStringLast;
	Boolean match;
	
	/* Allocate a temporary memory block to hold the header strings. */
	
	strings = MyNewHandle(10000);
	nextStringOffset = 0;
	
	/* Fetch the headers. */
		
	result = GetHeaders(groupName, headerName, first, last, strings,
		&nextStringOffset, nil, 0x7fff, &theHeaders, &numTheHeaders);
	if (result != 0) {
		MyDisposHandle(strings);
		return result;
	}
	
	/* Walk through the header strings and delete the ones which don't
	   match the pattern. */
	
	HLock(strings);
	HLock((Handle)theHeaders);
	patternLen = strlen(pattern);
	pHeaderEnd = *theHeaders + numTheHeaders;
	qHeader = *theHeaders;
	numMatchedHeaders = 0;
	for (pHeader = *theHeaders; pHeader < pHeaderEnd; pHeader++) {
		pString = *strings + pHeader->offset;
		strLen = strlen(pString);
		match = false;
		if (patternLen <= strLen) {
			pStringLast = pString + strLen - patternLen;
			for (; pString <= pStringLast; pString++) {
				if (strncasecmp(pattern, pString, patternLen) == 0) {
					match = true;
					break;
				}
			}
		}
		if (match) {
			*qHeader = *pHeader;
			qHeader++;
			numMatchedHeaders++;
		}
	} 
	
	/* Clean up and exit. */
	
	HUnlock((Handle)theHeaders);
	MySetHandleSize((Handle)theHeaders, numMatchedHeaders*sizeof(THeader));
	MyDisposHandle(strings);
	*headers = theHeaders;
	*numHeaders = numMatchedHeaders;
	return 0;
}



/*----------------------------------------------------------------------------
	SearchHeaders 
	
	Searches header lines and returns matches. The search is for any 
	substring and is case-insensitive. The XPAT command is used if the
	server supports it and the "use XPAT" preference is turned on. 
	Otherwise, we do it the hard way.

	Entry:	groupName = the group name.
			headerName = the header name.
			first = first article number.
			last = last article number.
			pattern = search string.
			
	Exit:	function result = 
				0 if no error.
				1 if group does not exist.
				2 if some other error.
			*headers = handle to array of THeader records.
			*numHeaders = number of headers.
----------------------------------------------------------------------------*/

short SearchHeaders (char *groupName, char *headerName, long first, long last,
	char *pattern, THeader ***headers, short *numHeaders)
{
	short result;
	char *p, *q;
	static Boolean haveXPAT = true;
	CStr255 msg;
	
	if (!haveXPAT || !gPrefs.useXPAT) 
		return SearchHeadersTheHardWay (groupName, headerName, first, last, 
			pattern, headers, numHeaders);

	if (!ResetConnection()) return 2;
	
	result = SetGroup(groupName);
	if (result != 0) return result;
	
	/* Build the XPAT command. Note the goofiness for the ignoreCase option because
	   the XPAT command is case sensitive. Note also that we add 1 to the last
	   article number, to avoid an error in inn 1.4 which never matches the last
	   article in the range. */

	sprintf(gBuffer, "XPAT %s %ld-%ld ", headerName, first, last);
	q = gBuffer + strlen(gBuffer);
	*q++ = '*';
	for (p = pattern; *p; p++) {
		if (isupper(*p)) {
			*q++ = '[';
			*q++ = *p;
			*q++ = tolower(*p);
			*q++ = ']';
		} else if (islower(*p)) {
			*q++ = '[';
			*q++ = *p;
			*q++ = toupper(*p);
			*q++ = ']';
		} else if (*p == '*' || *p == '?' || *p == '\\' || *p == '[' || *p == ']') {
			*q++ = '\\';
			*q++ = *p;
		} else {
			*q++ = *p;
		}
	}
	*q++ = '*';
	*q++ = 0;

	/* Send the command and process the reply. */

	if (!SendServerCommand()) return 2;
	
	result = ReadHeaders(nil, nil, nil, 0, first, last, "XPAT", headers, numHeaders, msg);
	if (result != 3) return result;
	haveXPAT = false;
	return SearchHeadersTheHardWay (groupName, headerName, first, last, 
		pattern, headers, numHeaders);
 
}



/*----------------------------------------------------------------------------
	CloseIdleNewsConnectionCompletionRoutine
	
	This is the asynchronous completion routine used to close idle news
	server connections.
	
	This completion routine chains to itself to do the following tasks
	involved in gracefully tearing down the connection in the background:
	
	Wait for QUIT command send to complete (the send is initiated by the
		CloseIdleNewsConnection function below).
	Read incoming data until an error occurs (signalling that the
		news server has closed its end of the connection).
	Close our end of the connection.
	Release the stream.
	
	Entry:	Register A0 = pointer to closing news connection queue element.
----------------------------------------------------------------------------*/

static void CloseIdleNewsConnectionCompletionRoutine (/* a0 = */ TCPiopb *iopb)
{
	TQueuedClosingNewsConnection *p;
	OSErr err;
	long savedA5;

	asm {
		move.l	a0,p
	}
	
	savedA5 = SetA5(p->myA5);
	
	if (p->pBlock.csCode == TCPSend) {
	
		/* The QUIT command has been sent. Start the first receive. */
		
		if (p->pBlock.ioResult != noErr) {
			p->destroy = true;
			SetA5(savedA5);
			return;
		}
		memset(&p->pBlock, 0, sizeof(TCPiopb));
		p->pBlock.ioCompletion = CloseIdleNewsConnectionCompletionRoutine;
		p->pBlock.ioCRefNum = p->ioCRefNum;
		p->pBlock.tcpStream = p->connID;
		p->pBlock.csCode = TCPRcv;
		p->pBlock.csParam.receive.commandTimeoutValue = 60;
		p->pBlock.csParam.receive.rcvBuff = (Ptr)&p->receiveBuf;
		p->pBlock.csParam.receive.rcvBuffLen = 256;
	
	} else if (p->pBlock.csCode == TCPRcv) {
	
		/* A receive operation has finished. If there was no error, start another
		   receive. If an error occurred, it was because the server has closed
		   its side of the connection. In this case, we start the TCPClose. */
	   
	   	err = p->pBlock.ioResult;
		memset(&p->pBlock, 0, sizeof(TCPiopb));
		p->pBlock.ioCompletion = CloseIdleNewsConnectionCompletionRoutine;
		p->pBlock.ioCRefNum = p->ioCRefNum;
		p->pBlock.tcpStream = p->connID;
		if (err == noErr) {
			p->pBlock.csCode = TCPRcv;
			p->pBlock.csParam.receive.commandTimeoutValue = 60;
			p->pBlock.csParam.receive.rcvBuff = (Ptr)&p->receiveBuf;
			p->pBlock.csParam.receive.rcvBuffLen = 256;
		} else {
			p->pBlock.csCode = TCPClose;
			p->pBlock.csParam.send.ulpTimeoutValue = 60;
			p->pBlock.csParam.send.ulpTimeoutAction = 1;
			p->pBlock.csParam.send.validityFlags = 0xC0;
		}
	
	} else if (p->pBlock.csCode == TCPClose) {
	
		/* The close has finished. Start the release. */

		if (p->pBlock.ioResult != noErr) {
			p->destroy = true;
			SetA5(savedA5);
			return;
		}
		memset(&p->pBlock, 0, sizeof(TCPiopb));
		p->pBlock.ioCompletion = CloseIdleNewsConnectionCompletionRoutine;
		p->pBlock.ioCRefNum = p->ioCRefNum;
		p->pBlock.tcpStream = p->connID;
		p->pBlock.csCode = TCPRelease;
	
	} else if (p->pBlock.csCode == TCPRelease) {
	
		/* The release finished. Set the "finished" field to signal that we can
		   now dispose of the queue element. */
		   
		if (p->pBlock.ioResult != noErr) {
			p->destroy = true;
		} else {
			p->finished = true;
		}
		SetA5(savedA5);
		return;
		
	}
	
	/* Issue the next PBControl call in the chain. */

	err = PBControl((ParmBlkPtr)&p->pBlock, true);
	if (err != noErr) p->destroy = true;
	SetA5(savedA5);
	
}



/*----------------------------------------------------------------------------
	DornerHack 
	
	Steve Dorner uses a goofy passthrough hack when he runs NewsWatcher. His
	hack doesn't work with the automatic connection closing. This code fixes
	his problem - if there is a resource of type 'DORN' with id=0, we
	don't automatically close news connections.
	
	Exit:	function result = true if Dorner.
----------------------------------------------------------------------------*/

static Boolean DornerHack (void)
{
	static short dornerStatus = -1;	/* -1 = haven't checked yet, 0 = not Dorner,
								       1 = Dorner */
	Handle h;


	if (dornerStatus >= 0) return dornerStatus;
	h = GetResource('DORN', 0);
	dornerStatus = h == nil ? 0 : 1;
	if (h != nil) ReleaseResource(h);
	return dornerStatus;
}



/*----------------------------------------------------------------------------
	CloseIdleNewsConnection 
	
	This function is called by the main event loop. It checks to see if the
	current connection to the news server has been idle for 10 minutes. If
	it has, the connection is closed. This prevents idle server processes from
	using up process slots and swap space on the server.
	
	Unlike all the other TCP/IP transactions in NewsWatcher, this kind of
	connection closing must be done asynchronously. We use asynchronous
	PBControl calls with completion routine chaining to do the close.
	
	All the pending stream closes are linked together in a queue. This function
	also takes care of removing the queue elements and deallocating memory when
	the close operations have completed.
	
	Entry:	destroy = true if program termination. All pending closes are abruptly
			terminated.
----------------------------------------------------------------------------*/

void CloseIdleNewsConnection (Boolean destroy)
{
	TQueuedClosingNewsConnection *p, *prev = nil, *next = nil;
	TCPiopb pb;
	unsigned long curTime;
	OSErr err;
	
	if (DornerHack()) return;

	/* First take care of completed close operations, if any, and the destroy
	   option. */

	for (p = gClosingNewsConnectionQueue; p != nil; p = next) {
		next = p->next;
		if (destroy || p->destroy || p->finished) {
			if (destroy || p->destroy) {
				pb.ioCRefNum = GetTCPRefNum();
				pb.tcpStream = gConnID;
				pb.csCode = TCPRelease;
				pb.tcpStream = p->pBlock.tcpStream;
				PBControl((ParmBlkPtr)&pb, false);
				MyDisposPtr(pb.csParam.create.rcvBuff);
			} else {
				MyDisposPtr(p->pBlock.csParam.create.rcvBuff);
			}
			if (prev == nil) {
				gClosingNewsConnectionQueue = next;
			} else {
				prev->next = next;
			}
			MyDisposPtr((Ptr)p);
		} else {
			prev = p;
		}
	}
	if (destroy) return;
	
	/* Check to see if the current news server connection has been idle for at
	   least 10 minutes. */
	   
	if (gConnID == 0) return;
	GetDateTime(&curTime);
	if (curTime <= gLastNewsServerTransactionTime + 10*60) return;
	
	/* Allocate a new queue element. */

	p = (TQueuedClosingNewsConnection*)MyNewPtr(sizeof(TQueuedClosingNewsConnection));
	p->myA5 = SetCurrentA5();
	p->destroy = false;
	p->finished = false;
	p->next = gClosingNewsConnectionQueue;
	p->startingTick = TickCount();
	gClosingNewsConnectionQueue = p;
	p->pBlock.ioCompletion = CloseIdleNewsConnectionCompletionRoutine;
	p->pBlock.ioCRefNum = p->ioCRefNum = GetTCPRefNum();
	p->pBlock.tcpStream = p->connID = gConnID;
	
	/* Force a new connection to be opened on the next transaction. */
	
	gConnID = 0;
	
	/* Send an asynchronous "QUIT" command to the server, chained to our
	   completion routine . */

	strcpy(p->sendBuf, "QUIT");
	strcat(p->sendBuf, CRLF);
	p->wds[0].length = 6;
	p->wds[0].ptr = (Ptr)&p->sendBuf;
	p->wds[1].length = 0;
	p->wds[1].ptr = nil;
	
	p->pBlock.csCode = TCPSend;
	p->pBlock.csParam.send.ulpTimeoutValue = 60;
	p->pBlock.csParam.send.ulpTimeoutAction = 1;
	p->pBlock.csParam.send.validityFlags = 0xC0;
	p->pBlock.csParam.send.wdsPtr = (Ptr)&p->wds;

	err = PBControl((ParmBlkPtr)&p->pBlock, true);
	if (err != noErr) p->destroy = true;
}



/*----------------------------------------------------------------------------
	GetServerInfoCopyHandler
	
	Handles the Edit menu Copy command for the get server info dialog.
		
	Entry:	dlg = pointer to dialog.
----------------------------------------------------------------------------*/

static void GetServerInfoCopyHandler (DialogPtr dlg)
{
	ZeroScrap();
	HLock(gServerInfoText);
	PutScrap(GetHandleSize(gServerInfoText), 'TEXT', *gServerInfoText);
	HUnlock(gServerInfoText);
	TEFromScrap();
}



/*----------------------------------------------------------------------------
	GetServerInfo 
	
	Handles the "Get Server Info" command in the Special menu.
----------------------------------------------------------------------------*/

void GetServerInfo (void)
{
	DialogPtr dlg;
	long serverCode;
	long responseLength;
	short numLinesReceived;
	char str[1000];
	long offset;
	char *p;
	short item;
	short fontNum;
	Handle serverHelpResponse = nil;
	Str255 dateStr, timeStr, versStr;
	CStr255 addrStr;
	unsigned long rawSecs;
	Handle vers1Resource;

	HiliteMenu(0);

	if (!ResetConnection()) return;

	GetDateTime(&rawSecs);
	IUDateString(rawSecs, abbrevDate, dateStr);
	IUTimeString(rawSecs, false, timeStr);
	vers1Resource = GetResource('vers', 1);
	pstrcpy(versStr, (StringPtr)*vers1Resource+6);
	sprintf(addrStr, "%ld.%ld.%ld.%ld", (gNNTPAddr >> 24) & 0xff, (gNNTPAddr >> 16) & 0xff, 
	(gNNTPAddr >> 8) & 0xff, gNNTPAddr & 0xff);
	sprintf(str, "%#s %#s\rNewsWatcher version %#s\rServer domain name: %#s\rServer IP address: %s\r\r%s\r\r", 
		dateStr, timeStr, versStr, gPrefs.newsServerName, addrStr, gHello);

	sprintf(gBuffer, "HELP%s", CRLF);
	if (!SendServerCommand()) return;
	if (!GetBigServerResponse(&serverCode, 
		&serverHelpResponse, &responseLength, &numLinesReceived)) return;
	for (offset = 0; offset >= 0; offset = Munger(serverHelpResponse, offset, LFSTR, 1, "", 0));
	p = *serverHelpResponse + GetHandleSize(serverHelpResponse) - 1;
	while (p > *serverHelpResponse && (*p == '.' || *p == CR)) p--;
	SetHandleSize(serverHelpResponse, p - *serverHelpResponse + 1);
		
	dlg = MyGetNewDialog(kServerInfoDlg);
	PtrToHand(str, &gServerInfoText, strlen(str));
	HLock(serverHelpResponse);
	HandAndHand(serverHelpResponse, gServerInfoText);
	HUnlock(serverHelpResponse);
	GetFNum("\pMonaco", &fontNum);
	SetItemReadOnly(dlg, kServerInfoTheInfoItem, gServerInfoText, fontNum, 9);
	MyDisposHandle(serverHelpResponse);
	
	SetDialogCustomCopyHandler(dlg, GetServerInfoCopyHandler);

	MyModalDialog(DialogFilter, &item, false, true);
	
	MyDisposDialog(dlg);
	MyDisposHandle(gServerInfoText);
}
