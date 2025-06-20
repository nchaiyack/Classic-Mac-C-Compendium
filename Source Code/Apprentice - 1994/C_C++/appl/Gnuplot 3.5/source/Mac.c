#include "Mac.h"
#include <MacHeaders> 
#include <console.h>

//private variables
static 	WindowPtr	itsWindow	=NULL;
static	GrafPtr 	savePort	=NULL;
static 	PicHandle 	itsPicture	=NULL;
static Handle saveMBar, myMBar;

//private functions prototype
static Boolean MAC_event(void);
static void SetUpMenu(void);
static void AdjustMenu(void);
static void RestoreMenu(void);

MAC_init()
{
	Rect	boundsRect = {50, 10, 200, 200};
	
	itsWindow = NewWindow(NULL, &boundsRect,MAC_WINDOW_TITLE, FALSE, 
							noGrowDocProc, (void *)-1L, FALSE, 0L);
	SizeWindow (itsWindow,MAC_XMAX+40,MAC_YMAX+20,FALSE);
    
    GetPort(&savePort);	
	SetPort(itsWindow);
	TextFont(monaco);
	TextSize(9);
	TextFace(normal);
	PenNormal();
	SetUpMenu();
	SetPort(savePort);
}


MAC_graphics()
{	
	
	GetPort(&savePort);
	SetPort(itsWindow);
 	EraseRect(&(((GrafPtr) itsWindow)->portRect));
    ShowWindow (itsWindow);
    BringToFront (itsWindow);
    HiliteWindow (itsWindow,TRUE);
	if(itsPicture != NULL){
		KillPicture(itsPicture);
		itsPicture=NULL;
	}
	itsPicture=OpenPicture (&(((GrafPtr) itsWindow)->portRect));
	TextFont(monaco);
	TextSize(9);
	TextFace(normal);
	PenNormal();
}


MAC_text()
{
	Rect myRect;
	
	ClosePicture();
	if(itsPicture != NULL){
		myRect=itsWindow->portRect;
		DrawPicture(itsPicture,&myRect);
	}
	if(itsWindow !=NULL){
		SetWTitle(itsWindow, "\pHit any key to continue.");
	  	while(!MAC_event());
		SetWTitle(itsWindow, MAC_WINDOW_TITLE);
	}
	SetPort(savePort);
	cshow(stdout);
}


MAC_linetype(linetype)
int linetype;
{
	int lt;
		
	lt=(linetype > 1?linetype:1);
	PenSize(lt,lt);
}


MAC_move(x,y)
unsigned int x,y;
{
	MoveTo(x+MAC_XOFFSET,MAC_YMAX-y);
}


MAC_vector(x,y)
unsigned int x,y;
{
	LineTo(x+MAC_XOFFSET,MAC_YMAX-y);
}


MAC_put_text(x,y,str)
unsigned int x,y;
char str[];
{
	MoveTo(x+MAC_XOFFSET,MAC_YMAX-y+MAC_VCHAR/2);
	PenNormal();
	DrawString(CtoPstr(str));
	PtoCstr(str);
}

MAC_reset()
{
    //GetPort(&savePort);	
	//SetPort(itsWindow);
 	//EraseRect(&(((GrafPtr) itsWindow)->portRect));
    //SetPort(savePort);	
}

//private functions

#define appleM			1
#define fileM			2
#define editM			3

/* Edit menu command indices */
#define undoCommand 	1
#define cutCommand		3
#define copyCommand		4
#define pasteCommand	5
#define clearCommand	6

#define fmNew		1
#define fmOpen		2
#define fmClose		4
#define fmSave		5

static Boolean MAC_event(){
	EventRecord	theEvent;
	WindowPtr	whichWindow;
	
	SystemTask ();
	
	if (GetNextEvent (everyEvent, &theEvent))
	  switch (theEvent.what){
		case mouseDown:
			switch(FindWindow(theEvent.where,&whichWindow)){
				case inSysWindow:
					SystemClick(&theEvent,whichWindow);
					break;
				case inDrag:{
						Rect dragRect;
						SetRect(&dragRect,4,24,
							screenBits.bounds.right-4,screenBits.bounds.bottom-4);
						DragWindow (whichWindow,theEvent.where, &dragRect);
					}
					break;
				case inMenuBar:
					AdjustMenu();
					doMenuCommand(MenuSelect(theEvent.where));
					RestoreMenu();
					break;
			};
			break;
		case keyDown:
		case autoKey:
		    if ((theEvent.modifiers & cmdKey) != 0){
				AdjustMenu();
		      	doMenuCommand(MenuKey(theEvent.message & charCodeMask));
				RestoreMenu();
			}
			else
				return TRUE;
			break;
			
		case updateEvt:
			BeginUpdate(itsWindow);
			if(itsPicture != NULL) DrawPicture(itsPicture,&(itsWindow->portRect));
			EndUpdate(itsWindow);
		    break;
		    
		case activateEvt:
			InvalRect(&itsWindow->portRect);
			break;
	    }
	    return FALSE;
}
	
