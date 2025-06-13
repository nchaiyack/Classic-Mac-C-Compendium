/*
	GetItemList.c		-- routines and patches to find the ListHandle to
							the list of files in the Standard File dialog.

	This file contains the rather involved routines to get the list of files
	in the Standard File dialog. This involves applying two trap patches and
	replacing the drawing procedure of a custom item in the dialog. Here’s the
	deal:

	What we want to do is get the appropriate ListHandle. The easiest way to
	do this is to patch Pack 0 (the List Manager), and look at the ListHandles
	that come throught the pike. The trick is know when the right one is
	coming through and remembering it. (Even though there is only one
	ListHandle in the standard Standard File dialogs, there is no guarantee
	that this is true in the custom dialogs. For example, custom dialogs that
	allow the user to select more than a single file usually do so by adding
	another list to the dialog. We want to make sure that we don’t snag the
	ListHandle to _that_ list.)

	We determine the right time for snagging the ListHandle by replacing the
	drawing procedure for the custom item used for drawing the list. When it’s
	called, we set a flag that tells our Pack 0 patch to snag the next
	ListHandle that’s being updated.

	So how do we install our drawing procedure? By patching SetDItem. Standard
	File has got to call SetDItem to install its drawing procedure for the
	file list. We detect when it does that, remember the procedure it passes
	in, and install our own.

	So that's the process backwards; here it is forwards:

	At INIT time, we places patches on SetDItem and Pack0. These patches are
	normally passive.

	Just after the dialog is created, we set a state variable that tells
	SetDItem to wake up and look for the call that sets the drawing procedure
	for the file list custom item. When it sees that call, it remembers the
	address of the drawing procedure in another variable and then installs our
	own drawing procedure. It then sets the state variable to tell our drawing
	procedure that it should detect when the custom item managing the file
	list is being drawn.

	When the drawing procedure is called to update the list, it sets our state
	variable to wake up the patch on Pack0. Our drawing procedure then calls
	the original drawing routine: the one that Standard File tried to install,
	but that we intercepted. Somewhere in the bowels of that routine, a call
	to LUpdate is going to be made. When that happens, our patch to Pack0 will
	catch it. When Pack0 is called, it will be passed the ListHandle that we
	want. We remember that ListHandle, and then set the state variable to turn
	everybody off.
*/

#include "Global.h"
#include "GetItemList.h"
#include "Pack3.h"				// GetDefaultItem

enum {kNothing, kSetDItem, kDrawProc, kListManager};

typedef pascal void	(*SetDItemProc)(DialogPtr, short, short, Handle, Rect*);
typedef pascal void (*UserItemProc)(DialogPtr, short);

UniversalProcPtr	gOldSetDItem;
UniversalProcPtr	gOldListManager;
ListHandle			gList = NIL;

short				pListItemState = kNothing;
long				pListDrawProc;

extern short		ListItem(void);
pascal void			MyListDrawRoutine(DialogPtr dlg, short item);


//--------------------------------------------------------------------------------

void	StartLookingForList()
{
	gList = NIL;
	pListItemState = kSetDItem;
}


//--------------------------------------------------------------------------------

pascal void	MySetDItem(DialogPtr theDialog, short item,
							short kind, Handle proc, Rect* bounds)
{
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MySetDItem");
	#endif

	SetUpA4();
	if ((pListItemState == kSetDItem)) {
		if ((item == ListItem())) {
			pListDrawProc = (long) proc;
			proc = (Handle) MyListDrawRoutine;
			pListItemState = kDrawProc;
		}
	}

	((SetDItemProc) gOldSetDItem)(theDialog, item, kind, proc, bounds);
	RestoreA4();
}


//--------------------------------------------------------------------------------

pascal void MyListDrawRoutine(DialogPtr dlg, short item)
{
	SetUpA4();
	if (pListItemState == kDrawProc) {
		pListItemState = kListManager;
	}

	((UserItemProc) pListDrawProc)(dlg, item);
	RestoreA4();
}


//--------------------------------------------------------------------------------

pascal void	MyListManager(void)
{
	typedef struct {
		long			oldA4;
		long			retAddr;
		short			selector;
	} stdParams;

	#define selLScroll		0x50
	typedef struct {
		stdParams		header;
		ListHandle		lHandle;
		short			dRows;
		short			dCols;
	} LScrollParams;

	#define selLSetSelect	0x5C
	typedef struct {
		stdParams		header;
		ListHandle		lHandle;
		Cell			theCell;
		Boolean			setIt;
	} LSetSelectParams;

	#define selLUpdate		0x64
	typedef struct {
		stdParams		header;
		ListHandle		lHandle;
		RgnHandle		theRgn;
	} LUpdateParams;
	
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyListManager");
	#endif

	SetUpA4();
	asm {
	;
	; See if we are updating a list item for the first time. If so, remember that item.
	;
		cmp.w	#kListManager,pListItemState
		bne.s	@CheckForRebound

		cmp.w	#selLUpdate,stdParams.selector(A7)
		bne.s	@CheckForRebound

		move.w	#kNothing,pListItemState
		move.l	LUpdateParams.lHandle(A7),gList
		bra.s	@CallPack0
	
	;
	; See if we are selecting a cell for a list representing a new directory. If so,
	; swap in the cell that we _really_ want (because of rebounding).
	;
	@CheckForRebound
		cmp.w	#selLSetSelect,stdParams.selector(A7)
		bne.s	@CheckForScroll
		
		move.l	A7,A0
		move.l	LSetSelectParams.lHandle(A0),-(A7)
		pea		LSetSelectParams.theCell(A0)
		move.w	LSetSelectParams.setIt(A0),-(A7)
		bsr		MyLSetSelect
		adda.w	#4+4+2,A7
		bra.s	@CallPack0
	
	;
	; Prevent Standard File from scrolling to the top of the list.
	;
	@CheckForScroll
		cmp.w	#selLScroll,stdParams.selector(A7)
		bne.s	@CallPack0
		
		move.l	A7,A0
		move.l	LScrollParams.lHandle(A0),-(A7)
		pea		LScrollParams.dRows(A0)
		pea		LScrollParams.dCols(A0)
		bsr		MyLScroll
		adda.w	#4+4+4,A7
		bra.s	@CallPack0
	

	@CallPack0
		move.l	gOldListManager,A0		; get original Pack 0 address
		move.l	(sp)+,A4				; restore original A4 (placed on stack by SetUpA4()
		jmp		(A0)					; jump to original Pack 0 (no tail patch)
	}
}

