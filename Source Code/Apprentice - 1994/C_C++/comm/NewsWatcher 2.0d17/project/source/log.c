/*----------------------------------------------------------------------------

	log.c

	This module handles logging.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Packages.h>
#include <stdio.h>
#include <string.h>

#include "glob.h"
#include "log.h"
#include "util.h"
#include "dlgutil.h"



static short gRefNum = 0;



/*----------------------------------------------------------------------------
	Munge
	
	Makes CR and LF characters visible in a relocatble block.
	
	Entry:	h = handle to block.
----------------------------------------------------------------------------*/

static void Munge (Handle h)
{
	long offset;
	
	for (offset = 0; offset >= 0; offset = Munger(h, offset, CRSTR, 1, "\\n", 2));
	for (offset = 0; offset >= 0; offset = Munger(h, offset, LFSTR, 1, "\\r\r", 3));
}



/*----------------------------------------------------------------------------
	PutString 
	
	Writes a string to the log file.
	
	Entry:	str = string to write.
----------------------------------------------------------------------------*/

static void PutString (char *str)
{
	char line[513];
	long len;
	
	sprintf(line, "%s\r", str);
	len = strlen(line);
	FSWrite(gRefNum, &len, line);
}



/*----------------------------------------------------------------------------
	LogBatchGroupCommandSend 
	
	Logs the batch sending of GROUP commands when checking for new
	articles.
	
	Entry:	groupCmds = GROUP commands as sent to server.
			groupCmdsLen = length of groupCmds buffer.
			numGroupsToCheck = number of GROUP commands sent.
----------------------------------------------------------------------------*/

void LogBatchGroupCommandSend (char *groupCmds, short groupCmdsLen, short numGroupsToCheck)
{
	Handle tmpHandle;
	long len;
	CStr255 msg;

	if (gRefNum == 0) return;
	PutString("");
	sprintf(msg, "Batch GROUP commands sent to news server.\r%d commands sent.", numGroupsToCheck);
	PutString(msg);
	PtrToHand(groupCmds, &tmpHandle, groupCmdsLen);
	Munge(tmpHandle);
	len = GetHandleSize(tmpHandle);
	HLock(tmpHandle);
	FSWrite(gRefNum, &len, *tmpHandle);
	MyDisposHandle(tmpHandle);
}



/*----------------------------------------------------------------------------
	LogBatchGroupCommandResponse 
	
	Logs a buffer received from the server in response to the batch sending 
	of GROUP commands when checking for new articles.
	
	Entry:	response = buffer received from server.
			length = length of buffer.
			numReceived = number of lines received from server so far.
----------------------------------------------------------------------------*/

void LogBatchGroupCommandResponse (char *response, long length, short numReceived)
{
	Handle tmpHandle;
	long len;
	CStr255 msg;

	if (gRefNum == 0) return;
	PutString("");
	PutString("Buffer received from news server in response to batch GROUP commands.");
	sprintf(msg, "%d responses received so far.", numReceived);
	PutString(msg);
	PtrToHand(response, &tmpHandle, length);
	Munge(tmpHandle);
	len = GetHandleSize(tmpHandle);
	HLock(tmpHandle);
	FSWrite(gRefNum, &len, *tmpHandle);
	if ((*tmpHandle)[len-1] != '\r') {
		len = 1;
		FSWrite(gRefNum, &len, CRSTR);
	}
	MyDisposHandle(tmpHandle);
}



/*----------------------------------------------------------------------------
	LogBatchGroupCommandsFinished 
	
	Logs the completion of a batch GROUP command send and receive operation.
----------------------------------------------------------------------------*/

void LogBatchGroupCommandsFinished (void)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("All news server responses successfully received from batch GROUP commands.");
}



/*----------------------------------------------------------------------------
	LogGroupCommand 
	
	Log a GROUP command.
	
	Entry:	groupCmd = pointer to GROUP command, no CRLF terminator,
				0 terminator.
----------------------------------------------------------------------------*/

