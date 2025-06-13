#ifndef __MUNGEDIALOG__
#define __MUNGEDIALOG__

extern UniversalProcPtr	gOldNewControl;
extern UniversalProcPtr	gOldDisposeControl;
extern UniversalProcPtr	gOldGetNewDialog;
extern UniversalProcPtr	gOldShowWindow;
extern short			gMyItemBase;

void					MungeNextDialog(void);
pascal ControlHandle	MyNewControl(WindowPtr theWindow, Rect* theRect, Str255 title,
									Boolean visible, short value, short min,
									short max, short procID, long refcon);
pascal void				MyDisposeControl(ControlHandle theControl);
pascal DialogPtr		MyGetNewDialog(short dialogID, void *dStorage, WindowPtr behind);
pascal void				MyShowWindow(WindowPtr window);

#endif