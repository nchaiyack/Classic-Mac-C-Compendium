/* CAutoNextSong.h */

#pragma once

#include "CCheckbox.h"

class CMyDocument;

struct	CAutoNextSong	:	CCheckbox
	{
		CMyDocument*		Document;

		void				IAutoNextSong(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
