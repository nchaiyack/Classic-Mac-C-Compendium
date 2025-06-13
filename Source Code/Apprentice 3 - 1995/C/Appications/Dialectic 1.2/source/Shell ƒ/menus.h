/**********************************************************************\

File:		menus.h

Purpose:	This is the header file for menus.c

\**********************************************************************/

extern	MenuHandle		gHelpMenu;			/* help.c needs access to this menu handle */
extern	MenuHandle		gDialectMenu;		/* dialect scrap.c needs access to this */

Boolean InitTheMenus(void);
void AdjustMenus(void);
void HandleMenu(long mSelect);
void DoTheCloseThing(WindowPeek theWindow);
