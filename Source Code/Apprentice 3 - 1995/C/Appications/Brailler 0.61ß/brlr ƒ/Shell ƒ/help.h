extern	short			gMainTopicShowing;
extern	short			gSubTopicShowing;

void SetupTheHelpWindow(WindowPtr theWindow);
void ShutDownTheHelpWindow(void);
void OpenTheHelpWindow(WindowPtr theWindow);
void DisposeTheHelpWindow(WindowPtr theWindow);
void KeyPressedInHelpWindow(WindowPtr theWindow, unsigned char keyPressed);
void MouseClickedInHelpWindow(WindowPtr theWindow, Point mouseLoc);
void DrawTheHelpWindow(WindowPtr theWindow, short theDepth);
