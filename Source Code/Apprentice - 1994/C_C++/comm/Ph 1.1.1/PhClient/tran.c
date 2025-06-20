/*_____________________________________________________________________

  	tran.c - Server Transaction Movable Modal Dialog.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "tran.h"
#include "utl.h"
#include "glob.h"
#include "oop.h"
#include "mtcp.h"
#include "wstm.h"

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static DialogPtr			Window;			/* ptr to dialog window */

static oop_Dispatch	dispatch = {
								tran_DoPeriodic,
								oop_DoClick,
								oop_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								oop_DoCommand
							};

/*_____________________________________________________________________

	tran_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void tran_DoPeriodic (WindowPtr w)

{
#pragma unused (w)

	utl_SpinCursor();
	if (oop_GetWindItemHit(Window) || Done) {
		mtcp_CancelTransaction();
		oop_ClearWindItemHit(Window);
	}
}

/*_____________________________________________________________________

	tran_BeginTransaction - Begin Server Transaction.
	
	Entry:	msgIndex = index in STR# of wait message.
				p0 = param to plug into wait message, or nil if none.
_____________________________________________________________________*/

void tran_BeginTransaction (short msgIndex, Str255 p0)

{
	Str255		msg;					/* wait message */
	Str255		tmpl;					/* wait message template */
	short			itemType;			/* item type */
	Handle		item;					/* handle to item */
	Rect			box;					/* item rectangle */

	utl_InitSpinCursor(BBArray, numBB, intervalBB);
	ShowCursor();
	Window = wstm_Restore(true, tranID, nil, &TranState);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	GetIndString(tmpl, stringsID, msgIndex);
	utl_PlugParams(tmpl, msg, p0, nil, nil, nil);
	GetDItem(Window, tranMsg, &itemType, &item, &box);
	SetIText(item, msg);
	oop_NewDialog(Window, tranModal, nil, &dispatch, true, 0, tranAbortBtn);
	ShowWindow(Window);
	oop_UpdateAll();
	mtcp_BeginTransaction();
}

/*_____________________________________________________________________

	tran_EndTransaction - End Server Transaction.
_____________________________________________________________________*/

void tran_EndTransaction (void)

{
	wstm_Save(Window, &TranState);
	oop_DoClose(Window);
	InitCursor();
	oop_UpdateAll();
}
