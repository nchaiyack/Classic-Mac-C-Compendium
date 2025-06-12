/* CSpecificNumRepeats.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CSpecificNumRepeats	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				ISpecificNumRepeats(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
