/* CPlayButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CPlayButton	:	CPicture
	{
		CMyDocument*		Document;

		void				IPlayButton(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
