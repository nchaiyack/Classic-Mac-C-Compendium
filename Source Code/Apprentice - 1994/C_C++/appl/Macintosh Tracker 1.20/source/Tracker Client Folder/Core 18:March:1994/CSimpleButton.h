/* CSimpleButton.h */

#pragma once

#include "CButton.h"

/* forward */
class CWindow;
class CEnclosure;

struct	CSimpleButton	:	CButton
	{
		Handle		Name;
		EXECUTE(MyBoolean Initialized;)

		/* */			~CSimpleButton();
		void			ISimpleButton(LongPoint Start, LongPoint Extent, Handle NameString,
								char Key, short Modifiers, CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			RedrawNormal();
		void			RedrawHilited();
	};
