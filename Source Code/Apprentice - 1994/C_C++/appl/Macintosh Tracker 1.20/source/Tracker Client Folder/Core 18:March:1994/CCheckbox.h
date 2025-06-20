/* CCheckbox.h */

#pragma once

#include "CButton.h"

/* forward */
class CWindow;
class CEnclosure;

struct	CCheckbox	:	CButton
	{
		Handle		Name;
		MyBoolean	State;
		short			FontID;
		short			PointSize;
		EXECUTE(MyBoolean Initialized;)

		/* */			~CCheckbox();
		void			ICheckbox(LongPoint Start, LongPoint Extent, Handle NameString,
								char Key, short Modifiers, short TheFontID, short ThePointSize,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			RedrawNormal(void);
		void			RedrawHilited(void);
		MyBoolean	DoThang(void);
	};
