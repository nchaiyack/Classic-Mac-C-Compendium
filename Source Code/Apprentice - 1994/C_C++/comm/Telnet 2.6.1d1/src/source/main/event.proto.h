
/* event.c */
void	HandleMouseDown(EventRecord myEvent);
short updateCursor(short force);
void NoWindow(void);
Boolean CheckPageKeys(short code);
void SendOneChar(unsigned char sendch);
void HandleKeyDown(EventRecord theEvent);
void DoEvents(void);
void	CloseAWindow(WindowPtr	theWindow);
