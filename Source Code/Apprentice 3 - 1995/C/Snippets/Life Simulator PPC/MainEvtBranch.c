/* Cell Proj 1.0 */

#include <fp.h>
#include <stdio.h>

#include "Cell_Proto.h"
#include "Cell_Definitions.h"
#include "Cell_Variables.h"

void HandleEvent(EventRecord *theEvent)
{
	char	theChar;

	switch ( theEvent->what ) {
		case nullEvent:
			generations++;
			DoCellularAutomata();
			break;
		case mouseDown:
			HandleMouseDown(theEvent);
			break;
		case keyDown:
		case autoKey:
			{
				// hitting a key while it's running will update the title
				// bar of the window to show generations, time, and fps rate
				// -DaveP 5/27/95
				long elapsedTime = TickCount() - startTime;
				Str255	newTitle;
				sprintf((char *)newTitle,"G:%d T:%d F:%d",
					generations,elapsedTime/60,
					generations/(elapsedTime/60));
				c2pstr((char *)newTitle);
				SetWTitle(gCellWindow,newTitle);
			}
			theChar = theEvent->message & charCodeMask;
			if (theChar == 'r') {	// allows you to restart timer without restarting sim
				startTime = TickCount();
				generations = 0;
			}
			if (( theEvent->modifiers & cmdKey ) != 0 )
				HandleMenuChoice( MenuKey( theChar ));
			break;
		case updateEvt: {
				BeginUpdate( ( WindowPtr ) theEvent->message );
				EndUpdate( ( WindowPtr ) theEvent->message );
			}
			break;
		case activateEvt:
			if (( theEvent->modifiers & activeFlag ) != 0 ) {
				SetPort(( WindowPtr ) theEvent->message);
				SelectWindow(( WindowPtr ) theEvent->message);
			} else {
				SetPort(( WindowPtr ) theEvent->message);
			}
			break;
	}
}