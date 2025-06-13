//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane
//|
//| This implements a pane which displays a keyboard shortcut
//|______________________________________________________________________

#include "CKeyPane.h"
#include <CPaneBorder.h>

#include <string.h>
#include <LoMem.h>


//============================ external globals =======================\\

extern CBureaucrat *gGopher;


//======================== Prototypes ========================\\

extern void GetKeyString(short keycode, char *key_name);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::IKeyPane
//|
//| Purpose: Initialize the key pane.
//|
//| Parameters: same as superclass
//|_________________________________________________________

void CKeyPane::IKeyPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing, short line)
{

#define PANE_WIDTH	50
#define PANE_HEIGHT	16

	IEditText(anEnclosure, aSupervisor,				//  Call superclass' initialization routine
				PANE_WIDTH, PANE_HEIGHT, aHEncl, aVEncl,
					aHSizing, aVSizing, line);
	SetAlignCmd(cmdAlignCenter);
	SetFontNumber(systemFont);
	SetFontSize(12);
	Specify(kNotEditable, kNotSelectable,			//  No editing of this text
				kNotStylable);
	SetWantsClicks(TRUE);							//  But it accepts clicks

	CPaneBorder *border = new(CPaneBorder);
	border->IPaneBorder(kBorderFrame);
	SetBorder(border);

}	//==== CKeyPane::IKeyPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::Draw
//|
//| Purpose: This draws the pane.
//|
//| Parameters: none
//|___________________________________________________________________________________

void CKeyPane::Draw(Rect *rect)
{

	inherited::Draw(rect);				//  Draw the text in the pane

	if (this == gGopher)				//  Highlight if this is active
		Highlight();
		
}	//==== CKeyPane::Draw() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::Highlight
//|
//| Purpose: This highlights the pane.
//|
//| Parameters: none
//|___________________________________________________________________________________

void CKeyPane::Highlight(void)
{

	Rect area;
	LongToQDRect(&frame, &area);
	BitClr(&HiliteMode, pHiliteBit);			//  Use color highlighting to highlight pane										 
	InvertRect(&area);

}	//==== CKeyPane::Highlight() ====\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::Activate
//|
//| Purpose: This highlights the pane.
//|
//| Parameters: none
//|___________________________________________________________________________________

void CKeyPane::Activate(void)
{

	Prepare();
	Rect area;
	LongToQDRect(&frame, &area);
	Draw(&area);								//  Draw pane
	
	inherited::Activate();

}	//==== CKeyPane::Activate ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::Deactivate
//|
//| Purpose: This de-highlights the pane.
//|
//| Parameters: none
//|___________________________________________________________________________________

void CKeyPane::Deactivate(void)
{

	Prepare();
	Rect area;
	LongToQDRect(&frame, &area);
	Draw(&area);								//  Draw pane
	
	inherited::Deactivate();

}	//==== CKeyPane::Deactivate ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::DoClick
//|
//| Purpose: Handle a click in the pane.
//|
//| Parameters: none
//|___________________________________________________________________________________

void CKeyPane::DoClick(Point hitPt, short modifierKeys, long when)
{

	BecomeGopher(TRUE);							//  Make this pane the gopher

}	//==== CKeyPane::DoClick ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::DoKeyDown
//|
//| Purpose: This procedure handles a key down event.
//|
//| Parameters: the_char: character associated with key
//|             key_code: the key code of the key
//|             event:    the keydown event
//|______________________________________________________________________

void	CKeyPane::DoKeyDown(char the_char, Byte key_code, EventRecord *event)
{

	SetKey(key_code);						//  Set the value of pane to the pressed key

}	//==== CKeyPane::DoKeyDown() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::GetKey
//|
//| Purpose: Get the current value of the keyboard shortcut
//|
//| Parameters: returns the key code of the current shortcut
//|_______________________________________________________________________

short CKeyPane::GetKey(void)
{

	return code;

}	//==== CKeyPane::GetKey() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::SetKey
//|
//| Purpose: Set the keyboard shortcut to a new value
//|
//| Parameters: keycode: the new shortcut
//|_______________________________________________________________________

void CKeyPane::SetKey(short keycode)
{

	code = keycode;								//  Save the key code

	char	key_name[10];
	GetKeyString(keycode, key_name);			//  Get the key as a string
	
	SetTextPtr(key_name, strlen(key_name));		//  Set the pane to display the string

}	//==== CKeyPane::SetKey() ====\\
