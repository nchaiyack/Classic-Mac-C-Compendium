#ifndef __MY_ABOUT_WINDOW_H__
#define __MY_ABOUT_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheAboutWindow(WindowRef theWindow);
extern	void IdleInTheAboutWindow(WindowRef theWindow);
extern	void ShutDownTheAboutWindow(void);
extern	void OpenTheAboutWindow(WindowRef theWindow);
extern	void DisposeTheAboutWindow(void);
extern	void ChangeDepthTheAboutWindow(void);
extern	void DrawTheAboutWindow(WindowRef theWindow);
extern	void CopybitsTheAboutWindow(WindowRef theWindow, WindowRef offscreenWindowRef);
extern	void KeyDownInAboutWindow(WindowRef theWindow);
extern	void MouseDownInAboutWindow(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
