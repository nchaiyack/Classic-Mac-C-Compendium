#ifndef __MY_BUILD_WINDOW_H__
#define __MY_BUILD_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheBuildWindow(WindowRef theWindow);
extern	void ShutDownTheBuildWindow(void);
extern	void OpenTheBuildWindow(WindowRef theWindow);
extern	void IdleInBuildWindow(WindowRef theWindow, Point mouseLoc);
extern	void KeyPressedInBuildWindow(WindowRef theWindow, unsigned char theChar);
extern	Boolean MouseClickedInBuildWindow(WindowRef theWindow, Point thePoint, Boolean dynamicScroll);
extern	void DisposeTheBuildWindow(WindowRef theWindow);
extern	void CopybitsTheBuildWindow(WindowRef theWindow, WindowRef offscreenWindowRef, Boolean active,
	Boolean drawGrowIconLines);
extern	void DrawTheBuildWindow(WindowRef theWindow, short theDepth);
extern	void ActivateTheBuildWindow(WindowRef theWindow, Boolean drawGrowIconLines);
extern	void DeactivateTheBuildWindow(WindowRef theWindow);
extern	void ResizeTheBuildWindow(WindowRef theWindow);
extern	void GetGrowSizeTheBuildWindow(WindowRef theWindow, Rect *sizeRect);
extern	short GetBuildHeaderHeight(void);
extern	void RememberBuildButtonState(WindowRef theWindow);
extern	void UpdateBuildButtons(WindowRef theWindow, Boolean onlyIfChanged);
extern	void SetBuildWindowTitle(Str255 theTitle);

#ifdef __cplusplus
}
#endif

#endif
