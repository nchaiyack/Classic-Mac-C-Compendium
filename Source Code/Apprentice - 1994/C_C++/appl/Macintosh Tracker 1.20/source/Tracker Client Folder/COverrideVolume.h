/* COverrideVolume.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideVolume	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideVolume(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
