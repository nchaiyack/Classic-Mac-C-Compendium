/* COverrideAntiAliasing.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideAntiAliasing	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideAntiAliasing(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
