/* CSpecificStereoOn.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CSpecificStereoOn	:	CCheckbox
	{
		CMyDocument*					Document;

		void				ISpecificStereoOn(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
