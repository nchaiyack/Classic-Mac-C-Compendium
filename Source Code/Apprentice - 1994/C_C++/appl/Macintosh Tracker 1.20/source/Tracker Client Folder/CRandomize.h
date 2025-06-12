/* CRandomize.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CRandomize	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IRandomize(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
