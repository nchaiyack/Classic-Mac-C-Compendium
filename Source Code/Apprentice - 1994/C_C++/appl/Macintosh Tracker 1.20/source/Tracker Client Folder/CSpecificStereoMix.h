/* CSpecificStereoMix.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CSpecificStereoMix	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				ISpecificStereoMix(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
