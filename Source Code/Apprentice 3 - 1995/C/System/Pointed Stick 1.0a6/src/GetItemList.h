#ifndef __GETITEMLIST__
#define __GETITEMLIST__

extern UniversalProcPtr	gOldSetDItem;
extern UniversalProcPtr	gOldListManager;
extern ListHandle		gList;

void					StartLookingForList(void);
pascal void				MySetDItem(DialogPtr theDialog, short item,
								short kind, Handle proc, Rect* bounds);
pascal void				MyListManager(void);

#endif