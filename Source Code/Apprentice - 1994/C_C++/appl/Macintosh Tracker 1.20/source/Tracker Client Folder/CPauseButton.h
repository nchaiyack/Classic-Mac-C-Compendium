/* CPauseButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CPauseButton	:	CPicture
	{
		CMyDocument*		Document;

		void				IPauseButton(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
