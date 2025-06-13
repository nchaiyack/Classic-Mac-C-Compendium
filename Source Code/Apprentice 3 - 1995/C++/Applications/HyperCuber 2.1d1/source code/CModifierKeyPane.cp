//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CModifierKeyPane
//|
//| This implements a pane which displays a keyboard shortcut, with modifier
//|__________________________________________________________________________

#include "CModifierKeyPane.h"



//========================== Prototypes ===========================\\

void DrawModifiers(short modifiers, short h, short v);


//============================ external globals =======================\\

extern CBureaucrat *gGopher;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CModifierKeyPane::IModifierKeyPane
//|
//| Purpose: Initialize the key pane.
//|
//| Parameters: same as superclass
//|_________________________________________________________

void CModifierKeyPane::IModifierKeyPane(CView *anEnclosure, CBureaucrat *aSupervisor,
						short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing, short line)
{

#define MODIFIERS_WIDTH	60

	IKeyPane(anEnclosure, aSupervisor,				//  Call superclass' initialization routine
				aHEncl, aVEncl,
					aHSizing, aVSizing, line);
	
	Rect delta_rect = {0, 0, 0, MODIFIERS_WIDTH};
	ChangeSize(&delta_rect, FALSE);					//  Make pane big enough for modifiers
	SetAlignCmd(cmdAlignLeft);						//  Align text left

	(**macTE).destRect.left += MODIFIERS_WIDTH;		//  Make room for the modifiers to left of key

}	//==== CModifierKeyPane::IModifierKeyPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CModifierKeyPane::Draw
//|
//| Purpose: This procedure draws the key and the modifiers.
//|
//| Parameters: area: part of the pane to draw
//|______________________________________________________________________

void CModifierKeyPane::Draw(Rect *area)
{

	CEditText::Draw(area);						//  Draw the key
	
	DrawModifiers(modifiers, MODIFIERS_WIDTH,
						frame.top + 3);			//  Draw the modifiers
	
	if (this == gGopher)						//  Highlight if this is active
		Highlight();
	
}	//==== CModifierKeyPane::Draw() ====\\
	

//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::DoKeyDown
//|
//| Purpose: This procedure handles a key down event.
//|
//| Parameters: the_char: character associated with key
//|             key_code: the key code of the key
//|             event:    the keydown event
//|
//| Modified January 5, 1995 by Greg Ferrar
//|   fixed bug where even non-keyboard-related modifiers were saved.
//|______________________________________________________________________

void	CModifierKeyPane::DoKeyDown(char the_char, Byte key_code, EventRecord *event)
{

	modifiers = (event->modifiers &
		(cmdKey | optionKey | shiftKey | controlKey | alphaLock));	//  Remember the modifiers

	inherited::DoKeyDown(the_char, key_code, event);	//  Call superclass

}	//==== CModifierKeyPane::DoKeyDown() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyPane::GetModifiers
//|
//| Purpose: Get the current value of the keyboard shortcut modifiers
//|
//| Parameters: returns the modifiers of the current shortcut
//|_______________________________________________________________________

short CModifierKeyPane::GetModifiers(void)
{

	return modifiers;

}	//==== CModifierKeyPane::GetModifiers() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CModifierKeyPane::SetModifiers
//|
//| Purpose: Set the keyboard shortcut to a new value
//|
//| Parameters: modifiers: the new shortcut
//|_______________________________________________________________________

void CModifierKeyPane::SetModifiers(short new_modifiers)
{

	modifiers = new_modifiers;
	
}	//==== CKeyPane::SetKey() ====\\
