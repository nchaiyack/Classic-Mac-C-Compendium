OSErr OpenTheIndWindow(short index);
GrafPtr PreallocateOffscreenWorld(WindowPtr theWindow);
OSErr UpdateTheWindow(WindowPtr theWindow);
Boolean CloseTheWindow(WindowPtr theWindow);
void GetMainScreenBounds(Rect *screenRect);
short GetBiggestDeviceDepth(WindowPtr theWindow);
short GetWindowRealDepth(WindowPtr theWindow);

