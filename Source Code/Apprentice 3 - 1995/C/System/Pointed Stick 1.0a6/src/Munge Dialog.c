#include "Global.h"
#include "Munge Dialog.h"
#include "Pack3.h"				// For gRefNum
#include "Utilities.h"			// For GetItemRect(), et. al.
#include "MenubarCDEF.h"

typedef pascal DialogPtr (*GetNewDialogProc)(short, void*, WindowPtr);
typedef pascal ControlHandle (*NewControlProc)(WindowPtr theWindow, Rect* theRect, Str255 title,
									Boolean visible, short value, short min,
									short max, short procID, long refcon);
typedef pascal void (*DisposeControlProc)(ControlHandle);
typedef pascal void (*ShowWindowProc)(WindowPtr);

UniversalProcPtr	gOldNewControl;
UniversalProcPtr	gOldDisposeControl;
UniversalProcPtr	gOldGetNewDialog;
UniversalProcPtr	gOldShowWindow;
short				gMyItemBase;

Boolean				pTimeToMunge = FALSE;
DialogPtr			pDialogToMunge;

void				AddMyItems(DialogPtr);
void				FixDITLHandleSize(Handle items);

//--------------------------------------------------------------------------------

void	MungeNextDialog()
{
	pTimeToMunge = TRUE;
	gMyItemBase = 0;
}


//--------------------------------------------------------------------------------

pascal ControlHandle MyNewControl(WindowPtr theWindow, Rect* theRect, Str255 title,
									Boolean visible, short value, short min,
									short max, short procID, long refcon)
{
	THz					currentZone;
	ControlHandle		control;
	AuxControlRecHdl	auxControl;
	StdHeaderHdl		zeroCDEF;
	
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyNewControl");
	#endif

	SetUpA4();
	
	control = ((NewControlProc) gOldNewControl)(theWindow, theRect, title,
										visible, value, min, max, procID, refcon);

	if (gRefNum != -1) {
		zeroCDEF = (StdHeaderHdl) MyGetResource('CDEF', kZeroCDEF);
		if ((zeroCDEF != NIL) && ((procID >> 4) != kMenubarCDEF)) {
			auxControl = (AuxControlRecHdl) NewHandleSysClear(sizeof(AuxControlRec));
			if (auxControl != NIL) {
				(**auxControl).theControl = control;
				(**auxControl).realCDEF = (StdHeaderHdl) (**control).contrlDefProc;
				(**control).contrlDefProc = (Handle) zeroCDEF;
				(**auxControl).nextAux = (AuxControlRecHdl) (**zeroCDEF).refCon;
				(**zeroCDEF).refCon = (long) auxControl;
			}
		}
	}
	
	RestoreA4();
	return control;
}


//--------------------------------------------------------------------------------

pascal void MyDisposeControl(ControlHandle theControl)
{
	AuxControlRecHdl	auxRecHandle;
	AuxControlRecHdl*	previous;
	StdHeaderHdl		zeroCDEF;

	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyDisposeControl");
	#endif

	SetUpA4();
	
	zeroCDEF = (StdHeaderHdl) MyGetResource('CDEF', kZeroCDEF);
	if (zeroCDEF != NIL) {
		previous = (AuxControlRecHdl*) &(**zeroCDEF).refCon;
		auxRecHandle = *previous;
		while (auxRecHandle != NIL) {
			if ((**auxRecHandle).theControl == theControl) {
				(**theControl).contrlDefProc = (Handle) (**auxRecHandle).realCDEF;
				*previous = (**auxRecHandle).nextAux;
				DisposeHandle((Handle) auxRecHandle);
				break;
			}
			previous = &(**auxRecHandle).nextAux;
			auxRecHandle = (**auxRecHandle).nextAux;
		}
	}

	((DisposeControlProc) gOldDisposeControl)(theControl);

	RestoreA4();
}


//--------------------------------------------------------------------------------

pascal DialogPtr MyGetNewDialog(short dialogID,
							void* dStorage, WindowPtr behind)
{
	DialogPtr	dlg;
	
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyGetNewDialog");
	#endif

	SetUpA4();
		
	dlg = ((GetNewDialogProc) gOldGetNewDialog)(dialogID, dStorage, behind);

	if (pTimeToMunge) {
		pDialogToMunge = dlg;
	}
	RestoreA4();

	return dlg;
}

//--------------------------------------------------------------------------------

pascal void MyShowWindow(WindowPtr window)
{
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyShowWindow");
	#endif

	SetUpA4();
		
	if ((pTimeToMunge) && (window == pDialogToMunge)) {
		pTimeToMunge = FALSE;
		AddMyItems(window);
	}

	((ShowWindowProc) gOldShowWindow)(window);

	RestoreA4();
}

//--------------------------------------------------------------------------------

void AddMyItems(DialogPtr dlg)
{
	Handle			myDialogItems;
	short			oldResFile;
	Point			dlgSize;
	GrafPtr			oldPort;
	Rect			bounds;
	short			menuHeight;
	ControlHandle	control;
	MenuHandle		menu;
	
	FixDITLHandleSize(((DialogPeek) dlg)->items);

	gMyItemBase = CountDITL(dlg);

	myDialogItems = MyGetResource('DITL', kExtraDITL);
//	DetachResource(myDialogItems);
	HNoPurge(myDialogItems);	
	
	oldResFile = UseMyResFile();
	AppendDITL(dlg, myDialogItems, overlayDITL);
	UseResFile(oldResFile);

//	DisposeHandle(myDialogItems);
	ReleaseResource(myDialogItems);
	
	bounds = GetItemRect(dlg, gMyItemBase + kMenubar);
	menuHeight = bounds.bottom - bounds.top;

	dlgSize.h = dlg->portRect.right - dlg->portRect.left;
	dlgSize.v = dlg->portRect.bottom - dlg->portRect.top;
	SizeWindow(dlg, dlgSize.h, dlgSize.v + menuHeight, FALSE);
	
	GetPort(&oldPort);
	SetPort(dlg);
	SetOrigin(dlg->portRect.left, dlg->portRect.top - menuHeight);
	SetPort(oldPort);
	
	bounds.top -= menuHeight;
	bounds.bottom -= menuHeight;
	SetItemRect(dlg, gMyItemBase + kMenubar, &bounds);

	control = (ControlHandle) GetItemHandle(dlg, gMyItemBase + kMenubar);
	(**control).contrlRect = bounds;
}

//--------------------------------------------------------------------------------

void	FixDITLHandleSize(Handle items)
{
	typedef struct DialogItem {
		long			placeHolder;
		Rect			bounds;
		char			itemType;
		unsigned char	dataLength;
		char			data[];			// variable length
	} DialogItem, *DialogItemPtr;

	typedef struct {
		short		numItems;		// actually num items less one
		DialogItem	items[1];		// variable length
	} DITLRecord, *DITLPtr, **DITLHandle;
	
	
	short			handleSize;
	DITLHandle		dialogItems = (DITLHandle) items;
	short			numItems = (**dialogItems).numItems;
	DialogItemPtr	curPtr = &(**dialogItems).items[0];
	DialogItemPtr	startPtr = curPtr;
		
	while (numItems-- >= 0) {
		curPtr = (DialogItemPtr) ( (long) curPtr + sizeof(DialogItem)
			+ ((curPtr->dataLength + 1) & ~1));
	}
	
	handleSize = (long) curPtr - (long) startPtr + sizeof(short);
	
	SetHandleSize(items, handleSize);
}



