/* CDefaultVolume.h */

#pragma once

#include "CNumberEdit.h"

class CMyDocument;

struct	CDefaultVolume	:	CNumberEdit
	{
		CMyDocument*		Document;

		void				IDefaultVolume(CMyDocument* TheDocument, CWindow* TheWindow);
		void				StoreValue(void);
	};
