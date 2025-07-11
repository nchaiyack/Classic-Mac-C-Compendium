void SetupTheMainWindow(WindowPtr theWindow);
void ShutDownTheMainWindow(void);
Boolean CloseTheMainWindow(WindowPtr theWindow);
void KeyPressedInMainWindow(WindowPtr theWindow, unsigned char theChar);
void MouseClickedInMainWindow(WindowPtr theWindow, Point thePoint);
void ActivateTheMainWindow(WindowPtr theWindow);
void DeactivateTheMainWindow(WindowPtr theWindow);
void DrawTheMainWindow(WindowPtr theWindow, short theDepth);
Boolean CopybitsTheMainWindow(WindowPtr theWindow, WindowPtr offscreenWindowPtr);
