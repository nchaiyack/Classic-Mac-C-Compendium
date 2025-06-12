#include "Show_help.h"

#define		MODAL_MASK	0x017F	// don't take disk, OS, or high-level events

/*	---------------------------------------------------------------------
	Fake_ModalDialog		ModalDialog cannot be used to handle movable
							modal dialogs (without skanky hacks) because
							it prevents one from switching to another
	application, even if the window is not of the dBoxProc type.  Therefore
	I use this routine, which simulates much of what ModalDialog does.
	---------------------------------------------------------------------
*/
pascal void Fake_ModalDialog( ModalFilterProcPtr filterProc,short *itemHit )
{
	EventRecord		event;
	Boolean			handled;
	DialogPtr		front, which_dialog;
	short			what_hit;
	
	*itemHit = 0;
	front = FrontWindow();
	
	do {
		GetNextEvent( MODAL_MASK, &event );
		handled = filterProc( front, &event, itemHit );
		if (!handled)
		{
			if (IsDialogEvent(&event))
			{
				if (DialogSelect( &event, &which_dialog, &what_hit ))
				{
					if (which_dialog == front)
					{
						*itemHit = what_hit;
					}
				}
				if (which_dialog == front)
					handled = true;
			}
			/*
				Beep if there is a mouse click on another window
				owned by this application.
			*/
			if ( !handled && (event.what == mouseDown) )
				SysBeep(1);
		}
	} while (*itemHit == 0);
}