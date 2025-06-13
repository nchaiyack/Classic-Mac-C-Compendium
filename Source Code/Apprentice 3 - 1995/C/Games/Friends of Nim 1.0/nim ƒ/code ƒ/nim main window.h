#ifndef __NIM_MAIN_WINDOW_H__
#define __NIM_MAIN_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheMainWindow(WindowRef theWindow);
extern	void OpenTheMainWindow(WindowRef theWindow);
extern	void IdleInMainWindow(WindowRef theWindow);
extern	void KeyPressedInMainWindow(WindowRef theWindow);
extern	void MouseClickedInMainWindow(WindowRef theWindow, Point thePoint);
extern	Boolean CloseTheMainWindow(void);
extern	void DrawTheMainWindow(WindowRef theWindow, short theDepth);
extern	void UndoInMainWindow(WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
