/* CSkipToNextButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CSkipToNextButton	:	CPicture
	{
		CMyDocument*		Document;

		void				ISkipToNextButton(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
