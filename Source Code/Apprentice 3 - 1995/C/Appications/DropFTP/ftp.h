Boolean FTPGetFile (char *host, char *user, char *pswd, 
	char *file, Handle data);
Boolean FTPPutFile (char *host, char *user, char *pswd, 
	char *file, Ptr data, long size);
void HandleUpdate (WindowPtr wind);
void ErrorMessage(char *s);
void UnexpectedErrorMessage(short s);
void MailOrFTPServerErrorMessage(char *s);