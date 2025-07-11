#ifndef	_MENU_
#define	_MENU_





typedef void (*ItemProc)(short menu,short item);





#define	kAllItems		-1





typedef struct MenuItemProc
{
	short		menu;
	short		item;
	ItemProc	proc;
} MenuItemProc, *MenuItemProcPtr, **MenuItemProcHandle;





void InitAppMenus(void);
void InsertAppleMenu(short menuID,short beforeID);
void InsertPlainMenu(short menuID,short beforeID);
void AddItemProc(ItemProc proc,short menu,short item);
void doMenuSelect(short menu,short item);
void AboutItemProc(short menu,short item);
void AppleItemsProc(short menu,short item);
void FileBinHexItemProc(short menu,short item);
void FileQuitItemProc(short menu,short item);


#endif	_MENU_