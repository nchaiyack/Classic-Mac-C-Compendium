/* CRewindButton.h */

#pragma once

#include "CPicture.h"

class CMyDocument;

struct	CRewindButton	:	CPicture
	{
		CMyDocument*		Document;

		void				IRewindButton(CMyDocument* TheDocument, CWindow* TheWindow);
		void				DoEnter(void);
		void				DoLeave(void);
	};
