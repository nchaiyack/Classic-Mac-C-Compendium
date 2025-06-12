/* CDefaultSpeed.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CDefaultSpeed	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				IDefaultSpeed(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
