/* multiopen.h */

pascal void setItem(DialogPtr theDialog, short item, Handle theProc);
pascal void hiliteItem(DialogPtr theDialog, short item, short hiliteValue);
pascal void getBox(DialogPtr theDialog, short item, Rect *box);
void addData(void);
void removeCells(void);
Boolean selected(void);
pascal void myItem(DialogPtr theDialog, short itemNumber);
pascal void initList(DialogPtr theDialog);
pascal Boolean myFileFilter(CInfoPBPtr pb);
pascal short myDlgHook(short item, DialogPtr theDialog);
pascal Boolean myFilterProc(DialogPtr theDialog, EventRecord *event, short *itemHit);
pascal void cleanup(void);
void processData(void);
void handleScrollBar(DialogPtr, EventRecord*);
pascal void scrollProc(ControlHandle theControl, short theCode);