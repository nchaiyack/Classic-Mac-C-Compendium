/* CDefaultNumRepeats.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CDefaultNumRepeats	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				IDefaultNumRepeats(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
