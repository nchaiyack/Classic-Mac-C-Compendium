/*****
 * mini.windows.h
 *
 *	Public interfaces for mini.windows.c
 *
 *****/

int SetUpWindows(void);
int MyGrowWindow(WindowPtr w, Point p);
int DoContent(WindowPtr theWindow, EventRecord *theEvent);
int ShowSelect(void);
int UpdateWindow (WindowPtr theWindow);
int CloseMyWindow(void);
