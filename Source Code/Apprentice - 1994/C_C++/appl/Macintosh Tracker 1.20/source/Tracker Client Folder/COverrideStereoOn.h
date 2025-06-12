/* COverrideStereoOn.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideStereoOn	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideStereoOn(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