void LogGroupCommand (char *groupCmd)
{
	char msg[512];

	if (gRefNum == 0) return;
	PutString("");
	sprintf(msg, "GROUP command sent to server: %s", groupCmd);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogGroupCommandResponse 
	
	Log a response to a GROUP command.
	
	Entry:	response = pointer to response, with CRLF terminator.
			length = length of response.
----------------------------------------------------------------------------*/

void LogGroupCommandResponse (char *response, unsigned short length)
{
	CStr255 resp;
	char msg[512];
	char *p, *pEnd;
	
	if (gRefNum == 0) return;
	if (length > 255) length = 255;
	pEnd = response + length;
	for (p = response; *p != CR && p < pEnd; p++);
	length = p - response;
	BlockMove(response, resp, length);
	resp[length] = 0;
	sprintf(msg, "GROUP command response received: %s", resp);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogGetGroupArrayArticleRangesOneAtATimeBegin 
	
	Log the begin of a get group array article range one at a time operation.
----------------------------------------------------------------------------*/

void LogGetGroupArrayArticleRangesOneAtATimeBegin (void)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("Begin get group array article range one a time operation.");
}



/*----------------------------------------------------------------------------
	LogGetGroupArrayArticleRangesOneAtATimeEnd 
	
	Log the end of a get group array article range one at a time operation.
----------------------------------------------------------------------------*/

void LogGetGroupArrayArticleRangesOneAtATimeEnd (void)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("End get group array article range one a time operation.");
}



/*----------------------------------------------------------------------------
	LogNewsrcParseBegin 
	
	Log the start of a newsrc parse.
	
	Entry:	newsrc = handle to newsrc with 0 terminator.
----------------------------------------------------------------------------*/

void LogNewsrcParseBegin (Handle newsrc)
{
	long len;

	if (gRefNum == 0) return;
	PutString("");
	PutString("Begin newsrc parse.");
	len = GetHandleSize(newsrc) - 1;
	HLock(newsrc);
	FSWrite(gRefNum, &len, *newsrc);
	HUnlock(newsrc);
	PutString("");
}



/*----------------------------------------------------------------------------
	LogNewsrcGroupNameTooLong 
	
	Log a newsrc group name too long error.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcGroupNameTooLong (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Group name too long: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcGroupNotInFullGroupList 
	
	Log a newsrc group not in full group list error.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcGroupNotInFullGroupList (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Group not in full group list: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcNoGroupName 
	
	Log a newsrc no group name error.
----------------------------------------------------------------------------*/

void LogNewsrcNoGroupName (void)
{
	if (gRefNum == 0) return;
	PutString("Missing group name");
}



/*----------------------------------------------------------------------------
	LogNewsrcReadListSyntaxError 
	
	Log a newsrc group article read list syntax error.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcReadListSyntaxError (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Syntax error in article read list: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcSubscribedOK 
	
	Log a newsrc group subscribed OK.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcSubscribedOK (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Subscribed, parsed OK: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcUnsubscribedOK 
	
	Log a newsrc unsubscribed group.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcUnsubscribedOK (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Unsubscribed, parsed OK: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcNoColonOrBang 
	
	Log a newsrc group no colon or bang syntax error.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcNoColonOrBang (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "No ':' or '!' following group name: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	LogNewsrcGroupNotOnServer 
	
	Log a newsrc group not on server error.
	
	Entry:	groupName = group name.
----------------------------------------------------------------------------*/

void LogNewsrcGroupNotOnServer (char *groupName)
{
	char msg[512];
	
	if (gRefNum == 0) return;
	sprintf(msg, "Group not on server: %s", groupName);
	PutString(msg);
}



/*----------------------------------------------------------------------------
	DumpUserGroupArray 
	
	Dump a user group array to the log file.
	
	Entry:	userGroupArray = handle to user group array.
			numUserGroups = number of groups in array.
			unsubscribed = handle to saved unsubscribed groups.
----------------------------------------------------------------------------*/

