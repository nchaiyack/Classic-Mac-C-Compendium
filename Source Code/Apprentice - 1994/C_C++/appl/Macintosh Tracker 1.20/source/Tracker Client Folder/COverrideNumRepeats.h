/* COverrideNumRepeats.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideNumRepeats	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideNumRepeats(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
