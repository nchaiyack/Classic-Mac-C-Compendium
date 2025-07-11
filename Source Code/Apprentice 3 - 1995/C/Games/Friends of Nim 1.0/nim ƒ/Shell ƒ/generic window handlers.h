#ifndef __MY_GENERIC_WINDOW_HANDLERS_H__
#define __MY_GENERIC_WINDOW_HANDLERS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void GenericCopybits(WindowRef theWindow, WindowRef offscreenWindowRef, Boolean active,
	Boolean drawGrowIconLines);
extern	void GenericResizeControls(WindowRef theWindow, short destOverload, short headerHeight,
	Boolean isStyled);
extern	void GenericGetGrowSize(Rect *sizeRect);
extern	void GenericIdleInWindow(WindowRef theWindow, Point mouseLoc);
extern	Boolean GenericKeyPressedInWindow(WindowRef theWindow, unsigned char theChar, Boolean isEditable,
	Boolean isStyled);
extern	Boolean GenericMouseClickedInWindow(WindowRef theWindow, Point thePoint, Boolean isEditable,
	Boolean dynamicScroll, Boolean isStyled);
extern	void GenericActivate(WindowRef theWindow, Boolean drawGrowIconLines);
extern	void GenericDeactivate(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
