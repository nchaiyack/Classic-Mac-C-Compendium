/* CAutoStartSongs.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CAutoStartSongs	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IAutoStartSongs(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
