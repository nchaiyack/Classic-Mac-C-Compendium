/* StringDialog.h */

#ifndef Included_StringDialog_h
#define Included_StringDialog_h

/* StringDialog module depends on */
/* MiscInfo.h */
/* Debug */
/* Audit */
/* Definitions */
/* Screen */
/* EventLoop */
/* SimpleButton */
/* TextEdit */
/* Memory */
/* Menus */
/* DataMunging */
/* Alert */

/* to avoid dragging Menus into the header */
struct MenuItemType;

/* present a dialog box displaying the string and allowing the user to make */
/* changes. if the user clicks OK, then True is returned, else False is returned. */
MyBoolean		DoStringDialog(char* Prompt, char** TheString, struct MenuItemType* MCut,
							struct MenuItemType* MPaste, struct MenuItemType* MCopy,
							struct MenuItemType* MUndo, struct MenuItemType* MSelectAll,
							struct MenuItemType* MClear);

#endif
