/* CDefaultStereoMix.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CDefaultStereoMix	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				IDefaultStereoMix(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