static void DumpUserGroupArray (TGroup **userGroupArray, short numUserGroups, Handle unsubscribed)
{
	char msg[512];
	TGroup theGroup;
	short i;
	TUnread **unread;
	long len;

	if (gRefNum == 0) return;
	sprintf(msg, "%d groups in user group list.", numUserGroups);
	HLock(gGroupNames);
	PutString(msg);
	for (i = 0; i < numUserGroups; i++) {
		theGroup = (*userGroupArray)[i];
		sprintf(msg, "%s: firstMess = %ld, lastMess = %ld, numUnread = %ld", 
			*gGroupNames + theGroup.nameOffset, theGroup.firstMess,
			theGroup.lastMess, theGroup.numUnread);
		PutString(msg);
		PutString("   Unread list:");
		for (unread = theGroup.unread; unread != nil; unread = (**unread).next) {
			sprintf(msg, "      %ld-%ld", (**unread).firstUnread, (**unread).lastUnread);
			PutString(msg);
		}
	}
	HUnlock(gGroupNames);
	len = GetHandleSize(unsubscribed);
	if (len > 0) {
		PutString("Unsubscribed groups:");
		HLock(unsubscribed);
		FSWrite(gRefNum, &len, *unsubscribed);
		HUnlock(unsubscribed);
	}
}



/*----------------------------------------------------------------------------
	LogNewsrcParseEnd 
	
	Log the end of a newsrc parse.
	
	Entry:	userGroupArray = handle to user group array.
			numUserGroups = number of groups in array.
			unsubscribed = handle to saved unsubscribed groups.
----------------------------------------------------------------------------*/

void LogNewsrcParseEnd (TGroup **userGroupArray, short numUserGroups, Handle unsubscribed)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("End of newsrc parse.");
	DumpUserGroupArray(userGroupArray, numUserGroups, unsubscribed);
}



/*----------------------------------------------------------------------------
	LogNewsrcWriteBegin 
	
	Log the beginning of a newsrc write.
	
	Entry:	userGroupArray = handle to user group array.
			numUserGroups = number of groups in array.
			unsubscribed = handle to saved unsubscribed groups.
----------------------------------------------------------------------------*/

void LogNewsrcWriteBegin (TGroup **userGroupArray, short numUserGroups, Handle unsubscribed)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("Begin newsrc write.");
	DumpUserGroupArray(userGroupArray, numUserGroups, unsubscribed);
}



/*----------------------------------------------------------------------------
	LogNewsrcWriteEnd 
	
	Log the end of a newsrc parse.
	
	Entry:	newsrc = handle to newsrc.
----------------------------------------------------------------------------*/

void LogNewsrcWriteEnd (Handle newsrc)
{
	long len;

	if (gRefNum == 0) return;
	PutString("");
	PutString("End newsrc write.");
	len = GetHandleSize(newsrc);
	HLock(newsrc);
	FSWrite(gRefNum, &len, *newsrc);
	HUnlock(newsrc);
}



/*----------------------------------------------------------------------------
	LogTestBatchGroupCommandsBegin 
	
	Log the beginning of a batch group commands test.
----------------------------------------------------------------------------*/

void LogTestBatchGroupCommandsBegin (void)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("Begin batch group commands test.");
}



/*----------------------------------------------------------------------------
	LogTestBatchGroupCommandsEnd 
	
	Log the end of a batch group commands test.
	
	Entry:	response = pointer to response text.
			respLen = length of response text.
			serverSupportsBatchGroupCommands = result of test.
----------------------------------------------------------------------------*/

void LogTestBatchGroupCommandsEnd (char *response, unsigned short respLen, 
	Boolean serverSupportsBatchGroupCommands)
{
	Handle tmpHandle;
	long len;
	CStr255 msg;

	if (gRefNum == 0) return;
	PutString("");
	sprintf(msg, "End of batch group commands test: gServerSupportsBatchGroupCommands = %s",
		serverSupportsBatchGroupCommands ? "true" : "false");
	PutString(msg);
	PtrToHand(response, &tmpHandle, respLen);
	Munge(tmpHandle);
	len = GetHandleSize(tmpHandle);
	HLock(tmpHandle);
	FSWrite(gRefNum, &len, *tmpHandle);
	MyDisposHandle(tmpHandle);
}



