//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CModifierKeyPane
//|
//| This implements a pane which displays a keyboard shortcut, with modifier
//|__________________________________________________________________________

#include "CKeyPane.h"

class CModifierKeyPane : public CKeyPane
	{
	
	short			modifiers;
	
  public:

	virtual void	IModifierKeyPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing, short line);

	virtual void	Draw(Rect *area);
	virtual void	SetModifiers(short keycode);
	virtual short	GetModifiers(void);

	virtual void	DoKeyDown(char the_char, Byte key_code, EventRecord *event);
	
	};

