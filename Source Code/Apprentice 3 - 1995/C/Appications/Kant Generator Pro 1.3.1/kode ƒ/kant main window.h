#ifndef __MY_MAIN_WINDOW_H__
#define __MY_MAIN_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetupTheMainWindow(WindowRef theWindow);
extern	void ShutDownTheMainWindow(void);
extern	void OpenTheMainWindow(WindowRef theWindow);
extern	void KeyPressedInMainWindow(WindowRef theWindow, unsigned char theChar);
extern	void DisposeTheMainWindow(WindowRef theWindow);
extern	Boolean CloseTheMainWindow(WindowRef theWindow);
extern	void PasteInMainWindow(WindowRef theWindow);
extern	void SetMainWindowTitle(Str255 theTitle);

#ifdef __cplusplus
}
#endif

#endif
