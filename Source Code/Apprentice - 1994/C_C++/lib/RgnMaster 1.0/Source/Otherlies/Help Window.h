void InitHelpWindow();
void ShowHelpWindow();
void HideHelpWindow();
void UpdateHelpWindow();
void ScrollHelpWindow(Point mouseLoc);
pascal void HelpWindowScrollProc(ControlHandle theControl, short actionPart);

extern WindowPtr gHelpWindow;
extern ControlHandle gScrollbar;