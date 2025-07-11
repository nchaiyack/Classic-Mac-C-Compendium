/* FindDialog.h */

#ifndef Included_FindDialog_h
#define Included_FindDialog_h

/* FindDialog module depends on */
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
/* Alert */

/* to avoid dragging Menus.h into the header */
struct MenuItemType;

/* these possible operations can be returned */
typedef enum {eFindCancel, eFindFromStart, eFindAgain, eDontFind} FindOpType;

/* present the find dialog, update internal variables, and return the operation to do */
FindOpType		DoFindDialog(char** SearchKey, char** Replace, struct MenuItemType* MCut,
								struct MenuItemType* MPaste, struct MenuItemType* MCopy,
								struct MenuItemType* MUndo, struct MenuItemType* MSelectAll,
								struct MenuItemType* MClear);

#endif
