/* CSaveBeforeClosing.h */

#pragma once

#include "CModalDialog.h"
#include "CSimpleButton.h"

/* result codes */
#define Yes_Save (-3)
#define No_Save (-2)
#define Cancel_Close (-1)

struct	CSaveBeforeClosingWindow	:	CModalDialog
	{
		short		SaveBeforeClosing(PString Name);
	};

struct	CAskButton	:	CSimpleButton
	{
		MyBoolean*	AnswerLoc;

		void			IAskButton(LongPoint Start, LongPoint Extent, Handle NameString, char Key,
								short Modifiers, CWindow* TheWindow, CEnclosure* TheEnclosure,
								MyBoolean* TheAnswerLoc);
		MyBoolean	DoThang(void);
	};
