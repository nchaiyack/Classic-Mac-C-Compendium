/* CButton.h */

#pragma once

#include "CViewRect.h"

/* forward */
class CWindow;

struct	CButton	:	CViewRect
	{
		char			KeyEquivalent;
		short			KeyModifiers;
		short			ActiveModifiers;
		EXECUTE(MyBoolean Initialized;)

		void			IButton(LongPoint Start, LongPoint Extent, char Key, short Modifiers,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			DoMouseDown(MyEventRec Event);
		MyBoolean	DoKeyDown(MyEventRec Event);
		void			DoUpdate(void);
		void			DoEnable(void);
		void			DoDisable(void);
		void			RedrawNormal(void);
		void			RedrawHilited(void);
		MyBoolean	DoThang(void);
		void			DoEnter(void);
		void			DoLeave(void);
	};
