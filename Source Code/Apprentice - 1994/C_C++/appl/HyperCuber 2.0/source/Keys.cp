//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Keys.cp
//|
//| This contains procedures relating to the use
//| of key controls in HyperCuber.
//|_________________________________________________________

#include "Keys.h"

#include <string.h>


//======================== Prototypes ========================\\

void DrawKey(short key_code, short modifiers, short h, short v);
void DrawModifiers(short modifiers, short h, short v);
void DrawKeySymbol(short pict_id, short& h, short v);
void GetKeyString(short keycode, char *key_name);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure DrawKey
//|
//| Purpose: Draws a text-and-icons description of a key, given the key code
//|          and the modifiers.  The text description of the key will be
//|          drawn to the right of (h, v) and the modifiers will be drawn to
//|          the left of (h, v).
//|
//| Parameters: key_code: the key code of the key
//|             modifers: the modifiers associated with the keyDown event
//|             h:        the horizontal position to draw
//|             v:        the vertical position to draw
//|______________________________________________________________________________

void DrawKey(short key_code, short modifiers, short h, short v)
{

#define COMMAND_PICT	130
#define OPTION_PICT		131
#define SHIFT_PICT		132
#define CONTROL_PICT	133
#define CAPS_LOCK_PICT	134

	char key_string[10];
	GetKeyString(key_code, key_string);			//  Get text description of the key
	MoveTo(h, v);
	DrawString(CtoPstr(key_string));			//  Draw the text description
	
	DrawModifiers(modifiers, h, v-9);			//  Draw the modifiers

}	//==== DrawKey() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure DrawModifiers
//|
//| Purpose: Draws the modifiers, given the modifiers word.  This procedure
//|          draws to the left of (h, v).
//|
//| Parameters: modifers: the modifiers word
//|             h:        the horizontal position to draw
//|             v:        the vertical position to draw
//|______________________________________________________________________________

void DrawModifiers(short modifiers, short h, short v)
{

	if (modifiers & cmdKey)
		DrawKeySymbol(COMMAND_PICT, h, v);
	
	if (modifiers & optionKey)
		DrawKeySymbol(OPTION_PICT, h, v);

	if (modifiers & shiftKey)
		DrawKeySymbol(SHIFT_PICT, h, v);

	if (modifiers & controlKey)
		DrawKeySymbol(CONTROL_PICT, h, v);

	if (modifiers & alphaLock)
		DrawKeySymbol(CAPS_LOCK_PICT, h, v);

}	//==== DrawModifiers() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure DrawKeySymbol
//|
//| Purpose: Draw the symbol for a modifier key.
//|
//| Parameters: pict_id: the id of the PICT for the symbol
//|             h:       the horizontal position of the cursor (this procedure
//|                      moves the cursor back in preparation for the next draw)
//|             v:       the vertical position of the cursor
//|______________________________________________________________________________

void DrawKeySymbol(short pict_id, short& h, short v)
{

	PicHandle picture = GetPicture(pict_id);			//  Get symbol picture
	Rect pict_rect = (*picture)->picFrame;				//  Find size of symbol
	h -= pict_rect.right - pict_rect.left + 2;			//  Move cursor back by width of symbol
	OffsetRect(&pict_rect, h, v);						//  Find location to draw symbol
	DrawPicture(picture, &pict_rect);					//  Draw symbol

}	//==== DrawKeySymbol() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure GetKeyString
//|
//| Purpose: Get a string version of a key.
//|
//| Parameters: keycode:  the key
//|             key_name: receives the string
//|______________________________________________________________________________

void GetKeyString(short keycode, char *key_name)
{

	static	long state = 0;

	char lowbyte = keycode;						//  Get key code as a byte
	
	switch(lowbyte)								//  Handle strange cases
		{
		case ' ':			strcpy(key_name, "space"); break;
		case KeyHome:		strcpy(key_name, "home"); break;
		case KeyEnd:		strcpy(key_name, "end"); break;
		case KeyPageUp:		strcpy(key_name, "pgup"); break;
		case KeyPageDown:	strcpy(key_name, "pgdn"); break;
		case KeyEscape:		strcpy(key_name, "esc"); break;
		case KeyClear:		strcpy(key_name, "clear"); break;
		case KeyHelp:		strcpy(key_name, "help"); break;
		case KeyFwdDelete:	strcpy(key_name, "fwdel"); break;
		case KeyLeftCursor:	strcpy(key_name, "left"); break;
		case KeyRightCursor:strcpy(key_name, "right"); break;
		case KeyUpCursor:	strcpy(key_name, "up"); break;
		case KeyDownCursor:	strcpy(key_name, "down"); break;
		case KeyReturn:		strcpy(key_name, "return"); break;
		case KeyDelete:		strcpy(key_name, "delete"); break;
		case KeyEnter:		strcpy(key_name, "enter"); break;
		case KeyF1:			strcpy(key_name, "F1"); break;
		case KeyF2:			strcpy(key_name, "F2"); break;
		case KeyF3:			strcpy(key_name, "F3"); break;
		case KeyF4:			strcpy(key_name, "F4"); break;
		case KeyF5:			strcpy(key_name, "F5"); break;
		case KeyF6:			strcpy(key_name, "F6"); break;
		case KeyF7:			strcpy(key_name, "F7"); break;
		case KeyF8:			strcpy(key_name, "F8"); break;
		case KeyF9:			strcpy(key_name, "F9"); break;
		case KeyF10:		strcpy(key_name, "F10"); break;
		case KeyF11:		strcpy(key_name, "F11"); break;
		case KeyF12:		strcpy(key_name, "F12"); break;
		case KeyF13:		strcpy(key_name, "F13"); break;
		case KeyF14:		strcpy(key_name, "F14"); break;
		case KeyF15:		strcpy(key_name, "F15"); break;
		case KeyPadEquals:	strcpy(key_name, "[=]"); break;
		case KeyPadDiv:		strcpy(key_name, "[/]"); break;
		case KeyPadMult:	strcpy(key_name, "[*]"); break;
		case KeyPadPlus:	strcpy(key_name, "[+]"); break;
		case KeyPadMinus:	strcpy(key_name, "[-]"); break;
		case KeyPadDot:		strcpy(key_name, "[.]"); break;
		case KeyPad0:		strcpy(key_name, "[0]"); break;
		case KeyPad1:		strcpy(key_name, "[1]"); break;
		case KeyPad2:		strcpy(key_name, "[2]"); break;
		case KeyPad3:		strcpy(key_name, "[3]"); break;
		case KeyPad4:		strcpy(key_name, "[4]"); break;
		case KeyPad5:		strcpy(key_name, "[5]"); break;
		case KeyPad6:		strcpy(key_name, "[6]"); break;
		case KeyPad7:		strcpy(key_name, "[7]"); break;
		case KeyPad8:		strcpy(key_name, "[8]"); break;
		case KeyPad9:		strcpy(key_name, "[9]"); break;
		
		default:
			Handle kchr = GetResource('KCHR', 0);				//  Handle normal characters
			long charcode = KeyTrans(*kchr, keycode, &state);
			key_name[0] = charcode;								//  Make a 1-character string
			key_name[1] = 0;
			
		}
	
}	//==== GetKeyString() ====\\
