void GenericCopybits(WindowPtr theWindow, WindowPtr offscreenWindowPtr, Boolean active,
	Boolean drawGrowIconLines);
void GenericResizeControls(WindowPtr theWindow, short destOverload, short headerHeight,
	Boolean isStyled);
void GenericGetGrowSize(WindowPtr theWindow, Rect *sizeRect);
void GenericIdleInWindow(WindowPtr theWindow, Point mouseLoc);
Boolean GenericKeyPressedInWindow(WindowPtr theWindow, unsigned char theChar, Boolean isEditable,
	Boolean isStyled);
Boolean GenericMouseClickedInWindow(WindowPtr theWindow, Point thePoint, Boolean isEditable,
	Boolean dynamicScroll, Boolean isStyled);
void GenericActivate(WindowPtr theWindow, Boolean drawGrowIconLines);
void GenericDeactivate(WindowPtr theWindow);
