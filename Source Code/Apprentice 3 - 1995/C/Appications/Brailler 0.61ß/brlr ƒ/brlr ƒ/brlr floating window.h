void SetupTheFloatingWindow(WindowPtr theWindow);
void OpenTheFloatingWindow(WindowPtr theWindow);
void CloseTheFloatingWindow(WindowPtr theWindow);
Boolean KeyPressedInFloatingWindow(WindowPtr theWindow, unsigned char theChar);
void MouseClickedInFloatingWindow(WindowPtr theWindow, Point thePoint);
void DisposeTheFloatingWindow(WindowPtr theWindow);
void DrawTheFloatingWindow(WindowPtr theWindow, short theDepth);
void ReceiveDragInFloatingWindow(WindowPtr theWindow);
