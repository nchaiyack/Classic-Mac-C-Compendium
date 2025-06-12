/* CSpecificSamplingRate.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CSpecificSamplingRate	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				ISpecificSamplingRate(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
