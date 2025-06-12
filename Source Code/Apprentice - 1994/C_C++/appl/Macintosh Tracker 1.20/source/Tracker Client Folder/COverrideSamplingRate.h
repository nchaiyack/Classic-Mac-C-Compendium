/* COverrideSamplingRate.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	COverrideSamplingRate	:	CCheckbox
	{
		CMyDocument*					Document;

		void				IOverrideSamplingRate(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
