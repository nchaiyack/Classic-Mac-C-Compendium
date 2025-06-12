/* COverrideStereoMix.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideStereoMix	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideStereoMix(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
