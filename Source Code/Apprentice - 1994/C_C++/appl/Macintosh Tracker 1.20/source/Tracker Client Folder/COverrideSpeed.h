/* COverrideSpeed.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideSpeed	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideSpeed(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
