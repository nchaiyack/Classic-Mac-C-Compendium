/* CSpecificSpeed.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CSpecificSpeed	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				ISpecificSpeed(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