/*----------------------------------------------------------------------------
	LogHeaders 
	
	Log a server response to an XHDR or XPAT command.
	
	Entry:	response = pointer to response text.
			respLen = length of response text.
----------------------------------------------------------------------------*/

void LogHeaders (char *response, long respLen)
{
	Handle tmpHandle;
	long len;
	
	if (gRefNum == 0) return;
	PutString("");
	PutString("Headers received from server.");
	PtrToHand(response, &tmpHandle, respLen);
	Munge(tmpHandle);
	len = GetHandleSize(tmpHandle);
	HLock(tmpHandle);
	FSWrite(gRefNum, &len, *tmpHandle);
	MyDisposHandle(tmpHandle);
}



/*----------------------------------------------------------------------------
	LogMailMessage 
	
	Log a mail message.
	
	Entry:	text = pointer to message text.
			textLen = length of message text.
----------------------------------------------------------------------------*/

void LogMailMessage (char *text, long textLen)
{
	Handle tmpHandle;
	long len;
	
	if (gRefNum == 0) return;
	PutString("");
	PutString("Mail message just before sending to SMTP server.");
	PtrToHand(text, &tmpHandle, textLen);
	Munge(tmpHandle);
	len = GetHandleSize(tmpHandle);
	HLock(tmpHandle);
	FSWrite(gRefNum, &len, *tmpHandle);
	MyDisposHandle(tmpHandle);
	PutString("");
}



/*----------------------------------------------------------------------------
	OpenLogFile 
	
	Opens the log file.
----------------------------------------------------------------------------*/

void OpenLogFile (void)
{
	FCBPBRec pBlock;
	FSSpec logFile;
	OSErr err;
	Handle vers1Resource;
	Str255 versStr;
	CStr255 str;
	
	if (gRefNum != 0) return;
	pBlock.ioNamePtr = nil;
	pBlock.ioVRefNum = 0;
	pBlock.ioRefNum = CurApRefNum;
	pBlock.ioFCBIndx = 0;
	err = PBGetFCBInfo(&pBlock, false);
	if (err != noErr) goto exit;
	err = FSMakeFSSpec(pBlock.ioFCBVRefNum, pBlock.ioFCBParID, 
		"\pNewsWatcher Log", &logFile);
	if (err != noErr && err != fnfErr) goto exit;
	err = FSpOpenDF(&logFile, fsRdWrPerm, &gRefNum);
	if (err == fnfErr) {
		err = FSpCreate(&logFile, gPrefs.textCreator, 'TEXT', smSystemScript);
		if (err != noErr) goto exit;
		err = FSpOpenDF(&logFile, fsRdWrPerm, &gRefNum);
	}
	if (err != noErr) goto exit;
	err = SetEOF(gRefNum, 0);
	if (err != noErr) goto exit;
	PutString("");
	PutString("------------------------------------------------------------------------------");
	PutString("");
	PutString("Log opened.");
	vers1Resource = GetResource('vers', 1);
	pstrcpy(versStr, (StringPtr)*vers1Resource+6);
	sprintf(str, "NewsWatcher version %#s", versStr);
	PutString(str);
	return;
	
exit:
	if (gRefNum != 0) FSClose(gRefNum);
	gRefNum = 0;
	ErrorMessage("Could not open log file. No log will be written.");
}



/*----------------------------------------------------------------------------
	CloseLogFile 
	
	Closes the log file.
----------------------------------------------------------------------------*/

void CloseLogFile (void)
{
	if (gRefNum == 0) return;
	PutString("");
	PutString("Log closed.");
	FSClose(gRefNum);
	gRefNum = 0;
}