void OpenFile (FSSpec *theFile);
void DoOpenGroupList (void);
void DoGetGroupListFromHost (Boolean autoFetch);
Boolean DoSaveAs (WindowPtr wind);
Boolean DoSave (WindowPtr wind);
Boolean CheckForSave (WindowPtr wind);
void DoSendGroupListToHost (WindowPtr wind, char *host, char *name, 
	char *pass, char *path);
