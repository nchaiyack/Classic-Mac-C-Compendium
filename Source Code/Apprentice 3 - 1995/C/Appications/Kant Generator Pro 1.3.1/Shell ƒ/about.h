#ifndef __MY_ABOUT_WINDOW_H__
#define __MY_ABOUT_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheAboutWindow(WindowRef theWindow);
extern	void IdleInTheAboutWindow(WindowRef theWindow);
extern	void ShutDownTheAboutWindow(void);
extern	void OpenTheAboutWindow(WindowRef theWindow);
extern	void DisposeTheAboutWindow(WindowRef theWindow);
extern	void ChangeDepthTheAboutWindow(WindowRef theWindow);
extern	void DrawTheAboutWindow(WindowRef theWindow, short theDepth);
extern	void CopybitsTheAboutWindow(WindowRef theWindow, WindowRef offscreenWindowRef);
extern	void KeyDownInAboutWindow(WindowRef theWindow, unsigned char theChar);
extern	void MouseDownInAboutWindow(WindowRef theWindow, Point thePoint);

#ifdef __cplusplus
}
#endif

#endif
