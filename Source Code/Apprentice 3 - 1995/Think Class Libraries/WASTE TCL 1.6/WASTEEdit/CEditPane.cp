/******************************************************************************
	CEditPane.c
	
	Methods for a text editing pane.
		
	Copyright � 1989 Symantec Corporation. All rights reserved.

 ******************************************************************************/


#include "CEditPane.h"
#include "Commands.h"
#include "CDocument.h"
#include "CBartender.h"
#include "Constants.h"

extern	CBartender	*gBartender;

void CEditPane::IEditPane(CView *anEnclosure, CBureaucrat *aSupervisor)

{
	Rect	margin;

	CWASTEText::IWASTEText(anEnclosure, aSupervisor, 1, 1, 0, 0,
						sizELASTIC, sizELASTIC, -1);
	FitToEnclosure(TRUE, TRUE);

		/**
		 **	Give the edit pane a little margin.
		 **	Each element of the margin rectangle
		 **	specifies by how much to change that
		 **	edge. Positive values are down and to
		 **	right, negative values are up and to
		 **	the left.
		 **
		 **/

	SetRect(&margin, 3, 3, -3, -3); // margins increased for drag and drop
	ChangeSize(&margin, FALSE);
#ifdef WASTE11
	InstallDragHandlers(); // to enable drag and drop
#endif
}

void CEditPane::DoCommand(long theCommand)

{
    
    if (((theCommand == cmdPaste) || (theCommand == cmdCut)) && 
        !((CDocument *)itsSupervisor)->dirty) {
        
		((CDocument *)itsSupervisor)->dirty = TRUE;
		gBartender->EnableCmd(cmdSave);
		gBartender->EnableCmd(cmdSaveAs);
	}

	inherited::DoCommand(theCommand);
}


void CEditPane::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)

{
    
    inherited::DoKeyDown(theChar, keyCode, macEvent);

    switch (keyCode) {
    
        case KeyHome:
        case KeyEnd:
        case KeyPageUp:
        case KeyPageDown:
            break;
            
        default:    
            if (!((CDocument *)itsSupervisor)->dirty) {
                ((CDocument *)itsSupervisor)->dirty = TRUE;
                gBartender->EnableCmd(cmdSave);
                gBartender->EnableCmd(cmdSaveAs);
            }
            break;
    }
}


void CEditPane::DoAutoKey(char theChar, Byte keyCode, EventRecord *macEvent)

{
    
    inherited::DoAutoKey(theChar, keyCode, macEvent);

    switch (keyCode) {
    
        case KeyHome:
        case KeyEnd:
        case KeyPageUp:
        case KeyPageDown:
            break;
            
        default:    
            if (!((CDocument *)itsSupervisor)->dirty) {
                ((CDocument *)itsSupervisor)->dirty = TRUE;
                gBartender->EnableCmd(cmdSave);
                gBartender->EnableCmd(cmdSaveAs);
            }
            break;
    }
}
