#ifndef __MY_HELP_WINDOW_H__
#define __MY_HELP_WINDOW_H__

extern	short			gMainTopicShowing;
extern	short			gSubTopicShowing;

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheHelpWindow(WindowRef theWindow);
extern	void ShutDownTheHelpWindow(void);
extern	void OpenTheHelpWindow(WindowRef theWindow);
extern	void DisposeTheHelpWindow(WindowRef theWindow);
extern	void KeyPressedInHelpWindow(WindowRef theWindow, unsigned char keyPressed);
extern	void MouseClickedInHelpWindow(WindowRef theWindow, Point mouseLoc);
extern	void DrawTheHelpWindow(WindowRef theWindow, short theDepth);

#ifdef __cplusplus
}
#endif

#endif
