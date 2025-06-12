/* CFastForwardButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CFastForwardButton	:	CPicture
	{
		CMyDocument*		Document;

		void				IFastForwardButton(CMyDocument* TheDocument, CWindow* TheWindow);
		void				DoEnter(void);
		void				DoLeave(void);
	};
