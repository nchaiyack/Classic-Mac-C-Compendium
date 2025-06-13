#ifndef __MY_WINDOWS_MENU_LAYER_H__
#define __MY_WINDOWS_MENU_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void AddToWindowsMenu(WindowRef theWindow);
extern	void ModifyFromWindowsMenu(WindowRef theWindow);
extern	void RemoveFromWindowsMenu(WindowRef theWindow);
extern	void SelectWindowsMenuItem(short item);
extern	void RebuildWindowsMenu(WindowRef checkedWindow);
extern	void AdjustWindowsMenu(WindowRef checkedWindow);
extern	short GetNumberOfOpenWindows(void);

#ifdef __cplusplus
}
#endif

#endif
