/* CRepeat.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CRepeat	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IRepeat(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
