/* CNumOutputBits.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CNumOutputBits	:	CCheckbox
	{
		CMyDocument*					Document;

		void				INumOutputBits(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
