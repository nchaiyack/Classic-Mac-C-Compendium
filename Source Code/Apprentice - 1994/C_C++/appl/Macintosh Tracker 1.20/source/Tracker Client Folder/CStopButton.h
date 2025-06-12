/* CStopButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CStopButton	:	CPicture
	{
		CMyDocument*		Document;

		void				IStopButton(CMyDocument* TheDocument, CWindow* TheWindow);
		MyBoolean		DoThang(void);
	};
