#ifndef __MY_MENUS_H__
#define __MY_MENUS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	Boolean InitTheMenus(void);
extern	void ShutDownTheMenus(void);
extern	void AdjustMenus(void);
extern	void HandleMenu(long mSelect);
extern	void DoTheCloseThing(WindowPeek theWindow);
extern	void RebuildModulesMenu(void);
extern	void RebuildInsertMenu(void);
extern	void IconifyMenus(Boolean useIcons);

#ifdef __cplusplus
}
#endif

#endif
