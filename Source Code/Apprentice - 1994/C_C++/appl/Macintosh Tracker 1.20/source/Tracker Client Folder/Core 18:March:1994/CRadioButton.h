/* CRadioButton.h */

#pragma once

#include "CButton.h"

/* forward */
class CWindow;
class CSack;
class CEnclosure;

struct	CRadioButton	:	CButton
	{
		Handle		Name;
		MyBoolean	State;
		CSack*		RadioGroup;
		short			PointSize;
		short			FontID;
		EXECUTE(MyBoolean Initialized;)

		/* */			~CRadioButton();
		void			IRadioButton(LongPoint Start, LongPoint Extent, Handle NameString,
								char Key, short Modifiers, CSack** CurrentRadioGroup, short TheFontID,
								short ThePointSize, CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			RedrawNormal(void);
		void			RedrawHilited(void);
		MyBoolean	DoThang(void);
		void			Uncheck(void);
		void			SetState(void);
		void			ClearState(void);
	};
