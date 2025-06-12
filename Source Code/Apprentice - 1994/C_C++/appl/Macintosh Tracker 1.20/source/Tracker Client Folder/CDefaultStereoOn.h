/* CDefaultStereoOn.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CDefaultStereoOn	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IDefaultStereoOn(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
