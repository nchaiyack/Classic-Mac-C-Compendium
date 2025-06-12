/* CDefaultSamplingRate.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CDefaultSamplingRate	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				IDefaultSamplingRate(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
