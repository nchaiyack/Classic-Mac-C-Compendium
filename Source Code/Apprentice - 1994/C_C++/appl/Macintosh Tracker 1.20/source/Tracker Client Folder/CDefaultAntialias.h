/* CDefaultAntiAliasing.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CDefaultAntiAliasing	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IDefaultAntiAliasing(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
