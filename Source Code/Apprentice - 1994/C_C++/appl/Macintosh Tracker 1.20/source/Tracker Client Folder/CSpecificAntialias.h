/* CSpecificAntiAliasing.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CSpecificAntiAliasing	:	CCheckbox
	{
		CMyDocument*					Document;

		void				ISpecificAntiAliasing(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