static  doMenuCommand(long mCmd){
	int 	item,menu;
	Str255		daName;
	short		daRefNum;

	item=LoWord(mCmd);
	menu=HiWord(mCmd);
	switch(menu){
		case appleM:
			switch ( item ) {
/* 				case iAbout:		/* bring up alert for About */
/*					break;			/*							*/
				default:			/* all non-About items in this menu are DAs */
					/* type Str255 is an array in MPW 3 */
					GetItem(GetMHandle(appleM), item, daName);
					daRefNum = OpenDeskAcc(daName);
					break;
			}
			break;
		case fileM:
			switch(item){
				case fmSave:
					doSave();
					break;
			}
			break;
		case editM:
			switch(item){
				case copyCommand:
					doCopy();
					break;
			}
			break;
	}
	HiliteMenu(0);
}

static doCopy(){
	if( ZeroScrap() != noErr) {
		fprintf(stderr, "Cann't initialize Clippboard\n");
#ifdef THINK_C
		exit(0);
#else
		exit();
#endif
	}
	else{
		HLock(itsPicture);
		PutScrap(GetHandleSize(itsPicture),'PICT', *itsPicture);
		HUnlock(itsPicture);
	}
}

// routines to create PICT file.
#define NIL NULL
long PICTCount;
int globalRef;
PicHandle newPICTHand;

pascal void PutPICTData(Ptr dataPtr, int byteCount){
	long longCount;
	int	err;
	
	longCount=byteCount;
	PICTCount+=byteCount;
	
	err=FSWrite(globalRef, &longCount, dataPtr);
	if(newPICTHand != NIL) (**newPICTHand).picSize=PICTCount;
}

static doSave(){
	Point where={97,103};
	SFReply reply;
	OSErr	err;
	int i;
	long longCount,longZero;
	Rect pFrame;
	QDProcs myProcs;
			
	SFPutFile(where,"\pSave picture into", "\pUntitled", NULL, &reply);
	if(reply.good) {
		err=Create(reply.fName,reply.vRefNum,'????','PICT');
		if( err == noErr || err == dupFNErr) {
			FSOpen(reply.fName,reply.vRefNum,&globalRef);
			SetStdProcs(&myProcs);
			itsWindow->grafProcs=&myProcs;
			myProcs.putPicProc=(Ptr) PutPICTData;
			longZero=0L;
			longCount=4;
			PICTCount=sizeof(Picture);
			for(i=1;i<=(512/4+sizeof(Picture));i++){
				FSWrite(globalRef,&longCount,&longZero);
			}
			pFrame=(**itsPicture).picFrame;
			newPICTHand=NIL;
			newPICTHand=OpenPicture(&pFrame);
			DrawPicture(itsPicture,&pFrame);
			ClosePicture();
			SetFPos(globalRef, fsFromStart,512);
			longCount=sizeof(Picture);
			FSWrite(globalRef,&longCount,(Ptr) (*newPICTHand));
			FSClose(globalRef);
			itsWindow->grafProcs=NIL;
			KillPicture(newPICTHand);
		}
		else{
		/* handle error */
		}
	}
}

static void SetUpMenu(void){
	MenuHandle mh;
	
	saveMBar=GetMenuBar();
	ClearMenuBar();
	mh=NewMenu(appleM,"\p\024");
	InsertMenu(mh,0);
	AddResMenu(GetMHandle(appleM), 'DRVR');	/* add DA names to Apple menu */
	mh=NewMenu(fileM,"\pFile");
	AppendMenu(mh, "\p(New/N;(Open/O;(-;(Close/W;Save/S");
	InsertMenu(mh,0);
	mh=NewMenu(editM,"\pEdit");
	AppendMenu(mh, "\p(Undo/Z;(-;(Cut/X;Copy/C");
	InsertMenu(mh,0);
	DrawMenuBar();
	myMBar=GetMenuBar();
}

static void AdjustMenu(void){
	SetMenuBar(myMBar);
	DrawMenuBar();
}

static void RestoreMenu(void){
	SetMenuBar(saveMBar);
	DrawMenuBar();
}

