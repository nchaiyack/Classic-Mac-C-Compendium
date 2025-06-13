void GenericCopybits(WindowPtr theWindow, WindowPtr offscreenWindowPtr, Boolean active);
void GenericResizeControls(WindowPtr theWindow);
void GenericGetGrowSize(WindowPtr theWindow, Rect *sizeRect);
Boolean GenericKeyPressedInWindow(WindowPtr theWindow, unsigned char theChar);
Boolean GenericMouseClickedInWindow(WindowPtr theWindow, Point thePoint, Boolean isEditable);
void GenericActivate(WindowPtr theWindow);
void GenericDeactivate(WindowPtr theWindow);
