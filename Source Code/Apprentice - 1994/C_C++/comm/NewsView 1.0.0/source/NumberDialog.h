/* NumberDialog.h */

#ifndef Included_NumberDialog_h
#define Included_NumberDialog_h

/* NumberDialog module depends on */
/* MiscInfo.h */
/* Debug */
/* Audit */
/* Definitions */
/* StringDialog */
/* Numbers */
/* Memory */

/* so we don't need Menus */
struct MenuItemType;

/* present a dialog displaying InitialValue and asking for a new value.  the */
/* new value is returned.  if the dialog couldn't be presented or the user cancelled, */
/* then the original number is returned */
long					DoNumberDialog(char* Prompt, long InitialValue, struct MenuItemType* MCut,
								struct MenuItemType* MPaste, struct MenuItemType* MCopy,
								struct MenuItemType* MUndo, struct MenuItemType* MSelectAll,
								struct MenuItemType* MClear);

#endif
