/* CSpecificVolume.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CSpecificVolume	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				ISpecificVolume(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
