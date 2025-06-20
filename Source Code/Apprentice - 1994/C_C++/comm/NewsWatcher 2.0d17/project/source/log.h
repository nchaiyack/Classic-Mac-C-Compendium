void LogBatchGroupCommandSend (char *groupCmds, short groupCmdsLen, 
	short numGroupsToCheck);
void LogBatchGroupCommandResponse (char *response, long length, short numReceived);
void LogBatchGroupCommandsFinished (void);
void LogGroupCommand (char *groupCmd);
void LogGroupCommandResponse (char *response, unsigned short length);
void LogGetGroupArrayArticleRangesOneAtATimeBegin (void);
void LogGetGroupArrayArticleRangesOneAtATimeEnd (void);
void LogNewsrcParseBegin (Handle newsrc);
void LogNewsrcGroupNameTooLong (char *groupName);
void LogNewsrcNoGroupName (void);
void LogNewsrcGroupNotInFullGroupList (char *groupName);
void LogNewsrcReadListSyntaxError (char *groupName);
void LogNewsrcSubscribedOK (char *groupName);
void LogNewsrcUnsubscribedOK (char *groupName);
void LogNewsrcNoColonOrBang (char *groupName);
void LogNewsrcGroupNotOnServer (char *groupName);
void LogNewsrcParseEnd (TGroup **userGroupArray, short numUserGroups, Handle unsubscribed);
void LogNewsrcWriteBegin (TGroup **userGroupArray, short numUserGroups, Handle unsubscribed);
void LogNewsrcWriteEnd (Handle newsrc);
void LogTestBatchGroupCommandsBegin (void);
void LogTestBatchGroupCommandsEnd (char *response, unsigned short respLen, 
	Boolean serverSupportsBatchGroupCommands);
void LogHeaders (char *response, long respLen);
void LogMailMessage (char *text, long textLen);
void OpenLogFile (void);
void CloseLogFile (void);